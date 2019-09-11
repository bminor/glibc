/* Look up a symbol in the loaded objects.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <alloca.h>
#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <dl-hash.h>
#include <dl-machine.h>
#include <sysdep-cancel.h>
#include <hp-timing.h>
#include <libc-lock.h>
#include <tls.h>
#include <atomic.h>

#include <assert.h>

/* Return nonzero if check_match should consider SYM to fail to match a
   symbol reference for some machine-specific reason.  */
#ifndef ELF_MACHINE_SYM_NO_MATCH
# define ELF_MACHINE_SYM_NO_MATCH(sym) 0
#endif

#define VERSTAG(tag)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (tag))

#ifndef ADDRIDX
# define ADDRIDX(tag) (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGNUM \
		       + DT_EXTRANUM + DT_VALNUM + DT_ADDRTAGIDX (tag))
#endif


struct sym_val
  {
    const ElfW(Sym) *s;
    struct link_map *m;
  };


/* Statistics function.  */
#ifdef SHARED
# define bump_num_relocations() ++GL(dl_num_relocations)
#else
# define bump_num_relocations() ((void) 0)
#endif

/* Utility function for do_lookup_x. The caller is called with undef_name,
   ref, version, flags and type_class, and those are passed as the first
   five arguments. The caller then computes sym, symidx, strtab, and map
   and passes them as the next four arguments. Lastly the caller passes in
   versioned_sym and num_versions which are modified by check_match during
   the checking process.  */
static const ElfW(Sym) *
check_match (const char *const undef_name,
	     const ElfW(Sym) *const ref,
	     const struct r_found_version *const version,
	     const int flags,
	     const int type_class,
	     const ElfW(Sym) *const sym,
	     const Elf_Symndx symidx,
	     const char *const strtab,
	     const struct link_map *const map,
	     const ElfW(Sym) **const versioned_sym,
	     int *const num_versions)
{
  unsigned int stt = ELFW(ST_TYPE) (sym->st_info);
  assert (ELF_RTYPE_CLASS_PLT == 1);
  if (__glibc_unlikely ((sym->st_value == 0 /* No value.  */
			 && stt != STT_TLS)
			|| ELF_MACHINE_SYM_NO_MATCH (sym)
			|| (type_class & (sym->st_shndx == SHN_UNDEF))))
    return NULL;

  /* Ignore all but STT_NOTYPE, STT_OBJECT, STT_FUNC,
     STT_COMMON, STT_TLS, and STT_GNU_IFUNC since these are no
     code/data definitions.  */
#define ALLOWED_STT \
  ((1 << STT_NOTYPE) | (1 << STT_OBJECT) | (1 << STT_FUNC) \
   | (1 << STT_COMMON) | (1 << STT_TLS) | (1 << STT_GNU_IFUNC))
  if (__glibc_unlikely (((1 << stt) & ALLOWED_STT) == 0))
    return NULL;

  if (sym != ref && strcmp (strtab + sym->st_name, undef_name))
    /* Not the symbol we are looking for.  */
    return NULL;

  const ElfW(Half) *verstab = map->l_versyms;
  if (version != NULL)
    {
      if (__glibc_unlikely (verstab == NULL))
	{
	  /* We need a versioned symbol but haven't found any.  If
	     this is the object which is referenced in the verneed
	     entry it is a bug in the library since a symbol must
	     not simply disappear.

	     It would also be a bug in the object since it means that
	     the list of required versions is incomplete and so the
	     tests in dl-version.c haven't found a problem.*/
	  assert (version->filename == NULL
		  || ! _dl_name_match_p (version->filename, map));

	  /* Otherwise we accept the symbol.  */
	}
      else
	{
	  /* We can match the version information or use the
	     default one if it is not hidden.  */
	  ElfW(Half) ndx = verstab[symidx] & 0x7fff;
	  if ((map->l_versions[ndx].hash != version->hash
	       || strcmp (map->l_versions[ndx].name, version->name))
	      && (version->hidden || map->l_versions[ndx].hash
		  || (verstab[symidx] & 0x8000)))
	    /* It's not the version we want.  */
	    return NULL;
	}
    }
  else
    {
      /* No specific version is selected.  There are two ways we
	 can got here:

	 - a binary which does not include versioning information
	 is loaded

	 - dlsym() instead of dlvsym() is used to get a symbol which
	 might exist in more than one form

	 If the library does not provide symbol version information
	 there is no problem at all: we simply use the symbol if it
	 is defined.

	 These two lookups need to be handled differently if the
	 library defines versions.  In the case of the old
	 unversioned application the oldest (default) version
	 should be used.  In case of a dlsym() call the latest and
	 public interface should be returned.  */
      if (verstab != NULL)
	{
	  if ((verstab[symidx] & 0x7fff)
	      >= ((flags & DL_LOOKUP_RETURN_NEWEST) ? 2 : 3))
	    {
	      /* Don't accept hidden symbols.  */
	      if ((verstab[symidx] & 0x8000) == 0
		  && (*num_versions)++ == 0)
		/* No version so far.  */
		*versioned_sym = sym;

	      return NULL;
	    }
	}
    }

  /* There cannot be another entry for this symbol so stop here.  */
  return sym;
}

/* Utility function for do_lookup_unique.  Add a symbol to TABLE.  */
static void
enter_unique_sym (struct unique_sym *table, size_t size,
                  unsigned int hash, const char *name,
                  const ElfW(Sym) *sym, const struct link_map *map)
{
  size_t idx = hash % size;
  size_t hash2 = 1 + hash % (size - 2);
  while (table[idx].name != NULL)
    {
      idx += hash2;
      if (idx >= size)
        idx -= size;
    }

  table[idx].hashval = hash;
  table[idx].name = name;
  table[idx].sym = sym;
  table[idx].map = map;
}

/* Utility function for do_lookup_x. Lookup an STB_GNU_UNIQUE symbol
   in the unique symbol table, creating a new entry if necessary.
   Return the matching symbol in RESULT.  */
static void
do_lookup_unique (const char *undef_name, uint_fast32_t new_hash,
		  const struct link_map *map, struct sym_val *result,
		  int type_class, const ElfW(Sym) *sym, const char *strtab,
		  const ElfW(Sym) *ref, const struct link_map *undef_map)
{
  /* We have to determine whether we already found a symbol with this
     name before.  If not then we have to add it to the search table.
     If we already found a definition we have to use it.  */

  struct unique_sym_table *tab
    = &GL(dl_ns)[map->l_ns]._ns_unique_sym_table;

  __rtld_lock_lock_recursive (tab->lock);

  struct unique_sym *entries = tab->entries;
  size_t size = tab->size;
  if (entries != NULL)
    {
      size_t idx = new_hash % size;
      size_t hash2 = 1 + new_hash % (size - 2);
      while (1)
	{
	  if (entries[idx].hashval == new_hash
	      && strcmp (entries[idx].name, undef_name) == 0)
	    {
	      if ((type_class & ELF_RTYPE_CLASS_COPY) != 0)
		{
		  /* We possibly have to initialize the central
		     copy from the copy addressed through the
		     relocation.  */
		  result->s = sym;
		  result->m = (struct link_map *) map;
		}
	      else
		{
		  result->s = entries[idx].sym;
		  result->m = (struct link_map *) entries[idx].map;
		}
	      __rtld_lock_unlock_recursive (tab->lock);
	      return;
	    }

	  if (entries[idx].name == NULL)
	    break;

	  idx += hash2;
	  if (idx >= size)
	    idx -= size;
	}

      if (size * 3 <= tab->n_elements * 4)
	{
	  /* Expand the table.  */
#ifdef RTLD_CHECK_FOREIGN_CALL
	  /* This must not happen during runtime relocations.  */
	  assert (!RTLD_CHECK_FOREIGN_CALL);
#endif
	  size_t newsize = _dl_higher_prime_number (size + 1);
	  struct unique_sym *newentries
	    = calloc (sizeof (struct unique_sym), newsize);
	  if (newentries == NULL)
	    {
	    nomem:
	      __rtld_lock_unlock_recursive (tab->lock);
	      _dl_fatal_printf ("out of memory\n");
	    }

	  for (idx = 0; idx < size; ++idx)
	    if (entries[idx].name != NULL)
	      enter_unique_sym (newentries, newsize, entries[idx].hashval,
                                entries[idx].name, entries[idx].sym,
                                entries[idx].map);

	  tab->free (entries);
	  tab->size = newsize;
	  size = newsize;
	  entries = tab->entries = newentries;
	  tab->free = free;
	}
    }
  else
    {
#ifdef RTLD_CHECK_FOREIGN_CALL
      /* This must not happen during runtime relocations.  */
      assert (!RTLD_CHECK_FOREIGN_CALL);
#endif

#ifdef SHARED
      /* If tab->entries is NULL, but tab->size is not, it means
	 this is the second, conflict finding, lookup for
	 LD_TRACE_PRELINKING in _dl_debug_bindings.  Don't
	 allocate anything and don't enter anything into the
	 hash table.  */
      if (__glibc_unlikely (tab->size))
	{
	  assert (GLRO(dl_debug_mask) & DL_DEBUG_PRELINK);
	  goto success;
	}
#endif

#define INITIAL_NUNIQUE_SYM_TABLE 31
      size = INITIAL_NUNIQUE_SYM_TABLE;
      entries = calloc (sizeof (struct unique_sym), size);
      if (entries == NULL)
	goto nomem;

      tab->entries = entries;
      tab->size = size;
      tab->free = free;
    }

  if ((type_class & ELF_RTYPE_CLASS_COPY) != 0)
    enter_unique_sym (entries, size, new_hash, strtab + sym->st_name, ref,
	   undef_map);
  else
    {
      enter_unique_sym (entries, size,
                        new_hash, strtab + sym->st_name, sym, map);

      if (map->l_type == lt_loaded)
	/* Make sure we don't unload this object by
	   setting the appropriate flag.  */
	((struct link_map *) map)->l_flags_1 |= DF_1_NODELETE;
    }
  ++tab->n_elements;

#ifdef SHARED
 success:
#endif
  __rtld_lock_unlock_recursive (tab->lock);

  result->s = sym;
  result->m = (struct link_map *) map;
}


/* A hash table used to speed up lookups when we have a lot of shared
   libraries.  We record in the table how many ELF objects in the link
   map we can safely skip, because they are known to not contain the symbol
   we are looking for.

   We go through each object (executable or DSO) in their order in the
   main_map and insert pos_in_main_map into the table.  Searches for
   symbols not present in the table start at num_objects, skipping every
   object preprocessed in this way.

   If there is already an earlier (smaller) entry in the table, we leave it
   alone.  It represents an earlier instance of the same symbol (eg weak
   __stdout definition in multiple objects).  */

struct position_hash
{
  /* The Bernstein hash, shifted right one bit.  (This lets us build a table
     directly from the precomputed values in DT_GNU_HASH sections, in which
     the low bit is repurposed to terminate hash chains.)  */
  uint32_t hash;
  /* The position of the first library with this symbol in the search list.  */
  uint32_t pos;
  /* The symbol name.  */
  const char *name;
};

static struct position_hash *position_hash_table;
static size_t position_hash_table_slots;
static size_t position_hash_table_slots_occupied;
static int position_hash_lookup_default;

static int
position_hash_put (struct position_hash *table, size_t slots,
                   struct position_hash *item)
{
  size_t mask = slots - 1;
  /* Search for a matching entry or a free slot.  This loop always terminates
     because we don't allow the hashtable to become completely full.  */
  for (size_t stride = 0, i = item->hash; ; i += ++stride)
    {
      struct position_hash *slot = &table[i & mask];
      if (slot->name == NULL)
        {
          if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
            _dl_debug_printf ("fastload:    put %s hash 0x%x pos %u slot 0x%lx\n",
                              item->name, item->hash, item->pos, slot - table);
          slot->hash = item->hash;
          slot->pos = item->pos;
          slot->name = item->name;
          return 1;
        }
      else if (slot->hash == item->hash &&
               (slot->name == item->name || !strcmp (slot->name, item->name)))
        {
          if (item->pos < slot->pos)
            slot->pos = item->pos;
          if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
            _dl_debug_printf ("fastload:    dup %s hash 0x%x pos %u slot 0x%lx\n",
                              item->name, item->hash, slot->pos, slot - table);
          return 0;
        }
    }
}

static void
position_hash_resize (struct position_hash *oldtable,
                      size_t oldslots, size_t newslots)
{
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
    _dl_debug_printf ("fastload: resizing hashtable to %lu slots\n", newslots);
  assert (!oldtable == !oldslots);
  void *ptr = mmap (NULL, newslots * sizeof *oldtable, PROT_READ|PROT_WRITE,
                    MAP_ANON|MAP_PRIVATE|MAP_POPULATE, -1, 0);
  if (ptr == MAP_FAILED)
    _dl_signal_error (errno, NULL, NULL, "cannot mmap fastload hashtable");
  struct position_hash *newtable = position_hash_table = ptr;
  position_hash_table_slots = newslots;
  if (oldtable == NULL)
    return;
  for (size_t i = 0; i < oldslots; ++i)
    {
      if (oldtable[i].name)
        position_hash_put (newtable, newslots, &oldtable[i]);
    }

  if (munmap (oldtable, oldslots * sizeof *oldtable))
    _dl_signal_error (errno, NULL, NULL, "cannot munmap fastload hashtable");
}

static void
position_hash_init (int lookup_default)
{
  assert (position_hash_table == NULL);
  position_hash_lookup_default = lookup_default;
  position_hash_resize (NULL, 0, 8192);
}

static void
position_hash_insert (uint32_t hash, const char *name, uint32_t pos)
{
  struct position_hash *table = position_hash_table;
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
    _dl_debug_printf ("fastload: insert %s hash 0x%x pos %u\n", name, hash, pos);
  size_t slots = position_hash_table_slots;
  struct position_hash item = { hash, pos, name };
  if (!position_hash_put (table, slots, &item))
    return;
  size_t newsize = ++position_hash_table_slots_occupied;
  if (newsize >= slots / 2)
    /* The load factor has reached 50%.  Double the table size and rehash.  */
    position_hash_resize (table, slots, 2 * slots);
}

static int
position_hash_lookup (uint32_t hash, const char *name)
{
  struct position_hash *table = position_hash_table;
  if (table == NULL)
    return 0;
  size_t mask = position_hash_table_slots - 1;
  for (size_t stride = 0, i = hash; ; i += ++stride)
    {
      struct position_hash *slot = &table[i & mask];
      if (slot->hash == hash &&
          (slot->name == name || !strcmp (slot->name, name)))
        {
          if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
            _dl_debug_printf ("fastload:  found %s at slot 0x%lx, pos %u\n",
                              name, slot - table, slot->pos);
          return slot->pos;
        }
      else if (slot->name == NULL)
        {
          if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
            _dl_debug_printf ("fastload: missed %s at slot 0x%lx, default pos %u\n",
                              name, slot - table, position_hash_lookup_default);
          return position_hash_lookup_default;
        }
    }
}

static int
position_hash_include_symbol (const ElfW(Sym) *sym)
{
  if (sym->st_value == 0 && ELFW(ST_TYPE) (sym->st_info) != STT_TLS)
    return 0;

  switch (ELFW(ST_TYPE) (sym->st_info))
    {
    case STT_SECTION:
    case STT_FILE:
    case STT_COMMON:
      return 0;
    }

  /* Local symbols are ignored.  */
  return ELFW(ST_BIND) (sym->st_info) != STB_LOCAL;
}

static int
last_gnu_hash_bucket (const struct link_map *map)
{
  /* The best documentation for GNU_HASH I found:
     http://www.linker-aliens.org/blogs/ali/entry/gnu_hash_elf_sections/

     _dl_setup_hash() has already set things up for us.  */

  if (__builtin_expect (map->l_nbuckets < 1, 0))
    /* Paranoia: neither gold nor gnu-ld will construct an empty
       .gnu.hash, but some other linker just might.  */
    return 0;

  /* In the GNU hash the symbol index of a symbol is determined by
     the offset of the symbol's entry in the hash table itself.

     We start at the last bucket map->l_gnu_buckets[map->l_nbuckets-1]
     and loop backward from the end until we find a bucket which is
     not zero.  */

  int last_bucket_idx = map->l_nbuckets - 1;

  while (last_bucket_idx > 0 &&
         map->l_gnu_buckets[last_bucket_idx] == 0)
    --last_bucket_idx;

  return map->l_gnu_buckets[last_bucket_idx];
}

static void
position_hash_fill_from_gnu_hash (const struct link_map *map,
                                  int pos_in_main_map)
{
  int last_bucket = last_gnu_hash_bucket (map);
  if (last_bucket == 0)
    return;

  /* Start of hash values.  */
  const Elf32_Word *chains = &map->l_gnu_buckets[map->l_nbuckets];

  /* Reconstruct the number of symbols omitted.  */
  const Elf32_Word *zero = map->l_gnu_chain_zero;
  int symbias = chains - zero;

  const ElfW(Sym) *const symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
  const char *const strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);

  /* Iterate through the symbols in the GNU hash chains and insert them
     into the position hash table.  If the symbol wouldn't pass the tests
     in do_lookup_x(), then don't insert it.  */
  for (int k = symbias; ; ++k)
    {
      uint32_t hash = zero[k];
      const ElfW(Sym) *sym = &symtab[k];
      if (position_hash_include_symbol (sym))
        position_hash_insert (hash >> 1, strtab + sym->st_name, pos_in_main_map);

      /* Stop when we've passed the start of the last chain and the sentinel bit
         terminating a chain is set.  This is the end of the section.  */
      if (k >= last_bucket && (hash & 1))
        break;
    }
}

static int
old_hash_nchain (const struct link_map *map)
{
  if (map->l_info[DT_HASH] == NULL)
    return 0;
  const Elf_Symndx *const hash = (void *) D_PTR (map, l_info[DT_HASH]);
  if (hash == NULL)
    return 0;
  /* Read the "nchain" field of the DT_HASH section; see
     https://refspecs.linuxfoundation.org/elf/gabi4+/ch5.dynamic.html#hash. */
  return hash[1];
}

static void
position_hash_fill_from_symtab (const struct link_map *const map,
                                int pos_in_main_map)
{
  const ElfW(Sym) *const symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
  const char *const strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
  const int num_symbols = old_hash_nchain (map);

  /* Iterate through the symbols defined in this map and insert them
     into the position hash table.  If the symbol wouldn't pass the tests
     in do_lookup_x(), then don't insert it.  */
  for (int k = 0; k < num_symbols; k++)
    {
      const ElfW(Sym) *sym = &symtab[k];
      if (position_hash_include_symbol (sym))
        {
          uint32_t hash = dl_new_hash (strtab + sym->st_name);
          position_hash_insert (hash >> 1, strtab + sym->st_name, pos_in_main_map);
        }
    }
}

/* Create the fastload position hash (if requested).  Given a link_map
   containing all required objects, iterate through all symbols. For each
   symbol which could match in do_lookup_x, insert the index of the
   providing object in main_map.  */

void
_dl_fill_position_hash (struct link_map *main_map)
{
  const int num_objects = main_map->l_searchlist.r_nlist;
  const int debug_fastload = GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD;
  int k;

  /* If we have more than dl_position_hash_cutoff shared libraries,
     fill in the hash table of earliest known positions for symbols in
     main_map.  We'll use this in do_lookup_x().  */

  /* If cutoff is negative, fastload is disabled.  */
  if (GLRO(dl_position_hash_cutoff) < 0)
    {
      if (__builtin_expect (debug_fastload, 0))
        _dl_debug_printf ("fastload: disabled (configuration)\n");
      return;
    }

  /* If we don't have enough mapped objects, fastload is disabled.  */
  if (num_objects <= GLRO(dl_position_hash_cutoff))
    {
      if (__builtin_expect (debug_fastload, 0))
        _dl_debug_printf ("fastload: disabled (too few objects,"
                          " %u <= cutoff %u)\n",
                          num_objects, GLRO(dl_position_hash_cutoff));
      return;
    }

  int timing = (HP_TIMING_AVAIL) && (GLRO(dl_debug_mask) & DL_DEBUG_STATISTICS);
  hp_timing_t start, stop, elapsed;
  if (timing)
    HP_TIMING_NOW (start);

  position_hash_init (num_objects);

  for (k = 0; k < num_objects; ++k)
    {
      const struct link_map *const map = main_map->l_searchlist.r_list[k];

      if (map->l_info[ADDRIDX (DT_GNU_HASH)] != NULL)
        position_hash_fill_from_gnu_hash (map, k);
      else
        position_hash_fill_from_symtab (map, k);

    }

  if (!timing)
    return;
  HP_TIMING_NOW (stop);
  HP_TIMING_DIFF (elapsed, start, stop);
  char buf[80];
  HP_TIMING_PRINT (buf, sizeof buf, elapsed);
  _dl_debug_printf ("\t  time to build fastload table: %s\n", buf);
}

/* Inner part of the lookup functions.  We return a value > 0 if we
   found the symbol, the value 0 if nothing is found and < 0 if
   something bad happened.  */
static int
__attribute_noinline__
do_lookup_x (const char *undef_name, uint_fast32_t new_hash,
	     unsigned long int *old_hash, const ElfW(Sym) *ref,
	     struct sym_val *result, struct r_scope_elem *scope, size_t i,
	     const struct r_found_version *const version, int flags,
	     struct link_map *skip, int type_class, struct link_map *undef_map)
{
  size_t n = scope->r_nlist;
  /* Make sure we read the value before proceeding.  Otherwise we
     might use r_list pointing to the initial scope and r_nlist being
     the value after a resize.  That is the only path in dl-open.c not
     protected by GSCOPE.  A read barrier here might be to expensive.  */
  __asm volatile ("" : "+r" (n), "+m" (scope->r_list));
  struct link_map **list = scope->r_list;

  if (scope == GL(dl_ns)[LM_ID_BASE]._ns_main_searchlist && i == 0)
    {
      const int skip_to = position_hash_lookup (new_hash >> 1, undef_name);

      if (skip_to < n)
	{
	  i = skip_to;
	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
	    _dl_debug_printf ("fastload: lookup %s skipping to %u\n",
			      undef_name, (unsigned int) i);
	}
      else
	{
	  /* Symbol was not found in any of the initial libraries,
	     and no new libraries have been added.  */

	  assert (skip_to == n);

	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
	    _dl_debug_printf ("fastload: lookup %s, %u >= %u (scope->r_nlist)\n",
			      undef_name, (unsigned int) skip_to,
			      (unsigned int) n);
	  return 0;
	}
    }

  do
    {
      const struct link_map *map = list[i]->l_real;

      /* Here come the extra test needed for `_dl_lookup_symbol_skip'.  */
      if (map == skip)
	continue;

      /* Don't search the executable when resolving a copy reloc.  */
      if ((type_class & ELF_RTYPE_CLASS_COPY) && map->l_type == lt_executable)
	continue;

      /* Do not look into objects which are going to be removed.  */
      if (map->l_removed)
	continue;

      /* Print some debugging info if wanted.  */
      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS))
	_dl_debug_printf ("symbol=%s;  lookup in file=%s [%lu]\n",
			  undef_name, DSO_FILENAME (map->l_name),
			  map->l_ns);

      /* If the hash table is empty there is nothing to do here.  */
      if (map->l_nbuckets == 0)
	continue;

      Elf_Symndx symidx;
      int num_versions = 0;
      const ElfW(Sym) *versioned_sym = NULL;

      /* The tables for this map.  */
      const ElfW(Sym) *symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
      const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);

      const ElfW(Sym) *sym;
      const ElfW(Addr) *bitmask = map->l_gnu_bitmask;
      if (__glibc_likely (bitmask != NULL))
	{
	  ElfW(Addr) bitmask_word
	    = bitmask[(new_hash / __ELF_NATIVE_CLASS)
		      & map->l_gnu_bitmask_idxbits];

	  unsigned int hashbit1 = new_hash & (__ELF_NATIVE_CLASS - 1);
	  unsigned int hashbit2 = ((new_hash >> map->l_gnu_shift)
				   & (__ELF_NATIVE_CLASS - 1));

	  if (__glibc_unlikely ((bitmask_word >> hashbit1)
				& (bitmask_word >> hashbit2) & 1))
	    {
	      Elf32_Word bucket = map->l_gnu_buckets[new_hash
						     % map->l_nbuckets];
	      if (bucket != 0)
		{
		  const Elf32_Word *hasharr = &map->l_gnu_chain_zero[bucket];

		  do
		    if (((*hasharr ^ new_hash) >> 1) == 0)
		      {
			symidx = hasharr - map->l_gnu_chain_zero;
			sym = check_match (undef_name, ref, version, flags,
					   type_class, &symtab[symidx], symidx,
					   strtab, map, &versioned_sym,
					   &num_versions);
			if (sym != NULL)
			  goto found_it;
		      }
		  while ((*hasharr++ & 1u) == 0);
		}
	    }
	  /* No symbol found.  */
	  symidx = SHN_UNDEF;
	}
      else
	{
	  if (*old_hash == 0xffffffff)
	    *old_hash = _dl_elf_hash (undef_name);

	  /* Use the old SysV-style hash table.  Search the appropriate
	     hash bucket in this object's symbol table for a definition
	     for the same symbol name.  */
	  for (symidx = map->l_buckets[*old_hash % map->l_nbuckets];
	       symidx != STN_UNDEF;
	       symidx = map->l_chain[symidx])
	    {
	      sym = check_match (undef_name, ref, version, flags,
				 type_class, &symtab[symidx], symidx,
				 strtab, map, &versioned_sym,
				 &num_versions);
	      if (sym != NULL)
		goto found_it;
	    }
	}

      /* If we have seen exactly one versioned symbol while we are
	 looking for an unversioned symbol and the version is not the
	 default version we still accept this symbol since there are
	 no possible ambiguities.  */
      sym = num_versions == 1 ? versioned_sym : NULL;

      if (sym != NULL)
	{
	found_it:
	  /* When UNDEF_MAP is NULL, which indicates we are called from
	     do_lookup_x on relocation against protected data, we skip
	     the data definion in the executable from copy reloc.  */
	  if (ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA
	      && undef_map == NULL
	      && map->l_type == lt_executable
	      && type_class == ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA)
	    {
	      const ElfW(Sym) *s;
	      unsigned int i;

#if ! ELF_MACHINE_NO_RELA
	      if (map->l_info[DT_RELA] != NULL
		  && map->l_info[DT_RELASZ] != NULL
		  && map->l_info[DT_RELASZ]->d_un.d_val != 0)
		{
		  const ElfW(Rela) *rela
		    = (const ElfW(Rela) *) D_PTR (map, l_info[DT_RELA]);
		  unsigned int rela_count
		    = map->l_info[DT_RELASZ]->d_un.d_val / sizeof (*rela);

		  for (i = 0; i < rela_count; i++, rela++)
		    if (elf_machine_type_class (ELFW(R_TYPE) (rela->r_info))
			== ELF_RTYPE_CLASS_COPY)
		      {
			s = &symtab[ELFW(R_SYM) (rela->r_info)];
			if (!strcmp (strtab + s->st_name, undef_name))
			  goto skip;
		      }
		}
#endif
#if ! ELF_MACHINE_NO_REL
	      if (map->l_info[DT_REL] != NULL
		  && map->l_info[DT_RELSZ] != NULL
		  && map->l_info[DT_RELSZ]->d_un.d_val != 0)
		{
		  const ElfW(Rel) *rel
		    = (const ElfW(Rel) *) D_PTR (map, l_info[DT_REL]);
		  unsigned int rel_count
		    = map->l_info[DT_RELSZ]->d_un.d_val / sizeof (*rel);

		  for (i = 0; i < rel_count; i++, rel++)
		    if (elf_machine_type_class (ELFW(R_TYPE) (rel->r_info))
			== ELF_RTYPE_CLASS_COPY)
		      {
			s = &symtab[ELFW(R_SYM) (rel->r_info)];
			if (!strcmp (strtab + s->st_name, undef_name))
			  goto skip;
		      }
		}
#endif
	    }

	  /* Hidden and internal symbols are local, ignore them.  */
	  if (__glibc_unlikely (dl_symbol_visibility_binds_local_p (sym)))
	    goto skip;

	  switch (ELFW(ST_BIND) (sym->st_info))
	    {
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find another.  */
	      if (__glibc_unlikely (GLRO(dl_dynamic_weak)))
		{
		  if (! result->s)
		    {
		      result->s = sym;
		      result->m = (struct link_map *) map;
		    }
		  break;
		}
	      /* FALLTHROUGH */
	    case STB_GLOBAL:
	      /* Global definition.  Just what we need.  */
	      result->s = sym;
	      result->m = (struct link_map *) map;
	      return 1;

	    case STB_GNU_UNIQUE:;
	      do_lookup_unique (undef_name, new_hash, map, result, type_class,
				sym, strtab, ref, undef_map);
	      return 1;

	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }
	}

skip:
      /* If this current map is the one mentioned in the verneed entry
	 and we have not found a weak entry, it is a bug.  */
      if (symidx == STN_UNDEF && version != NULL && version->filename != NULL
	  && __glibc_unlikely (_dl_name_match_p (version->filename, map)))
	return -1;
    }
  while (++i < n);

  /* We have not found anything until now.  */
  return 0;
}


/* Add extra dependency on MAP to UNDEF_MAP.  */
static int
add_dependency (struct link_map *undef_map, struct link_map *map, int flags)
{
  struct link_map *runp;
  unsigned int i;
  int result = 0;

  /* Avoid self-references and references to objects which cannot be
     unloaded anyway.  */
  if (undef_map == map)
    return 0;

  /* Avoid references to objects which cannot be unloaded anyway.  */
  assert (map->l_type == lt_loaded);
  if ((map->l_flags_1 & DF_1_NODELETE) != 0)
    return 0;

  struct link_map_reldeps *l_reldeps
    = atomic_forced_read (undef_map->l_reldeps);

  /* Make sure l_reldeps is read before l_initfini.  */
  atomic_read_barrier ();

  /* Determine whether UNDEF_MAP already has a reference to MAP.  First
     look in the normal dependencies.  */
  struct link_map **l_initfini = atomic_forced_read (undef_map->l_initfini);
  if (l_initfini != NULL)
    {
      for (i = 0; l_initfini[i] != NULL; ++i)
	if (l_initfini[i] == map)
	  return 0;
    }

  /* No normal dependency.  See whether we already had to add it
     to the special list of dynamic dependencies.  */
  unsigned int l_reldepsact = 0;
  if (l_reldeps != NULL)
    {
      struct link_map **list = &l_reldeps->list[0];
      l_reldepsact = l_reldeps->act;
      for (i = 0; i < l_reldepsact; ++i)
	if (list[i] == map)
	  return 0;
    }

  /* Save serial number of the target MAP.  */
  unsigned long long serial = map->l_serial;

  /* Make sure nobody can unload the object while we are at it.  */
  if (__glibc_unlikely (flags & DL_LOOKUP_GSCOPE_LOCK))
    {
      /* We can't just call __rtld_lock_lock_recursive (GL(dl_load_lock))
	 here, that can result in ABBA deadlock.  */
      THREAD_GSCOPE_RESET_FLAG ();
      __rtld_lock_lock_recursive (GL(dl_load_lock));
      /* While MAP value won't change, after THREAD_GSCOPE_RESET_FLAG ()
	 it can e.g. point to unallocated memory.  So avoid the optimizer
	 treating the above read from MAP->l_serial as ensurance it
	 can safely dereference it.  */
      map = atomic_forced_read (map);

      /* From this point on it is unsafe to dereference MAP, until it
	 has been found in one of the lists.  */

      /* Redo the l_initfini check in case undef_map's l_initfini
	 changed in the mean time.  */
      if (undef_map->l_initfini != l_initfini
	  && undef_map->l_initfini != NULL)
	{
	  l_initfini = undef_map->l_initfini;
	  for (i = 0; l_initfini[i] != NULL; ++i)
	    if (l_initfini[i] == map)
	      goto out_check;
	}

      /* Redo the l_reldeps check if undef_map's l_reldeps changed in
	 the mean time.  */
      if (undef_map->l_reldeps != NULL)
	{
	  if (undef_map->l_reldeps != l_reldeps)
	    {
	      struct link_map **list = &undef_map->l_reldeps->list[0];
	      l_reldepsact = undef_map->l_reldeps->act;
	      for (i = 0; i < l_reldepsact; ++i)
		if (list[i] == map)
		  goto out_check;
	    }
	  else if (undef_map->l_reldeps->act > l_reldepsact)
	    {
	      struct link_map **list
		= &undef_map->l_reldeps->list[0];
	      i = l_reldepsact;
	      l_reldepsact = undef_map->l_reldeps->act;
	      for (; i < l_reldepsact; ++i)
		if (list[i] == map)
		  goto out_check;
	    }
	}
    }
  else
    __rtld_lock_lock_recursive (GL(dl_load_lock));

  /* The object is not yet in the dependency list.  Before we add
     it make sure just one more time the object we are about to
     reference is still available.  There is a brief period in
     which the object could have been removed since we found the
     definition.  */
  runp = GL(dl_ns)[undef_map->l_ns]._ns_loaded;
  while (runp != NULL && runp != map)
    runp = runp->l_next;

  if (runp != NULL)
    {
      /* The object is still available.  */

      /* MAP could have been dlclosed, freed and then some other dlopened
	 library could have the same link_map pointer.  */
      if (map->l_serial != serial)
	goto out_check;

      /* Redo the NODELETE check, as when dl_load_lock wasn't held
	 yet this could have changed.  */
      if ((map->l_flags_1 & DF_1_NODELETE) != 0)
	goto out;

      /* If the object with the undefined reference cannot be removed ever
	 just make sure the same is true for the object which contains the
	 definition.  */
      if (undef_map->l_type != lt_loaded
	  || (undef_map->l_flags_1 & DF_1_NODELETE) != 0)
	{
	  map->l_flags_1 |= DF_1_NODELETE;
	  goto out;
	}

      /* Add the reference now.  */
      if (__glibc_unlikely (l_reldepsact >= undef_map->l_reldepsmax))
	{
	  /* Allocate more memory for the dependency list.  Since this
	     can never happen during the startup phase we can use
	     `realloc'.  */
	  struct link_map_reldeps *newp;
	  unsigned int max
	    = undef_map->l_reldepsmax ? undef_map->l_reldepsmax * 2 : 10;

#ifdef RTLD_PREPARE_FOREIGN_CALL
	  RTLD_PREPARE_FOREIGN_CALL;
#endif

	  newp = malloc (sizeof (*newp) + max * sizeof (struct link_map *));
	  if (newp == NULL)
	    {
	      /* If we didn't manage to allocate memory for the list this is
		 no fatal problem.  We simply make sure the referenced object
		 cannot be unloaded.  This is semantically the correct
		 behavior.  */
	      map->l_flags_1 |= DF_1_NODELETE;
	      goto out;
	    }
	  else
	    {
	      if (l_reldepsact)
		memcpy (&newp->list[0], &undef_map->l_reldeps->list[0],
			l_reldepsact * sizeof (struct link_map *));
	      newp->list[l_reldepsact] = map;
	      newp->act = l_reldepsact + 1;
	      atomic_write_barrier ();
	      void *old = undef_map->l_reldeps;
	      undef_map->l_reldeps = newp;
	      undef_map->l_reldepsmax = max;
	      if (old)
		_dl_scope_free (old);
	    }
	}
      else
	{
	  undef_map->l_reldeps->list[l_reldepsact] = map;
	  atomic_write_barrier ();
	  undef_map->l_reldeps->act = l_reldepsact + 1;
	}

      /* Display information if we are debugging.  */
      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_FILES))
	_dl_debug_printf ("\
\nfile=%s [%lu];  needed by %s [%lu] (relocation dependency)\n\n",
			  DSO_FILENAME (map->l_name),
			  map->l_ns,
			  DSO_FILENAME (undef_map->l_name),
			  undef_map->l_ns);
    }
  else
    /* Whoa, that was bad luck.  We have to search again.  */
    result = -1;

 out:
  /* Release the lock.  */
  __rtld_lock_unlock_recursive (GL(dl_load_lock));

  if (__glibc_unlikely (flags & DL_LOOKUP_GSCOPE_LOCK))
    THREAD_GSCOPE_SET_FLAG ();

  return result;

 out_check:
  if (map->l_serial != serial)
    result = -1;
  goto out;
}

static void
_dl_debug_bindings (const char *undef_name, struct link_map *undef_map,
		    const ElfW(Sym) **ref, struct sym_val *value,
		    const struct r_found_version *version, int type_class,
		    int protected);


/* Search loaded objects' symbol tables for a definition of the symbol
   UNDEF_NAME, perhaps with a requested version for the symbol.

   We must never have calls to the audit functions inside this function
   or in any function which gets called.  If this would happen the audit
   code might create a thread which can throw off all the scope locking.  */
lookup_t
_dl_lookup_symbol_x (const char *undef_name, struct link_map *undef_map,
		     const ElfW(Sym) **ref,
		     struct r_scope_elem *symbol_scope[],
		     const struct r_found_version *version,
		     int type_class, int flags, struct link_map *skip_map)
{
  const uint_fast32_t new_hash = dl_new_hash (undef_name);
  unsigned long int old_hash = 0xffffffff;
  struct sym_val current_value = { NULL, NULL };
  struct r_scope_elem **scope = symbol_scope;

  bump_num_relocations ();

  /* No other flag than DL_LOOKUP_ADD_DEPENDENCY or DL_LOOKUP_GSCOPE_LOCK
     is allowed if we look up a versioned symbol.  */
  assert (version == NULL
	  || (flags & ~(DL_LOOKUP_ADD_DEPENDENCY | DL_LOOKUP_GSCOPE_LOCK))
	     == 0);

  size_t i = 0;
  if (__glibc_unlikely (skip_map != NULL))
    /* Search the relevant loaded objects for a definition.  */
    while ((*scope)->r_list[i] != skip_map)
      ++i;

  /* Search the relevant loaded objects for a definition.  */
  for (size_t start = i; *scope != NULL; start = 0, ++scope)
    {
      int res = do_lookup_x (undef_name, new_hash, &old_hash, *ref,
			     &current_value, *scope, start, version, flags,
			     skip_map, type_class, undef_map);
      if (res > 0)
	break;

      if (__glibc_unlikely (res < 0) && skip_map == NULL)
	{
	  /* Oh, oh.  The file named in the relocation entry does not
	     contain the needed symbol.  This code is never reached
	     for unversioned lookups.  */
	  assert (version != NULL);
	  const char *reference_name = undef_map ? undef_map->l_name : "";
	  struct dl_exception exception;
	  /* XXX We cannot translate the message.  */
	  _dl_exception_create_format
	    (&exception, DSO_FILENAME (reference_name),
	     "symbol %s version %s not defined in file %s"
	     " with link time reference%s",
	     undef_name, version->name, version->filename,
	     res == -2 ? " (no version symbols)" : "");
	  _dl_signal_cexception (0, &exception, N_("relocation error"));
	  _dl_exception_free (&exception);
	  *ref = NULL;
	  return 0;
	}
    }

  if (__glibc_unlikely (current_value.s == NULL))
    {
      if ((*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	  && !(GLRO(dl_debug_mask) & DL_DEBUG_UNUSED))
	{
	  /* We could find no value for a strong reference.  */
	  const char *reference_name = undef_map ? undef_map->l_name : "";
	  const char *versionstr = version ? ", version " : "";
	  const char *versionname = (version && version->name
				     ? version->name : "");
	  struct dl_exception exception;
	  /* XXX We cannot translate the message.  */
	  _dl_exception_create_format
	    (&exception, DSO_FILENAME (reference_name),
	     "undefined symbol: %s%s%s",
	     undef_name, versionstr, versionname);
	  _dl_signal_cexception (0, &exception, N_("symbol lookup error"));
	  _dl_exception_free (&exception);
	}
      *ref = NULL;
      return 0;
    }

  int protected = (*ref
		   && ELFW(ST_VISIBILITY) ((*ref)->st_other) == STV_PROTECTED);
  if (__glibc_unlikely (protected != 0))
    {
      /* It is very tricky.  We need to figure out what value to
	 return for the protected symbol.  */
      if (type_class == ELF_RTYPE_CLASS_PLT)
	{
	  if (current_value.s != NULL && current_value.m != undef_map)
	    {
	      current_value.s = *ref;
	      current_value.m = undef_map;
	    }
	}
      else
	{
	  struct sym_val protected_value = { NULL, NULL };

	  for (scope = symbol_scope; *scope != NULL; i = 0, ++scope)
	    if (do_lookup_x (undef_name, new_hash, &old_hash, *ref,
			     &protected_value, *scope, i, version, flags,
			     skip_map,
			     (ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA
			      && ELFW(ST_TYPE) ((*ref)->st_info) == STT_OBJECT
			      && type_class == ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA)
			     ? ELF_RTYPE_CLASS_EXTERN_PROTECTED_DATA
			     : ELF_RTYPE_CLASS_PLT, NULL) != 0)
	      break;

	  if (protected_value.s != NULL && protected_value.m != undef_map)
	    {
	      current_value.s = *ref;
	      current_value.m = undef_map;
	    }
	}
    }

  /* We have to check whether this would bind UNDEF_MAP to an object
     in the global scope which was dynamically loaded.  In this case
     we have to prevent the latter from being unloaded unless the
     UNDEF_MAP object is also unloaded.  */
  if (__glibc_unlikely (current_value.m->l_type == lt_loaded)
      /* Don't do this for explicit lookups as opposed to implicit
	 runtime lookups.  */
      && (flags & DL_LOOKUP_ADD_DEPENDENCY) != 0
      /* Add UNDEF_MAP to the dependencies.  */
      && add_dependency (undef_map, current_value.m, flags) < 0)
      /* Something went wrong.  Perhaps the object we tried to reference
	 was just removed.  Try finding another definition.  */
      return _dl_lookup_symbol_x (undef_name, undef_map, ref,
				  (flags & DL_LOOKUP_GSCOPE_LOCK)
				  ? undef_map->l_scope : symbol_scope,
				  version, type_class, flags, skip_map);

  /* The object is used.  */
  if (__glibc_unlikely (current_value.m->l_used == 0))
    current_value.m->l_used = 1;

  if (__glibc_unlikely (GLRO(dl_debug_mask)
			& (DL_DEBUG_BINDINGS|DL_DEBUG_PRELINK)))
    _dl_debug_bindings (undef_name, undef_map, ref,
			&current_value, version, type_class, protected);

  *ref = current_value.s;
  return LOOKUP_VALUE (current_value.m);
}


/* Cache the location of MAP's hash table.  */

void
_dl_setup_hash (struct link_map *map)
{
  Elf_Symndx *hash;

  if (__glibc_likely (map->l_info[DT_ADDRTAGIDX (DT_GNU_HASH) + DT_NUM
				    + DT_THISPROCNUM + DT_VERSIONTAGNUM
				    + DT_EXTRANUM + DT_VALNUM] != NULL))
    {
      Elf32_Word *hash32
	= (void *) D_PTR (map, l_info[DT_ADDRTAGIDX (DT_GNU_HASH) + DT_NUM
				      + DT_THISPROCNUM + DT_VERSIONTAGNUM
				      + DT_EXTRANUM + DT_VALNUM]);
      map->l_nbuckets = *hash32++;
      Elf32_Word symbias = *hash32++;
      Elf32_Word bitmask_nwords = *hash32++;
      /* Must be a power of two.  */
      assert ((bitmask_nwords & (bitmask_nwords - 1)) == 0);
      map->l_gnu_bitmask_idxbits = bitmask_nwords - 1;
      map->l_gnu_shift = *hash32++;

      map->l_gnu_bitmask = (ElfW(Addr) *) hash32;
      hash32 += __ELF_NATIVE_CLASS / 32 * bitmask_nwords;

      map->l_gnu_buckets = hash32;
      hash32 += map->l_nbuckets;
      map->l_gnu_chain_zero = hash32 - symbias;
      return;
    }

  if (!map->l_info[DT_HASH])
    return;
  hash = (void *) D_PTR (map, l_info[DT_HASH]);

  map->l_nbuckets = *hash++;
  /* Skip nchain.  */
  hash++;
  map->l_buckets = hash;
  hash += map->l_nbuckets;
  map->l_chain = hash;
}


static void
_dl_debug_bindings (const char *undef_name, struct link_map *undef_map,
		    const ElfW(Sym) **ref, struct sym_val *value,
		    const struct r_found_version *version, int type_class,
		    int protected)
{
  const char *reference_name = undef_map->l_name;

  if (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS)
    {
      _dl_debug_printf ("binding file %s [%lu] to %s [%lu]: %s symbol `%s'",
			DSO_FILENAME (reference_name),
			undef_map->l_ns,
			DSO_FILENAME (value->m->l_name),
			value->m->l_ns,
			protected ? "protected" : "normal", undef_name);
      if (version)
	_dl_debug_printf_c (" [%s]\n", version->name);
      else
	_dl_debug_printf_c ("\n");
    }
#ifdef SHARED
  if (GLRO(dl_debug_mask) & DL_DEBUG_PRELINK)
    {
/* ELF_RTYPE_CLASS_XXX must match RTYPE_CLASS_XXX used by prelink with
   LD_TRACE_PRELINKING.  */
#define RTYPE_CLASS_VALID	8
#define RTYPE_CLASS_PLT		(8|1)
#define RTYPE_CLASS_COPY	(8|2)
#define RTYPE_CLASS_TLS		(8|4)
#if ELF_RTYPE_CLASS_PLT != 0 && ELF_RTYPE_CLASS_PLT != 1
# error ELF_RTYPE_CLASS_PLT must be 0 or 1!
#endif
#if ELF_RTYPE_CLASS_COPY != 0 && ELF_RTYPE_CLASS_COPY != 2
# error ELF_RTYPE_CLASS_COPY must be 0 or 2!
#endif
      int conflict = 0;
      struct sym_val val = { NULL, NULL };

      if ((GLRO(dl_trace_prelink_map) == NULL
	   || GLRO(dl_trace_prelink_map) == GL(dl_ns)[LM_ID_BASE]._ns_loaded)
	  && undef_map != GL(dl_ns)[LM_ID_BASE]._ns_loaded)
	{
	  const uint_fast32_t new_hash = dl_new_hash (undef_name);
	  unsigned long int old_hash = 0xffffffff;
	  struct unique_sym *saved_entries
	    = GL(dl_ns)[LM_ID_BASE]._ns_unique_sym_table.entries;

	  GL(dl_ns)[LM_ID_BASE]._ns_unique_sym_table.entries = NULL;
	  do_lookup_x (undef_name, new_hash, &old_hash, *ref, &val,
		       undef_map->l_local_scope[0], 0, version, 0, NULL,
		       type_class, undef_map);
	  if (val.s != value->s || val.m != value->m)
	    conflict = 1;
	  else if (__glibc_unlikely (undef_map->l_symbolic_in_local_scope)
		   && val.s
		   && __glibc_unlikely (ELFW(ST_BIND) (val.s->st_info)
					== STB_GNU_UNIQUE))
	    {
	      /* If it is STB_GNU_UNIQUE and undef_map's l_local_scope
		 contains any DT_SYMBOLIC libraries, unfortunately there
		 can be conflicts even if the above is equal.  As symbol
		 resolution goes from the last library to the first and
		 if a STB_GNU_UNIQUE symbol is found in some late DT_SYMBOLIC
		 library, it would be the one that is looked up.  */
	      struct sym_val val2 = { NULL, NULL };
	      size_t n;
	      struct r_scope_elem *scope = undef_map->l_local_scope[0];

	      for (n = 0; n < scope->r_nlist; n++)
		if (scope->r_list[n] == val.m)
		  break;

	      for (n++; n < scope->r_nlist; n++)
		if (scope->r_list[n]->l_info[DT_SYMBOLIC] != NULL
		    && do_lookup_x (undef_name, new_hash, &old_hash, *ref,
				    &val2,
				    &scope->r_list[n]->l_symbolic_searchlist,
				    0, version, 0, NULL, type_class,
				    undef_map) > 0)
		  {
		    conflict = 1;
		    val = val2;
		    break;
		  }
	    }
	  GL(dl_ns)[LM_ID_BASE]._ns_unique_sym_table.entries = saved_entries;
	}

      if (value->s)
	{
	  /* Keep only ELF_RTYPE_CLASS_PLT and ELF_RTYPE_CLASS_COPY
	     bits since since prelink only uses them.  */
	  type_class &= ELF_RTYPE_CLASS_PLT | ELF_RTYPE_CLASS_COPY;
	  if (__glibc_unlikely (ELFW(ST_TYPE) (value->s->st_info)
				== STT_TLS))
	    /* Clear the RTYPE_CLASS_VALID bit in RTYPE_CLASS_TLS.  */
	    type_class = RTYPE_CLASS_TLS & ~RTYPE_CLASS_VALID;
	  else if (__glibc_unlikely (ELFW(ST_TYPE) (value->s->st_info)
				     == STT_GNU_IFUNC))
	    /* Set the RTYPE_CLASS_VALID bit.  */
	    type_class |= RTYPE_CLASS_VALID;
	}

      if (conflict
	  || GLRO(dl_trace_prelink_map) == undef_map
	  || GLRO(dl_trace_prelink_map) == NULL
	  || type_class >= 4)
	{
	  _dl_printf ("%s 0x%0*Zx 0x%0*Zx -> 0x%0*Zx 0x%0*Zx ",
		      conflict ? "conflict" : "lookup",
		      (int) sizeof (ElfW(Addr)) * 2,
		      (size_t) undef_map->l_map_start,
		      (int) sizeof (ElfW(Addr)) * 2,
		      (size_t) (((ElfW(Addr)) *ref) - undef_map->l_map_start),
		      (int) sizeof (ElfW(Addr)) * 2,
		      (size_t) (value->s ? value->m->l_map_start : 0),
		      (int) sizeof (ElfW(Addr)) * 2,
		      (size_t) (value->s ? value->s->st_value : 0));

	  if (conflict)
	    _dl_printf ("x 0x%0*Zx 0x%0*Zx ",
			(int) sizeof (ElfW(Addr)) * 2,
			(size_t) (val.s ? val.m->l_map_start : 0),
			(int) sizeof (ElfW(Addr)) * 2,
			(size_t) (val.s ? val.s->st_value : 0));

	  _dl_printf ("/%x %s\n", type_class, undef_name);
	}
    }
#endif
}
