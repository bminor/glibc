/* Multiple versions of memcmp.
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

/* Define multiple versions only for the definition in lib.  */
#if IS_IN (libc)
/* Redefine memcmp so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef memcmp
# define memcmp __redirect_memcmp
# include <string.h>
# undef memcmp

# include <init-arch.h>

extern __typeof (__redirect_memcmp) __memcmp_i386 attribute_hidden;
extern __typeof (__redirect_memcmp) __memcmp_i686 attribute_hidden;
extern __typeof (__redirect_memcmp) __memcmp_ssse3 attribute_hidden;
extern __typeof (__redirect_memcmp) __memcmp_sse4_2 attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_memcmp) memcmp;
extern void *memcmp_ifunc (void) __asm__ ("memcmp");

void *
memcmp_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE4_2))
    return __memcmp_sse4_2;
  else if (HAS_CPU_FEATURE (SSSE3))
    return __memcmp_ssse3;

  if (USE_I686)
    return __memcmp_i686;
  else
    return __memcmp_i386;
}
__asm__ (".type memcmp, %gnu_indirect_function");

weak_alias (memcmp, bcmp)
#endif
