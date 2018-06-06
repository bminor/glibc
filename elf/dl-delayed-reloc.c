/* Delayed relocation processing.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#if HAVE_IFUNC

# include <assert.h>
# include <dl-delayed-reloc.h>
# include <errno.h>
# include <ldsodefs.h>
# include <sys/mman.h>
# include <unistd.h>

/* Machine-specific definitions.  */
# include <dl-delayed-reloc-machine.h>

/* This struct covers a whole page containing individual struct
   dl_delayed_reloc elements, which are allocated individually by
   allocate_reloc below.  */
struct dl_delayed_reloc_array
{
  struct dl_delayed_reloc_array *next;
  struct dl_delayed_reloc data[];
};

/* Pointer to global state.  We use this indirection so that we do not
   have to add the entire struct to the BSS segment.  */
static struct dl_delayed_reloc_global *global;

/* Allocate a new struct dl_delayed_reloc_array object.  Update global
   *accordingly.  */
static void
allocate_array (void)
{
  size_t page_size = GLRO(dl_pagesize);
  void *ptr = __mmap (NULL, page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED)
    _dl_signal_error (ENOMEM, NULL, NULL,
                      "cannot allocate IFUNC resolver information");
  struct dl_delayed_reloc_array *new_head = ptr;

  if (global->array_list_tail == NULL)
    {
      /* First allocation.  */
      global->array_list_head = new_head;
      global->array_list_tail = new_head;
      global->array_limit
        = (page_size - offsetof (struct dl_delayed_reloc_array, data))
            / sizeof (new_head->data[0]);
    }
  else
    {
      global->array_list_tail->next = new_head;
      global->array_list_tail = new_head;
      global->tail_array_count = 0;
    }
}

/* Allocate one struct dl_delayed_reloc element from the active
   allocation array.  */
static struct dl_delayed_reloc *
allocate_reloc (void)
{
  assert (global != NULL);

  /* Allocate a new array if none exists or the current array is
     full.  */
  if (global->tail_array_count == global->array_limit)
    allocate_array ();
  assert (global->tail_array_count < global->array_limit);
  return &global->array_list_tail->data[global->tail_array_count++];
}

/* Deallocate the list of array allocations starting at
   array_list.  */
static void
free_allocations (void)
{
  size_t page_size = GLRO(dl_pagesize);
  struct dl_delayed_reloc_array *p = global->array_list_head;
  while (p != NULL)
    {
      struct dl_delayed_reloc_array *next = p->next;
      __munmap (p, page_size);
      p = next;
    }
  /* The caller needs to call _dl_delayed_reloc_init again to start
     over.  */
  global = NULL;
}

/* Called in debugging mode to print details about a delayed
   relocation.  */
static void
report_delayed_relocation (struct link_map **current_map,
                           struct dl_delayed_reloc *dr)
{
  if (dr->map != *current_map)
    {
      *current_map = dr->map;

      /* l_name is NULL for the main executable.  */
      const char *map_name;
      if (dr->map->l_name != NULL && *dr->map->l_name != '\0')
        map_name = dr->map->l_name;
      else
        map_name = "<executable>";

      _dl_debug_printf ("applying delayed relocations for %s\n", map_name);
    }

  if (dr->sym != NULL)
    {
      const char *strtab
        = (const char *) D_PTR (dr->sym_map, l_info[DT_STRTAB]);
      if (dr->sym_map->l_name != NULL)
        _dl_debug_printf ("delayed relocation of symbol %s in %s\n",
                          strtab + dr->sym->st_name, dr->sym_map->l_name);
      else
        _dl_debug_printf ("delayed relocation of symbol %s\n",
                          strtab + dr->sym->st_name);
    }
  else
    {
      unsigned long int where = (uintptr_t) dr->reloc_addr;
      _dl_debug_printf ("delayed relative relocation at 0x%lx\n", where);
    }
}

/* Process all delayed IFUNC resolutions for IFUNC_MAP alone.  */
static void
apply_relocations (void)
{
  size_t array_limit = global->array_limit;
  if (array_limit == 0)
    /* No delayed relocations have been allocated, so there is nothing
       to do.  */
    return;

  /* Used for debugging output, to report switches from relocated
     object to another.  */
  struct link_map *current_map = NULL;
  unsigned long int count = 0;

  for (struct dl_delayed_reloc_array *list = global->array_list_head;
       list != NULL; list = list->next)
    {
      for (size_t index = 0; index < array_limit; ++index)
        {
          struct dl_delayed_reloc *dr = list->data + index;
          if (dr->reloc == NULL)
            /* An incompletely filled array marks the end of the
               list.  */
            goto out;

          if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS))
            report_delayed_relocation (&current_map, dr);
          _dl_delayed_reloc_machine (dr);

          /* Mark the object as fully relocated, for subsequent dlopen
             calls.  This will clear the flag even if there are still
             pending relocations to process, but we keep executing the
             loop, so this is not a problem.  */
          dr->map->l_delayed_relocations = false;

          ++count;
        }

    }

 out:
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_BINDINGS))
    _dl_debug_printf ("%lu delayed relocations performed\n", count);
}

void
_dl_delayed_reloc_init (struct dl_delayed_reloc_global *new_global)
{
  assert (global == NULL);
  global = new_global;
  *global = (struct dl_delayed_reloc_global) { };
}

void
_dl_delayed_reloc_record (struct link_map *map,
                          const ElfW(Sym) *refsym,
                          const ElfW(Rela) *reloc,
                          ElfW(Addr) *reloc_addr,
                          struct link_map *sym_map,
                          const ElfW(Sym) *sym)
{
  /* reloc == NULL is a marker to find the end of the allocations.  */
  assert (reloc != NULL);

  /* Add the delayed relocation to the global list.  */
  struct dl_delayed_reloc *dr = allocate_reloc ();
  *dr = (struct dl_delayed_reloc)
    {
      .map = map,
      .refsym = refsym,
      .reloc = reloc,
      .reloc_addr = reloc_addr,
      .sym = sym,
      .sym_map = sym_map,
    };

  /* The map containing the relocation will now need special
     processing for future copy and relative IFUNC relocations.  */
  map->l_delayed_relocations = true;
}

void
_dl_delayed_reloc_apply (void)
{
  assert (global != NULL);

  apply_relocations ();
  free_allocations ();
}

void
_dl_delayed_reloc_clear (void)
{
  /* This can be called from error handling, where the initialization
     may not yet have happened.  */
  if (global == NULL)
    return;

  free_allocations ();
}

#endif /* HAVE_IFUNC */
