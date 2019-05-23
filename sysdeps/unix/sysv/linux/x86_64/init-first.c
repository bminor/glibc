/* Initialization code run first thing by the ELF startup code.  Linux/x86-64.
   Copyright (C) 2007-2019 Free Software Foundation, Inc.
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

#include <time.h>
#include <sysdep.h>
#include <dl-vdso.h>
#include <libc-vdso.h>

long int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *)
  attribute_hidden;
long int (*VDSO_SYMBOL(getcpu)) (unsigned *, unsigned *, void *)
  attribute_hidden;

extern __typeof (clock_gettime) __syscall_clock_gettime attribute_hidden;


static inline void
__vdso_platform_setup (void)
{
  void *p = get_vdso_symbol ("__vdso_clock_gettime");
  if (p == NULL)
    p = __syscall_clock_gettime;
  PTR_MANGLE (p);
  VDSO_SYMBOL(clock_gettime) = p;

  p = get_vdso_symbol ("__vdso_getcpu");
  PTR_MANGLE (p);
  VDSO_SYMBOL(getcpu) = p;
}

#define VDSO_SETUP __vdso_platform_setup

#include <csu/init-first.c>
