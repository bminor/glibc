/* Early initialization of libc.so.
   Copyright (C) 2020-2022 Free Software Foundation, Inc.
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

#ifndef _LIBC_EARLY_INIT_H
#define _LIBC_EARLY_INIT_H

#include <libc_early_init_name.h>

struct link_map;

/* In the shared case, this function is defined in libc.so and invoked
   from ld.so (or on the fist static dlopen) after complete relocation
   of a new loaded libc.so, but before user-defined ELF constructors
   run.  In the static case, this function is called directly from the
   startup code.  If INITIAL is true, the libc being initialized is
   the libc for the main program.  INITIAL is false for libcs loaded
   for audit modules, dlmopen, and static dlopen.  */
void __libc_early_init (_Bool initial)
#ifdef SHARED
/* Redirect to the actual implementation name.  */
  __asm__ (LIBC_EARLY_INIT_NAME_STRING)
#endif
  ;

/* Attempts to find the appropriately named __libc_early_init function
   in LIBC_MAP.  On lookup failure, an exception is signaled,
   indicating an ld.so/libc.so mismatch.  */
__typeof (__libc_early_init) *_dl_lookup_libc_early_init (struct link_map *
                                                          libc_map)
  attribute_hidden;

#endif /* _LIBC_EARLY_INIT_H */
