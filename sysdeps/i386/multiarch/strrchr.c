/* Multiple versions of strrchr
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
/* Redefine strrchr so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strrchr
# define strrchr __redirect_strrchr
# include <string.h>
# undef strrchr

# include <init-arch.h>

extern __typeof (__redirect_strrchr) __strrchr_i386 attribute_hidden;
extern __typeof (__redirect_strrchr) __strrchr_sse2 attribute_hidden;
extern __typeof (__redirect_strrchr) __strrchr_sse2_bsf attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_strrchr) strrchr;
extern void *strrchr_ifunc (void) __asm__ ("strrchr");

void *
strrchr_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Slow_BSF))
	return __strrchr_sse2;
      else
	return __strrchr_sse2_bsf;
    }

  return __strrchr_i386;
}
__asm__ (".type strrchr, %gnu_indirect_function");
weak_alias (strrchr, rindex)
#endif
