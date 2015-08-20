/* Multiple versions of __mempcpy_chk
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2015 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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
   DSO.  There are no multiarch mempcpy functions for static binaries.
 */
#if IS_IN (libc)
# ifdef SHARED
#  include <stddef.h>

extern void * __mempcpy_chk (void *, const void *, size_t, size_t);

extern __typeof (__mempcpy_chk) __mempcpy_chk_i386 attribute_hidden;
extern __typeof (__mempcpy_chk) __mempcpy_chk_i586 attribute_hidden;
extern __typeof (__mempcpy_chk) __mempcpy_chk_i686 attribute_hidden;
extern __typeof (__mempcpy_chk) __mempcpy_chk_sse2_unaligned attribute_hidden;
extern __typeof (__mempcpy_chk) __mempcpy_chk_ssse3 attribute_hidden;
extern __typeof (__mempcpy_chk) __mempcpy_chk_ssse3_rep attribute_hidden;

#  include <init-arch.h>

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern void *mempcpy_chk_ifunc (void) __asm__ ("__mempcpy_chk");

void *
mempcpy_chk_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Unaligned_Load))
	return __mempcpy_chk_sse2_unaligned;
      else if (HAS_CPU_FEATURE (SSSE3))
	{
	  if (HAS_ARCH_FEATURE (Fast_Rep_String))
	    return __mempcpy_chk_ssse3_rep;
	  else
	    return __mempcpy_chk_ssse3;
	}
    }

  if (USE_I686)
    return __mempcpy_chk_i686;
  else if (USE_I586)
    return __mempcpy_chk_i586;
  else
    return __mempcpy_chk_i386;
}
__asm__ (".type __mempcpy_chk, %gnu_indirect_function");
# else
#  include <debug/mempcpy_chk.c>
# endif
#endif
