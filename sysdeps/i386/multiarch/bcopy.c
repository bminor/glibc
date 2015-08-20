/* Multiple versions of bcopy.
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
/* Redefine bcopy so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef bcopy
# define bcopy __redirect_bcopy
# include <string.h>
# undef bcopy

# include <init-arch.h>

extern __typeof (__redirect_bcopy) __bcopy_i386 attribute_hidden;
extern __typeof (__redirect_bcopy) __bcopy_i686 attribute_hidden;
extern __typeof (__redirect_bcopy) __bcopy_sse2_unaligned attribute_hidden;
extern __typeof (__redirect_bcopy) __bcopy_ssse3 attribute_hidden;
extern __typeof (__redirect_bcopy) __bcopy_ssse3_rep attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_bcopy) bcopy;
extern void *bcopy_ifunc (void) __asm__ ("bcopy");

void *
bcopy_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Unaligned_Load))
	return __bcopy_sse2_unaligned;
      else if (HAS_CPU_FEATURE (SSSE3))
	{
	  if (HAS_ARCH_FEATURE (Fast_Rep_String))
	    return __bcopy_ssse3_rep;
	  else
	    return __bcopy_ssse3;
	}
    }

  if (USE_I686)
    return __bcopy_i686;
  else
    return __bcopy_i386;
}
__asm__ (".type bcopy, %gnu_indirect_function");
#endif
