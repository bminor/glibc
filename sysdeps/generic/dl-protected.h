/* Support for STV_PROTECTED visibility.  Generic version.
   Copyright (C) 2021-2022 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef _DL_PROTECTED_H
#define _DL_PROTECTED_H

static inline void __attribute__ ((always_inline))
_dl_check_protected_symbol (const char *undef_name,
			    const struct link_map *undef_map,
			    const ElfW(Sym) *ref,
			    const struct link_map *map,
			    int type_class)
{
  if (undef_map != NULL
      && undef_map->l_type == lt_executable
      && !(undef_map->l_1_needed
	   & GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS)
      && (map->l_1_needed
	  & GNU_PROPERTY_1_NEEDED_INDIRECT_EXTERN_ACCESS))
    {
      if ((type_class & ELF_RTYPE_CLASS_COPY))
	/* Disallow copy relocations in executable against protected
	   data symbols in a shared object which needs indirect external
	   access.  */
	_dl_signal_error (0, map->l_name, undef_name,
			  N_("copy relocation against non-copyable protected symbol"));
      else if (ref->st_value != 0
	       && ref->st_shndx == SHN_UNDEF
	       && (type_class & ELF_RTYPE_CLASS_PLT))
	/* Disallow non-zero symbol values of undefined symbols in
	   executable, which are used as the function pointer, against
	   protected function symbols in a shared object with indirect
	   external access.  */
	_dl_signal_error (0, map->l_name, undef_name,
			  N_("non-canonical reference to canonical protected function"));
    }
}

#endif /* _DL_PROTECTED_H */
