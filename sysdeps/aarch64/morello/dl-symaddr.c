/* Get the symbol address.  Morello version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <ldsodefs.h>
#include <dl-machine.h>

void *
_dl_symbol_address (struct link_map *map, const ElfW(Sym) *ref)
{
  elfptr_t value = SYMBOL_ADDRESS (map, ref, false);
  if (map == NULL)
    return (void *) value;
  if (ELFW(ST_TYPE) (ref->st_info) == STT_OBJECT)
    {
      unsigned long perm_mask = CAP_PERM_MASK_R;
      for (int i = 0; i < map->l_rw_count; i++)
	if (map->l_rw_range[i].start <= value
	    && map->l_rw_range[i].end > value)
	  {
	    value = dl_rw_ptr (map, value - map->l_addr);
	    perm_mask = CAP_PERM_MASK_RW;
	    break;
	  }
      value = __builtin_cheri_bounds_set_exact (value, ref->st_size);
      value = __builtin_cheri_perms_and (value, perm_mask);
    }
  else if (ELFW(ST_TYPE) (ref->st_info) == STT_FUNC)
    {
      /* Seal function pointers.  Note: ifunc is handled by the caller.  */
      value = __builtin_cheri_seal_entry (value);
    }
  return (void *) value;
}
rtld_hidden_def (_dl_symbol_address)
