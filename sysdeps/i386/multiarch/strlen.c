/* Multiple versions of strlen.
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
#if defined SHARED && IS_IN (libc)
/* Redefine strlen so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strlen
# define strlen __redirect_strlen
# include <string.h>
# undef strlen

# include <init-arch.h>

extern __typeof (__redirect_strlen) __strlen_i386 attribute_hidden;
extern __typeof (__redirect_strlen) __strlen_i586 attribute_hidden;
extern __typeof (__redirect_strlen) __strlen_sse2 attribute_hidden;
extern __typeof (__redirect_strlen) __strlen_sse2_bsf attribute_hidden;

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_strlen) strlen;
extern void *strlen_ifunc (void) __asm__ ("strlen");

void *
strlen_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Slow_BSF))
	return __strlen_sse2;
      else
	return __strlen_sse2_bsf;
    }

  if (USE_I586)
    return __strlen_i586;
  else
    return __strlen_i386;
}
__asm__ (".type strlen, %gnu_indirect_function");
#endif
