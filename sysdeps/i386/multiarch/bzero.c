/* Multiple versions of bzero.
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
/* Redefine bzero so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef bzero
# define bzero __redirect_bzero
# include <string.h>
# undef bzero

# include <init-arch.h>

extern __typeof (__redirect_bzero) __bzero_i386 attribute_hidden;
extern __typeof (__redirect_bzero) __bzero_i586 attribute_hidden;
extern __typeof (__redirect_bzero) __bzero_i686 attribute_hidden;
extern __typeof (__redirect_bzero) __bzero_sse2 attribute_hidden;
extern __typeof (__redirect_bzero) __bzero_sse2_rep attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_bzero) __bzero;
extern void *bzero_ifunc (void) __asm__ ("__bzero");

void *
bzero_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Rep_String))
	return __bzero_sse2_rep;
      else
	return __bzero_sse2;
    }

  if (HAS_I686)
    return __bzero_i686;
  else if (HAS_I586)
    return __bzero_i586;
  else
    return __bzero_i386;
}
__asm__ (".type __bzero, %gnu_indirect_function");

weak_alias (__bzero, bzero)
#endif
