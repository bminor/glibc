/* Support for GNU properties for static builds.  Generic version.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#ifndef _LIBC_PROP_H
#define _LIBC_PROP_H

#include <dl-prop.h>

/* Called at the start of program execution to handle GNU attribute from
   PT_NOTE / PT_GNU_PROPERTY.  Must be on a top-level stack frame that does
   not return.  */
static __always_inline void
__libc_process_gnu_attributes (void)
{
# ifndef SHARED
  struct link_map *main_map = _dl_get_dl_main_map ();
  const ElfW(Phdr) *phdr = GL(dl_phdr);
  const ElfW(Phdr) *ph;
  for (ph = phdr; ph < phdr + GL(dl_phnum); ph++)
    if (ph->p_type == PT_GNU_PROPERTY)
      {
	_dl_process_pt_gnu_property (main_map, -1, ph);
	_rtld_main_check (main_map, _dl_argv[0]);
	break;
      }
# endif
}

#endif
