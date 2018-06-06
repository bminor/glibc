/* Private declarations for delayed relocation processing.
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

/* Delayed relocation processing attempts to address relocation
   dependencies which are not reflected in DT_NEEDED dependencies
   because objects are incompletely linked, or the relocation order
   derived from the dependencies causes objects to refer to objects
   relocated later due to symbol interposition.  See bugs 20019,
   21041, 23240.

   Architecture-specific changes are required to implement delayed
   relocation processing, typically in the implementation of
   elf_machine_rel or elf_machine_rela.  There are three reasons why a
   relocation needs to be delayed:

     * The relocation is bound with the help of an IFUNC resolver, and
       the IFUNC resolver resides in an object which has not been
       fully relocated yet (i.e., l_relocated is false for its link
       map, or l_delayed_relocations is true).  The reason for the
       delay is that the IFUNC resolver may depend on the
       yet-unprocessed relocations.

     * The relocation is a copy relocation and the source symbol
       resides in an object which has not been fully relocated yet
       (l_delayed_relocations is true).  The reason for the delay is
       that the copy relocation could otherwise copy data which has
       not been fully initialized yet because some of the delayed
       relocations affect it.

     * The is a relative IFUNC relocation, and the link map for the
       object containing the relocation has other delayed relocations
       (l_delayed_relocations is true).  The reason for the delay is
       that the resolver for the relocation depends on other delayed
       relocations for this object which have not yet been performed.

   All delayed relocations are processed in the order they were
   recorded, which reflects the initial relocation processing order
   between objects, and the relocation order as determined by the
   static link editor within objects.

   This does not process correctly all implicit dependencies between
   IFUNC resolvers (e.g., two IFUNC resolvers calling each other will
   still not work), but it addresses common cases of implicit
   dependencies, for example as the result of symbol interposition or
   due to missing DT_NEEDED entries in objects.  In particular, data
   relocations used by the IFUNC resolvers in glibc itself will be
   resolved before these IFUNC resolvers run, so it is safe to call
   glibc string functions from other (non-glibc) IFUNC resolvers, or
   use pointers to glibc functions (with IFUNC resolvers) in global
   data initializers (inside or outside of glibc).

   Note that delayed relocation processing does not address the
   phenomenon that IFUNC resolvers run before ELF constructors and C++
   constructors.  This means that IFUNC resolvers cannot assume that
   glibc itself has been initialized, or that global data structures
   in other objects have the expected values, particularly if their
   initializers are not constants.  */

#ifndef _DL_DELAYED_RELOC_H
#define _DL_DELAYED_RELOC_H

#include <link.h>

/* All functions declared in this file must be called while the global
   rtld lock is acquired.  */

/* Internal storage for delayed relocations.  */
struct dl_delayed_reloc_array;

/* The full implementation is only available if IFUNCs are
   supported.  */
#if HAVE_IFUNC

struct dl_delayed_reloc_global
{
  /* List of allocated arrays of struct dl_delayed_reloc elements.  */
  struct dl_delayed_reloc_array *array_list_head;

  /* Last element of the list.  Used for allocations.  */
  struct dl_delayed_reloc_array *array_list_tail;

  /* Number of entries in the current allocation array.  */
  size_t tail_array_count;

  /* Maximum number of entries in each array.  */
  size_t array_limit;
};

/* Prepare for relocation processing.  *PGLOBAL must remain in scope
   until _dl_delayed_reloc_apply or _dl_delayed_reloc_clear is
   called.  */
void _dl_delayed_reloc_init (struct dl_delayed_reloc_global *pglobal)
  attribute_hidden;

/* Apply all pending delayed relocations.  Deallocate all auxiliary
   allocations.  */
void _dl_delayed_reloc_apply (void) attribute_hidden;

/* Clear all allocated delayed relocations.  Deallocate all auxiliary
   allocations.  This function is intended for clearing up after a
   dlopen failure.  */
void _dl_delayed_reloc_clear (void) attribute_hidden;

/* Delayed relocation.  These are stored in arrays inside struct
   dl_delayed_reloc_array.

   In case a copy or relative IFUNC relocation is encountered while
   the link map has delayed (IFUNC) relocations, these relocations are
   added to the list as well because the result of these relocations
   could affect their results.  */
struct dl_delayed_reloc
{
  /* Information about the relocation.  */
  const ElfW(Sym) *refsym;
  struct link_map *map;
  const ElfW(Rela) *reloc;
  ElfW(Addr) *reloc_addr;

  /* Information about the target symbol (either an IFUNC resolver, or
     the source of a copy relocation).  The corresponding symbol map
     is implied.  NULL for relative IFUNC relocations.  */
  const ElfW(Sym) *sym;

  /* The link map corresponding to sym.  */
  struct link_map *sym_map;
};

/* Record a delayed relocation for SYM at *RELOC_ADDR.  The relocation
   is associated with MAP.  Can raise an error using _dl_signal_error.
   SYM can be NULL if the relocation is a relative IFUNC
   relocation.  */
void _dl_delayed_reloc_record (struct link_map *map,
                               const ElfW(Sym) *refsym,
                               const ElfW(Rela) *reloc,
                               ElfW(Addr) *reloc_addr,
                               struct link_map *sym_map,
                               const ElfW(Sym) *sym) attribute_hidden;

#else /* !HAVE_IFUNC  */

/* Dummy implementations for targets without IFUNC support.  */

struct dl_delayed_reloc_global
{
};

static inline void
_dl_delayed_reloc_init (struct dl_delayed_reloc_global *global)
{
}

static inline void
_dl_delayed_reloc_apply (void)
{
}

static inline void
_dl_delayed_reloc_clear (void)
{
}

#endif /* !HAVE_IFUNC */

#endif /* _DL_DELAYED_RELOC_H */
