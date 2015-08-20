/* Multiple versions of strncat.
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

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncat in static library since we
   need strncat before the initialization happened.  */
#if defined  SHARED && IS_IN (libc)
# define _HAVE_STRING_ARCH_strncat
/* Redefine strncat so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strncat
# define strncat __redirect_strncat
# include <string.h>
# undef strncat

# include <init-arch.h>

extern __typeof (__redirect_strncat) __strncat_i386 attribute_hidden;
extern __typeof (__redirect_strncat) __strncat_sse2 attribute_hidden;
extern __typeof (__redirect_strncat) __strncat_ssse3 attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_strncat) strncat;
extern void *strncat_ifunc (void) __asm__ ("strncat");

void *
strncat_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSSE3))
    return __strncat_ssse3;
  else if (HAS_CPU_FEATURE (SSE2))
    return __strncat_sse2;

  return __strncat_i386;
}
__asm__ (".type strncat, %gnu_indirect_function");
#endif
