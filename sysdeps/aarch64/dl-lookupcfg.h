/* Configuration of lookup functions.
   Copyright (C) 2006-2022 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#define DL_UNMAP_IS_SPECIAL

#include_next <dl-lookupcfg.h>

struct link_map;

#ifdef __CHERI_PURE_CAPABILITY__
/* Symbol pointer with correct capability permission and bounds.  */
void *_dl_symbol_address (struct link_map *map, const ElfW(Sym) *ref);
rtld_hidden_proto (_dl_symbol_address)

# define DL_SYMBOL_ADDRESS(map, ref) _dl_symbol_address(map, ref)
#endif

extern void _dl_unmap (struct link_map *map);

#define DL_UNMAP(map) _dl_unmap (map)
