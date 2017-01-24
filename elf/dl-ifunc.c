/* Delayed IFUNC processing.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#ifdef HAVE_IFUNC

# include <dl-ifunc.h>
# include <errno.h>
# include <ldsodefs.h>
# include <sys/mman.h>
# include <unistd.h>

/* Machine-specific definitions.  */
# include <dl-ifunc-reloc.h>

/* This struct covers a whole page containing individual struct
   dl_ifunc_relocation elements, which are allocated individually by
   allocate_relocation below.  */
struct dl_ifunc_relocation_array
{
  struct dl_ifunc_relocation_array *next;
  struct dl_ifunc_relocation data[];
};

/* Global list of allocated arrays of struct dl_ifunc_relocation
   elements.  */
static struct dl_ifunc_relocation_array *array_list;

/* Position of the next allocation in the current array used as the
   source of struct dl_ifunc_relocation allocations.  */
static struct dl_ifunc_relocation *next_alloc;
static unsigned int remaining_alloc;

/* Allocate a new struct dl_ifunc_relocation_array object.  Return the
   first data object therein.  Update array_list, next_alloc,
   remaining_alloc.  This function assumes taht remaining_alloc is
   zero. */
static struct dl_ifunc_relocation *
allocate_array (void)
{
  size_t page_size = GLRO(dl_pagesize);
  void *ptr = __mmap (NULL, page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED)
    _dl_signal_error (ENOMEM, NULL, NULL,
                      "cannot allocate IFUNC resolver information");
  struct dl_ifunc_relocation_array *new_head = ptr;
  new_head->next = array_list;
  array_list = new_head;
  next_alloc = &new_head->data[1];
  /* The function returns the first element of the new array.  */
  remaining_alloc
    = (page_size - offsetof (struct dl_ifunc_relocation_array, data))
    / sizeof (new_head->data[0]) - 1;
  return &new_head->data[0];
}

/* Allocate whone struct dl_ifunc_relocation element from the active
   array.  Updated next_alloc, remaining_alloc, and potentially
   array_list.  */
static struct dl_ifunc_relocation *
allocate_ifunc (void)
{
  if (remaining_alloc == 0)
    return allocate_array ();
  --remaining_alloc;
  return next_alloc++;
}

/* Deallocate the list of array allocations starting at
   array_list.  */
static void
free_allocations (void)
{
  size_t page_size = GLRO(dl_pagesize);
  struct dl_ifunc_relocation_array *p = array_list;
  while (p != NULL)
    {
      struct dl_ifunc_relocation_array *next = p->next;
      munmap (p, page_size);
      p = next;
    }
  /* Restart from scratch if _dl_ifunc_record_reloc is called
     again.  */
  array_list = NULL;
  next_alloc = NULL;
  remaining_alloc = 0;
}

/* Process all delayed IFUNC resolutions for IFUNC_MAP alone.  */
static void
apply_relocations (struct link_map *ifunc_map)
{
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS)
      && ifunc_map->l_name != NULL)
    _dl_debug_printf ("applying delayed IFUNC relocations against %s\n",
                      ifunc_map->l_name);

  struct dl_ifunc_relocation *p = ifunc_map->l_ifunc_relocations;
  unsigned int count = 0;
  while (p != NULL)
    {
      if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS)
          && ifunc_map->l_name != NULL)
        {
          if (p->ifunc_sym == NULL)
            _dl_debug_printf ("processing relative IFUNC relocation for %s\n",
                              ifunc_map->l_name);
          else
            {
              const char *strtab = (const char *) D_PTR (ifunc_map, l_info[DT_STRTAB]);
              _dl_debug_printf ("binding IFUNC symbol %s in %s against %s\n",
                                strtab + p->ifunc_sym->st_name,
                                p->reloc_map->l_name, ifunc_map->l_name);
            }
        }
      _dl_ifunc_process_relocation (p, ifunc_map);
      p = p->next;
      ++count;
    }

  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS)
      && ifunc_map->l_name != NULL)
    _dl_debug_printf ("%u delayed IFUNC relocations performed against %s\n",
                      count, ifunc_map->l_name);

  /* No deallocation takes place here.  All allocated objects are
     freed in bulk by a later call to free_allocations.  */
  ifunc_map->l_ifunc_relocations = NULL;
}

void internal_function
_dl_ifunc_record_reloc (struct link_map *reloc_map,
                        const ElfW(Rela) *reloc,
                        ElfW(Addr) *reloc_addr,
                        struct link_map *ifunc_map,
                        const ElfW(Sym) *ifunc_sym)
{
  /* Add the delayed IFUNC relocation to the list for ifunc_map.  */
  struct dl_ifunc_relocation *ifunc = allocate_ifunc ();
  *ifunc = (struct dl_ifunc_relocation)
    {
      .reloc_map = reloc_map,
      .reloc = reloc,
      .reloc_addr = reloc_addr,
      .ifunc_sym = ifunc_sym,
      .next = ifunc_map->l_ifunc_relocations,
    };
  ifunc_map->l_ifunc_relocations = ifunc;
}

void internal_function
_dl_ifunc_apply_relocations (struct link_map *new)
{
  /* Traverse the search list in reverse order, in case an IFUNC
     resolver calls into one its dependencies, and those dependency
     needs IFUNC resolution as well.  */
  struct link_map **r_list = new->l_searchlist.r_list;
  for (unsigned int i = new->l_searchlist.r_nlist; i-- > 0; )
    {
      struct link_map *l = r_list[i];
      if (l->l_ifunc_relocations != NULL)
        apply_relocations (l);
    }
  free_allocations ();
}

void internal_function
_dl_ifunc_clear_relocations (struct link_map *map)
{
  Lmid_t nsid = map->l_ns;
  struct link_namespaces *ns = &GL(dl_ns)[nsid];
  for (struct link_map *l = ns->_ns_loaded; l != NULL; l = l->l_next)
    l->l_ifunc_relocations = NULL;
  free_allocations ();
}

#endif  /* HAVE_IFUNC */
