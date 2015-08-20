/* Multiple versions of stpncpy.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
# define _HAVE_STRING_ARCH_stpncpy
/* Redefine stpncpy so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef stpncpy
# define stpncpy __redirect_stpncpy
# include <string.h>
# undef stpncpy

# include <init-arch.h>

extern __typeof (__redirect_stpncpy) __stpncpy_i386 attribute_hidden;
extern __typeof (__redirect_stpncpy) __stpncpy_sse2 attribute_hidden;
extern __typeof (__redirect_stpncpy) __stpncpy_ssse3 attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_stpncpy) __stpncpy;
extern void *stpncpy_ifunc (void) __asm__ ("__stpncpy");

void *
stpncpy_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSSE3))
    return __stpncpy_ssse3;
  else if (HAS_CPU_FEATURE (SSE2))
    return __stpncpy_sse2;

  return __stpncpy_i386;
}
__asm__ (".type __stpncpy, %gnu_indirect_function");

weak_alias (__stpncpy, stpncpy)
#endif
