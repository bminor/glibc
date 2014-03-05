/* Look up a symbol in the loaded objects.
   Copyright (C) 1995-2014 Free Software Foundation, Inc.
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
#include <bits/libc-lock.h>
#include <tls.h>
#include <atomic.h>

#include <assert.h>

#define VERSTAG(tag)	(DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGIDX (tag))

#ifndef ADDRIDX
# define ADDRIDX(tag) (DT_NUM + DT_THISPROCNUM + DT_VERSIONTAGNUM \
		       + DT_EXTRANUM + DT_VALNUM + DT_ADDRTAGIDX (tag))
#endif

/* We need this string more than once.  */
static const char undefined_msg[] = "undefined symbol: ";


struct sym_val
  {
    const ElfW(Sym) *s;
    struct link_map *m;
  };


#define make_string(string, rest...) \
  ({									      \
    const char *all[] = { string, ## rest };				      \
    size_t len, cnt;							      \
    char *result, *cp;							      \
									      \
    len = 1;								      \
    for (cnt = 0; cnt < sizeof (all) / sizeof (all[0]); ++cnt)		      \
      len += strlen (all[cnt]);						      \
									      \
    cp = result = alloca (len);						      \
    for (cnt = 0; cnt < sizeof (all) / sizeof (all[0]); ++cnt)		      \
      cp = __stpcpy (cp, all[cnt]);					      \
									      \
    result;								      \
  })

/* Statistics function.  */
#ifdef SHARED
# define bump_num_relocations() ++GL(dl_num_relocations)
#else
# define bump_num_relocations() ((void) 0)
#endif

/* Bob Jenkins's hash function, free for any use.
   http://burtleburtle.net/bob/hash/  */
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

typedef unsigned int uint32 __attribute__ ((mode (SI)));
static uint32
hash_with_seed (const char *k, uint32 length, uint32 seed)
{
   uint32 a, b, c, len;

   /* Set up the internal state */
   len = length;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = seed;            /* the previous hash value */

   /*---------------------------------------- handle most of the key */
   while (len >= 12)
     {
       a += (k[0] +((uint32)k[1]<<8) +((uint32)k[2]<<16) +((uint32)k[3]<<24));
       b += (k[4] +((uint32)k[5]<<8) +((uint32)k[6]<<16) +((uint32)k[7]<<24));
       c += (k[8] +((uint32)k[9]<<8) +((uint32)k[10]<<16)+((uint32)k[11]<<24));
       mix (a,b,c);
       k += 12;
       len -= 12;
     }

   /*------------------------------------- handle the last 11 bytes */
   c += length;
   switch (len)              /* all the case statements fall through */
     {
     case 11: c+=((uint32)k[10]<<24);
     case 10: c+=((uint32)k[9]<<16);
     case 9 : c+=((uint32)k[8]<<8);
         /* the first byte of c is reserved for the length */
     case 8 : b+=((uint32)k[7]<<24);
     case 7 : b+=((uint32)k[6]<<16);
     case 6 : b+=((uint32)k[5]<<8);
     case 5 : b+=k[4];
     case 4 : a+=((uint32)k[3]<<24);
     case 3 : a+=((uint32)k[2]<<16);
     case 2 : a+=((uint32)k[1]<<8);
     case 1 : a+=k[0];
       /* case 0: nothing left to add */
     }
   mix (a,b,c);
   /*-------------------------------------------- report the result */
   return c;
}


/* A hash table used to speed up lookups when we have a lot of shared
   libraries.  We record in the table how many ELF objects in the link
   map we can safely skip, because they are known to not contain the symbol
   we are looking for.

   Values start out at N (num objects in the main_map).  We then go through
   each object (executable or DSO) in their order in the main_map and insert
   pos_in_map_map into the table.

   If there is already an earlier (smaller) entry in the table, we leave it
   alone.  It could be a hash collision, or it could be an earlier
   instance of the same symbol (eg weak __stdout definition in
   multiple objects).

   To save space, the table contains uint16_t entries, and so we can't
   record main map positions over 65535.

   It turns out that even when only 25% of slots are used, collision
   rate is around 10%, which is quite high, and causes lookups to be slow.

   So we use a second table (using a different hash seed). Hash collisions
   using two separate hashes are quite rare.

   Note that we allocate the two tables together (a single block twice
   the size of one table).  */

static void
insert_into_hash_table_1 (int pos_in_main_map, const char *symbol_name,
			  size_t name_len, dl_position_table_entry_t *table,
			  int seed)
{
  uint32 hash = hash_with_seed (symbol_name, name_len, seed);
  int pos = hash & GLRO(dl_position_hash_mask);
  if (pos_in_main_map < table[pos])
    {
      table[pos] = pos_in_main_map;
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
        _dl_debug_printf ("fastload hash for %s: [%u] = %u (table %u)\n",
                          symbol_name, (unsigned int) pos,
                          (unsigned int) table[pos],
                          (GLRO(dl_position_hash_table) == table) ? 0 : 1);
    }
  else
    {
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
        _dl_debug_printf
          ("fastload hash for %s: [%u] already set to %u <= %u (table %u)\n",
           symbol_name, (unsigned int) pos,
           (unsigned int) table[pos],
           (unsigned int) pos_in_main_map,
           (GLRO(dl_position_hash_table) == table) ? 0 : 1);
    }
}

#define HASH_SEED_A 0
#define HASH_SEED_B 5381  /* As good as any other value.  */

static void
insert_into_hash_table (int pos_in_main_map, const char *symbol_name)
{
  dl_position_table_entry_t *const table1 = GLRO(dl_position_hash_table);
  const int table_size = GLRO(dl_position_hash_mask) + 1;
  dl_position_table_entry_t *const table2 = table1 + table_size;
  const size_t name_len = strlen (symbol_name);

  insert_into_hash_table_1 (pos_in_main_map, symbol_name, name_len, table1,
			    HASH_SEED_A);
  insert_into_hash_table_1 (pos_in_main_map, symbol_name, name_len, table2,
			    HASH_SEED_B);
}

static inline int
earliest_pos_from_hash_table_1 (const char *symbol_name,
                                const dl_position_table_entry_t *table,
                                int seed)
{
  int pos;
  uint32 hash;

  hash = hash_with_seed (symbol_name, strlen (symbol_name), seed);
  pos = hash & GLRO(dl_position_hash_mask);
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
    _dl_debug_printf
      ("fastload earliest pos for %s: [%u] = %u (table %u)\n",
       symbol_name, (unsigned int) pos, (unsigned int) table[pos],
       (GLRO(dl_position_hash_table) == table) ? 0 : 1);

  return table[pos];
}

static inline int
earliest_pos_from_hash_table (const char *undef_name)
{
  dl_position_table_entry_t *const table1 = GLRO(dl_position_hash_table);

  if (table1 == NULL)
    return 0;  /* Search from the beginning of the list.  */
  else
    {
      const int table_size = GLRO(dl_position_hash_mask) + 1;
      dl_position_table_entry_t *const table2 = table1 + table_size;
      const int skip_1 = earliest_pos_from_hash_table_1 (undef_name, table1,
                                                         HASH_SEED_A);
      const int skip_2 = earliest_pos_from_hash_table_1 (undef_name, table2,
                                                         HASH_SEED_B);

      /* Possibilities:

       * Both skip_1 and skip_2 are positive and same:
         there were no collisions at insertion in either table, or there
         was double-collision with symbols from the same library.

         If skip_1 == skip_2 == N (num_directly_loaded_objects), then the
         slot was empty in both tables after they were filled (i.e. there
         is no symbol that hashes to corresponding slot).  In that case,
         searching first [0, N) is pointless. See b/5609692.

         True skip_to == skip_1 == skip_2.

       * Both skip_1 and skip_2 are positive but different:
         there was a collision in one or both tables, or one of the slots
         remained unfilled after all symbols have been inserted.
         It is safe to start at max, true skip_to count can not be less
         than max.  */

      return (skip_1 < skip_2) ? skip_2 : skip_1;
    }
}


static void
fill_hash_table (const struct link_map *const map, int pos_in_main_map,
                 int num_symbols)
{
  const ElfW(Sym) *const symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
  const char *const strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
  int k;

  /* Iterate through the symbols defined in this map and insert them
     into the bloom filter.  If the symbol wouldn't pass the tests in
     do_lookup_x(), then don't insert it.  */
  for (k = 0; k < num_symbols; k++)
    {
      const ElfW(Sym) *sym = &symtab[k];

      if (sym->st_value == 0
          && ELFW(ST_TYPE) (sym->st_info) != STT_TLS)
        continue;

      switch (ELFW(ST_TYPE) (sym->st_info))
	{
	case STT_SECTION:
	case STT_FILE:
	case STT_COMMON:
	  continue;
	default:
	  break;
	}

      switch (ELFW(ST_BIND) (sym->st_info))
	{
	case STB_LOCAL:  /* Local symbols are ignored.  */
	  continue;
	default:
	  break;
	}

      insert_into_hash_table (pos_in_main_map, strtab + sym->st_name);
    }
}

static dl_position_table_entry_t *
allocate_position_hash_table (size_t table_size, int max_pos)
{
  /* Implementation detail: we actually allocate two "logical" tables in a
     single block.  */

  const size_t total_size = 2 * table_size;
  dl_position_table_entry_t *table
    = malloc (total_size * sizeof (dl_position_table_entry_t));

  if (table == NULL)
    return NULL;

  for (int k = 0; k < total_size; ++k)
    table[k] = max_pos;

  return table;
}

static int
num_dynsyms (const struct link_map *const map)
{
  if (map->l_info[DT_HASH] != NULL)
    {
      /* If DT_HASH is present, we unconditionally use it, since it already
         has the answer precomputed and waiting for us.  */
      const Elf_Symndx *const hash = (void *) D_PTR (map, l_info[DT_HASH]);

      if (hash != NULL)
	{
	  /* DT_HASH dynamic structure points to a symbol hash table
	     with the following layout:

	     nbucket
	     nchain       (== number of symbols in dynamic symbol table)
	     bucket[0]
	     ...

	     http://refspecs.freestandards.org/elf/TIS1.1.pdf (p. 2.19). */

	  return hash[1];
	}
    }
  else if (map->l_info[ADDRIDX (DT_GNU_HASH)] != NULL)
    {
      /* The best documentation for GNU_HASH I found:
	 http://blogs.sun.com/ali/entry/gnu_hash_elf_sections

	 _dl_setup_hash() has already set things up for us.  */

      if (__builtin_expect (map->l_nbuckets < 1, 0))
	{
	  /* Paranoia: neither gold nor gnu-ld will construct an empty
	     .gnu.hash, but some other linker just might.  */
	  return 0;
	}

      /* In the GNU hash the symbol index of a symbol is determined by
	 the offset of the symbol's entry in the hash table itself.

	 We start at the last bucket map->l_gnu_chain_zero[map->l_nbuckets-1]
	 and loop backward from the end until we find a bucket which is
	 not zero.

	 Then we walk forward to find the last entry in the chain which
	 starts at that bucket.  The terminating entry gives us desired
	 symbol index.  */

      int last_bucket_idx = map->l_nbuckets - 1;

      while (last_bucket_idx > 0 &&
	     map->l_gnu_buckets[last_bucket_idx] == 0)
	{
	  --last_bucket_idx;
	}

      const Elf32_Word last_bucket = map->l_gnu_buckets[last_bucket_idx];

      if (__builtin_expect (last_bucket == 0, 0))
	{
	  /* This shouldn't happen unless the library has only static
	     variables with global ctors, and nothing else.  */
	  return 0;
	}

      /* Start of the hash chain for the last non-zero bucket.  */
      const Elf32_Word *hasharr = &map->l_gnu_chain_zero[last_bucket];

      /* The chain is terminated by an entry with LSBit set.  */
      while ((*hasharr & 1u) == 0)
	++hasharr;

      /* Size of dynamic symbol table is "index of last symbol" + 1.  */
      return hasharr - map->l_gnu_chain_zero + 1;
    }

  return 0;
}

/* Create the fastload position hash (if requested).  Given a link_map
   containing all required objects, iterate through all symbols. For each
   symbol which could match in do_lookup_x, insert the index of the
   providing object in main_map.  */

void
internal_function
_dl_fill_position_hash (struct link_map *main_map)
{
  const int num_objects = main_map->l_searchlist.r_nlist;
  const int debug_fastload = GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD;
  int k;

  /* If we have more than dl_position_hash_cutoff shared libraries,
     fill in the hash table of earliest known positions for symbols in
     main_map.  We'll use this in do_lookup_x().  */

  /* If cutoff or size is negative, fastload is disabled.  */
  if (GLRO(dl_position_hash_cutoff) < 0 || GLRO(dl_position_hash_bits) < 0)
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

  int *ndynsyms = NULL;

  if (GLRO(dl_position_hash_bits) == 0)
    {
      /* Table size has not been specified via LD_FASTLOAD_HASH_BITS.
         Compute appropriate size for <= 25% load factor.  */

      int total_symbols = 0;

      /* It is safe to call alloca() here, because we are executing in
	 the context of dl_main, i.e. we are on the main stack, and are
	 not yet using much of it.  Assuming sane upper limit of 10K direct
	 dependencies, we'll use at most 40K bytes here.  */
      ndynsyms = alloca (num_objects * sizeof (int));

      for (k = 0; k < num_objects; ++k)
        {
          const struct link_map *const map = main_map->l_searchlist.r_list[k];
	  const int nsyms = num_dynsyms (map);

	  ndynsyms[k] = nsyms;
	  total_symbols += nsyms;
	  if (__builtin_expect (debug_fastload, 0))
	    _dl_debug_printf ("fastload: %s: %u symbols\n", map->l_name, nsyms);
	}

      /* Ensure table size is >= 4 * total_symbols.  Generally this
         over-sizes the table by more than 4, since there usually are lots
         of duplicate symbols (from different DSOs) as well.  */

      GLRO(dl_position_hash_bits)
        = (8 * sizeof (total_symbols)) - __builtin_clz (total_symbols) + 2;

      if (__builtin_expect (debug_fastload, 0))
        _dl_debug_printf ("fastload: will use %u hash bits for %u total "
                          "symbols\n",
                          GLRO(dl_position_hash_bits), total_symbols);
    }

  /* If the requested or computed table size is too large, limit it.  */
  if (GLRO(dl_position_hash_bits) > DL_POSITION_HASH_BITS_MAX)
    {
      if (__builtin_expect (debug_fastload, 0))
        _dl_debug_printf ("fastload: requested table too large"
                          " (%u > max %u bits)\n",
                          GLRO(dl_position_hash_bits),
                          DL_POSITION_HASH_BITS_MAX);
      GLRO(dl_position_hash_bits) = DL_POSITION_HASH_BITS_MAX;
    }

  int table_size = 1 << GLRO(dl_position_hash_bits);
  GLRO(dl_position_hash_mask) = table_size - 1;
  if (__builtin_expect (debug_fastload, 0))
    _dl_debug_printf ("fastload: enabled with %u entry table\n", table_size);

  int max_pos = num_objects < DL_POSITION_MAX ? num_objects : DL_POSITION_MAX;
  GLRO(dl_position_hash_table) = allocate_position_hash_table (table_size,
							       max_pos);
  if (GLRO(dl_position_hash_table) == NULL)
    {
      if (__builtin_expect (debug_fastload, 0))
	_dl_debug_printf ("fastload: failed to allocate table\n");
      return;
    }

  /* There is no point in going beyond max_pos: the rest of the table
     has already been saturated with max_pos value.  */
  for (k = 0; k < max_pos; ++k)
    {
      const struct link_map *const map = main_map->l_searchlist.r_list[k];

      /* Reuse num_dynsyms() result we computed earlier if possible:
	 recomputing it may be somewhat expensive with DT_GNU_HASH.  */
      const int nsyms = ndynsyms ? ndynsyms[k] : num_dynsyms (map);

      fill_hash_table (map, k, nsyms);
    }
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
      const int skip_to = earliest_pos_from_hash_table (undef_name);

      if (skip_to < n)
	{
	  i = skip_to;
	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
	    _dl_debug_printf ("fastload %s skipping to %u\n",
			      undef_name, (unsigned int) i);
	}
      else
	{
	  /* Symbol was not found in any of the initial libraries,
	     and no new libraries have been added.  */

	  assert (skip_to == n);

	  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FASTLOAD, 0))
	    _dl_debug_printf ("fastload %s, %u >= %u (scope->r_nlist)\n",
			      undef_name, (unsigned int) skip_to,
			      (unsigned int) n);
	  return 0;
	}
    }

  do
    {
      /* These variables are used in the nested function.  */
      Elf_Symndx symidx;
      int num_versions = 0;
      const ElfW(Sym) *versioned_sym = NULL;

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
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_SYMBOLS, 0))
	_dl_debug_printf ("symbol=%s;  lookup in file=%s [%lu]\n",
			  undef_name, DSO_FILENAME (map->l_name),
			  map->l_ns);

      /* If the hash table is empty there is nothing to do here.  */
      if (map->l_nbuckets == 0)
	continue;

      /* The tables for this map.  */
      const ElfW(Sym) *symtab = (const void *) D_PTR (map, l_info[DT_SYMTAB]);
      const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);


      /* Nested routine to check whether the symbol matches.  */
      const ElfW(Sym) *
      __attribute_noinline__
      check_match (const ElfW(Sym) *sym)
      {
	unsigned int stt = ELFW(ST_TYPE) (sym->st_info);
	assert (ELF_RTYPE_CLASS_PLT == 1);
	if (__builtin_expect ((sym->st_value == 0 /* No value.  */
			       && stt != STT_TLS)
			      || (type_class & (sym->st_shndx == SHN_UNDEF)),
			      0))
	  return NULL;

	/* Ignore all but STT_NOTYPE, STT_OBJECT, STT_FUNC,
	   STT_COMMON, STT_TLS, and STT_GNU_IFUNC since these are no
	   code/data definitions.  */
#define ALLOWED_STT \
	((1 << STT_NOTYPE) | (1 << STT_OBJECT) | (1 << STT_FUNC) \
	 | (1 << STT_COMMON) | (1 << STT_TLS) | (1 << STT_GNU_IFUNC))
	if (__builtin_expect (((1 << stt) & ALLOWED_STT) == 0, 0))
	  return NULL;

	if (sym != ref && strcmp (strtab + sym->st_name, undef_name))
	  /* Not the symbol we are looking for.  */
	  return NULL;

	const ElfW(Half) *verstab = map->l_versyms;
	if (version != NULL)
	  {
	    if (__builtin_expect (verstab == NULL, 0))
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
			&& num_versions++ == 0)
		      /* No version so far.  */
		      versioned_sym = sym;

		    return NULL;
		  }
	      }
	  }

	/* There cannot be another entry for this symbol so stop here.  */
	return sym;
      }

      const ElfW(Sym) *sym;
      const ElfW(Addr) *bitmask = map->l_gnu_bitmask;
      if (__builtin_expect (bitmask != NULL, 1))
	{
	  ElfW(Addr) bitmask_word
	    = bitmask[(new_hash / __ELF_NATIVE_CLASS)
		      & map->l_gnu_bitmask_idxbits];

	  unsigned int hashbit1 = new_hash & (__ELF_NATIVE_CLASS - 1);
	  unsigned int hashbit2 = ((new_hash >> map->l_gnu_shift)
				   & (__ELF_NATIVE_CLASS - 1));

	  if (__builtin_expect ((bitmask_word >> hashbit1)
				& (bitmask_word >> hashbit2) & 1, 0))
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
			sym = check_match (&symtab[symidx]);
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
	      sym = check_match (&symtab[symidx]);
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
	  switch (__builtin_expect (ELFW(ST_BIND) (sym->st_info), STB_GLOBAL))
	    {
	    case STB_WEAK:
	      /* Weak definition.  Use this value if we don't find another.  */
	      if (__builtin_expect (GLRO(dl_dynamic_weak), 0))
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
	    success:
	      /* Global definition.  Just what we need.  */
	      result->s = sym;
	      result->m = (struct link_map *) map;
	      return 1;

	    case STB_GNU_UNIQUE:;
	      /* We have to determine whether we already found a
		 symbol with this name before.  If not then we have to
		 add it to the search table.  If we already found a
		 definition we have to use it.  */
	      void enter (struct unique_sym *table, size_t size,
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
			  return 1;
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
			  enter (newentries, newsize, entries[idx].hashval,
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
		  if (__builtin_expect (tab->size, 0))
		    {
		      assert (GLRO(dl_debug_mask) & DL_DEBUG_PRELINK);
		      __rtld_lock_unlock_recursive (tab->lock);
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
		enter (entries, size, new_hash, strtab + sym->st_name, ref,
		       undef_map);
	      else
		{
		  enter (entries, size, new_hash, strtab + sym->st_name, sym,
			 map);

		  if (map->l_type == lt_loaded)
		    /* Make sure we don't unload this object by
		       setting the appropriate flag.  */
		    ((struct link_map *) map)->l_flags_1 |= DF_1_NODELETE;
		}
	      ++tab->n_elements;

	      __rtld_lock_unlock_recursive (tab->lock);

	      goto success;

	    default:
	      /* Local symbols are ignored.  */
	      break;
	    }
	}

      /* If this current map is the one mentioned in the verneed entry
	 and we have not found a weak entry, it is a bug.  */
      if (symidx == STN_UNDEF && version != NULL && version->filename != NULL
	  && __builtin_expect (_dl_name_match_p (version->filename, map), 0))
	return -1;
    }
  while (++i < n);

  /* We have not found anything until now.  */
  return 0;
}


static uint_fast32_t
dl_new_hash (const char *s)
{
  uint_fast32_t h = 5381;
  for (unsigned char c = *s; c != '\0'; c = *++s)
    h = h * 33 + c;
  return h & 0xffffffff;
}


/* Add extra dependency on MAP to UNDEF_MAP.  */
static int
internal_function
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
  if (__builtin_expect (flags & DL_LOOKUP_GSCOPE_LOCK, 0))
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
      if (__builtin_expect (l_reldepsact >= undef_map->l_reldepsmax, 0))
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
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0))
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

  if (__builtin_expect (flags & DL_LOOKUP_GSCOPE_LOCK, 0))
    THREAD_GSCOPE_SET_FLAG ();

  return result;

 out_check:
  if (map->l_serial != serial)
    result = -1;
  goto out;
}

static void
internal_function
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
internal_function
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
  if (__builtin_expect (skip_map != NULL, 0))
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

      if (__builtin_expect (res, 0) < 0 && skip_map == NULL)
	{
	  /* Oh, oh.  The file named in the relocation entry does not
	     contain the needed symbol.  This code is never reached
	     for unversioned lookups.  */
	  assert (version != NULL);
	  const char *reference_name = undef_map ? undef_map->l_name : "";

	  /* XXX We cannot translate the message.  */
	  _dl_signal_cerror (0, DSO_FILENAME (reference_name),
			     N_("relocation error"),
			     make_string ("symbol ", undef_name, ", version ",
					  version->name,
					  " not defined in file ",
					  version->filename,
					  " with link time reference",
					  res == -2
					  ? " (no version symbols)" : ""));
	  *ref = NULL;
	  return 0;
	}
    }

  if (__builtin_expect (current_value.s == NULL, 0))
    {
      if ((*ref == NULL || ELFW(ST_BIND) ((*ref)->st_info) != STB_WEAK)
	  && skip_map == NULL
	  && !(GLRO(dl_debug_mask) & DL_DEBUG_UNUSED))
	{
	  /* We could find no value for a strong reference.  */
	  const char *reference_name = undef_map ? undef_map->l_name : "";
	  const char *versionstr = version ? ", version " : "";
	  const char *versionname = (version && version->name
				     ? version->name : "");

	  /* XXX We cannot translate the message.  */
	  _dl_signal_cerror (0, DSO_FILENAME (reference_name),
			     N_("symbol lookup error"),
			     make_string (undefined_msg, undef_name,
					  versionstr, versionname));
	}
      *ref = NULL;
      return 0;
    }

  int protected = (*ref
		   && ELFW(ST_VISIBILITY) ((*ref)->st_other) == STV_PROTECTED);
  if (__builtin_expect (protected != 0, 0))
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
			     skip_map, ELF_RTYPE_CLASS_PLT, NULL) != 0)
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
  if (__builtin_expect (current_value.m->l_type == lt_loaded, 0)
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
  if (__builtin_expect (current_value.m->l_used == 0, 0))
    current_value.m->l_used = 1;

  if (__builtin_expect (GLRO(dl_debug_mask)
			& (DL_DEBUG_BINDINGS|DL_DEBUG_PRELINK), 0))
    _dl_debug_bindings (undef_name, undef_map, ref,
			&current_value, version, type_class, protected);

  *ref = current_value.s;
  return LOOKUP_VALUE (current_value.m);
}


/* Cache the location of MAP's hash table.  */

void
internal_function
_dl_setup_hash (struct link_map *map)
{
  Elf_Symndx *hash;

  if (__builtin_expect (map->l_info[DT_ADDRTAGIDX (DT_GNU_HASH) + DT_NUM
				    + DT_THISPROCNUM + DT_VERSIONTAGNUM
				    + DT_EXTRANUM + DT_VALNUM] != NULL, 1))
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
internal_function
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
	  else if (__builtin_expect (undef_map->l_symbolic_in_local_scope, 0)
		   && val.s
		   && __builtin_expect (ELFW(ST_BIND) (val.s->st_info),
					STB_GLOBAL) == STB_GNU_UNIQUE)
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
	  if (__builtin_expect (ELFW(ST_TYPE) (value->s->st_info)
				== STT_TLS, 0))
	    type_class = 4;
	  else if (__builtin_expect (ELFW(ST_TYPE) (value->s->st_info)
				     == STT_GNU_IFUNC, 0))
	    type_class |= 8;
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
