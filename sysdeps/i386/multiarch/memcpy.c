/* Multiple versions of memcpy.
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

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need memcpy before the initialization
   happened.  */
#if defined SHARED && IS_IN (libc)
/* Redefine memcpy so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef memcpy
# define memcpy __redirect_memcpy
# include <string.h>
# undef memcpy

# include <init-arch.h>

extern __typeof (__redirect_memcpy) __memcpy_i386 attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_i586 attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_i686 attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_sse2_unaligned attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_ssse3 attribute_hidden;
extern __typeof (__redirect_memcpy) __memcpy_ssse3_rep attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_memcpy) memcpy;
extern void *memcpy_ifunc (void) __asm__ ("memcpy");

void *
memcpy_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Unaligned_Load))
	return __memcpy_sse2_unaligned;
      else if (HAS_CPU_FEATURE (SSSE3))
	{
	  if (HAS_ARCH_FEATURE (Fast_Rep_String))
	    return __memcpy_ssse3_rep;
	  else
	    return __memcpy_ssse3;
	}
    }

  if (USE_I686)
    return __memcpy_i686;
  else if (USE_I586)
    return __memcpy_i586;
  else
    return __memcpy_i386;
}
__asm__ (".type memcpy, %gnu_indirect_function");
#endif
