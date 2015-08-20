/* Multiple versions of strncasecmp_l.
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
/* Redefine strncasecmp_l so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strncasecmp_l
# define strncasecmp_l __redirect_strncasecmp_l
# include <string.h>
# undef strncasecmp_l

# include <init-arch.h>

extern __typeof (__redirect_strncasecmp_l) __strncasecmp_l_i386 attribute_hidden;
extern __typeof (__redirect_strncasecmp_l) __strncasecmp_l_ssse3 attribute_hidden;
extern __typeof (__redirect_strncasecmp_l) __strncasecmp_l_sse4_2 attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_strncasecmp_l) __strncasecmp_l;
extern void *strncasecmp_l_ifunc (void) __asm__ ("__strncasecmp_l");

void *
strncasecmp_l_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE4_2))
    return __strncasecmp_l_sse4_2;
  else if (HAS_CPU_FEATURE (SSSE3))
    return __strncasecmp_l_ssse3;

  return __strncasecmp_l_i386;
}
__asm__ (".type __strncasecmp_l, %gnu_indirect_function");

weak_alias (__strncasecmp_l, strncasecmp_l)
#endif
