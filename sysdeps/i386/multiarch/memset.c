/* Multiple versions of memset.
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
/* Redefine memset so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef memset
# define memset __redirect_memset
# include <string.h>
# undef memset

# include <init-arch.h>

extern __typeof (__redirect_memset) __memset_i386 attribute_hidden;
extern __typeof (__redirect_memset) __memset_i586 attribute_hidden;
extern __typeof (__redirect_memset) __memset_i686 attribute_hidden;
extern __typeof (__redirect_memset) __memset_sse2 attribute_hidden;
extern __typeof (__redirect_memset) __memset_sse2_rep attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_memset) __memset;
extern void *memset_ifunc (void) __asm__ ("__memset");

void *
memset_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Rep_String))
	return __memset_sse2_rep;
      else
	return __memset_sse2;
    }

  if (USE_I686)
    return __memset_i686;
  else if (USE_I586)
    return __memset_i586;
  else
    return __memset_i386;
}
__asm__ (".type __memset, %gnu_indirect_function");

strong_alias (__memset, memset)
#endif
