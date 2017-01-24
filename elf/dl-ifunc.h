/* Private declarations for delayed IFUNC processing.
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

#ifndef DL_IFUNC_H
#define DL_IFUNC_H

#include <link.h>

/* All functions declared in this file must be called while the global
   rtld lock is acquired.  */

/* Apply all pending IFUNC relocations in the search scope of the new
   link map.  Deallocate all auxiliary allocations.  */
void _dl_ifunc_apply_relocations (struct link_map *new)
  attribute_hidden internal_function;

/* Clear all allocated delayed IFUNC relocations in the namespace of
   the link map.  Deallocate all auxiliary allocations.  This function
   is intended for clearing up after a dlopen failure.  */
void _dl_ifunc_clear_relocations (struct link_map *map)
  attribute_hidden internal_function;

/* The no-op implementatoin is only available if IFUNCs are
   supported.  */
#ifdef HAVE_IFUNC

/* Delayed IFUNC relocation.  For each link map with a referenced
   IFUNC symbol, a separate single-linked list of delayed IFUNC
   relocations exists while the dynamic linker is running.  */
struct dl_ifunc_relocation
{
  /* Information about the relocation.  */
  struct link_map *reloc_map;
  const ElfW(Rela) *reloc;
  ElfW(Addr) *reloc_addr;

  /* Information about the IFUNC resolver.  The corresponding symbol
     map is implied.  */
  const ElfW(Sym) *ifunc_sym;

  /* Pointer to the next element in the list of IFUNC relocations for
     the symbol map.  */
  struct dl_ifunc_relocation *next;
};

/* Record a delayed IFUNC relocation for IFUNC_SYM at *RELOC_ADDR.
   The relocation is associated with IFUNC_MAP.  Can raise an error
   using _dl_signal_error.  IFUNC_SYM can be NULL if the relocation is
   a relative IFUNC relocation.  */
void _dl_ifunc_record_reloc (struct link_map *reloc_map,
                             const ElfW(Rela) *reloc,
                             ElfW(Addr) *reloc_addr,
                             struct link_map *ifunc_map,
                             const ElfW(Sym) *ifunc_sym)
  attribute_hidden internal_function;

#else /* !HAVE_IFUNC  */

/* Dummy implementations for targets without IFUNC support.  */

static inline void
_dl_ifunc_apply_relocations (struct link_map *new)
{
}

static inline void
_dl_ifunc_clear_relocations (struct link_map *map)
{
}

#endif /* !HAVE_IFUNC */

#endif /* DL_IFUNC_H */
