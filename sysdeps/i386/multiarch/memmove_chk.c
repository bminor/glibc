/* Multiple versions of __memmove_chk
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

/* Define multiple versions only for the definition in lib.  */
#if IS_IN (libc)
#  include <stddef.h>

extern void * __memmove_chk (void *, const void *, size_t, size_t);

# ifdef SHARED
extern __typeof (__memmove_chk) __memmove_chk_i386 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_i686 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_sse2_unaligned attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3_rep attribute_hidden;

#  include <init-arch.h>
# else
/* Redefine memmove so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
#  undef memmove
#  define memmove __redirect_memmove
#  include <string.h>
#  undef memmove

extern __typeof (__redirect_memmove) __memmove_i386 attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_i686 attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_sse2_unaligned attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_ssse3 attribute_hidden;
extern __typeof (__redirect_memmove) __memmove_ssse3_rep attribute_hidden;

/* Due to
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=10837
   noreturn attribute disable tail call optimization.  Removes noreturn
   attribute to enable tail call optimization.  */
extern void *chk_fail (void) __asm__ ("__chk_fail") attribute_hidden;

#  include <init-arch.h>

#define ifunc_chk(func, arch) \
static void *							\
func##_chk_##arch (void *dstpp, const void *srcpp, size_t len,	\
		   size_t dstlen)				\
{								\
  if (__glibc_unlikely (dstlen < len))				\
    return chk_fail ();						\
  return func##_##arch (dstpp, srcpp, len);			\
}

#  if MINIMUM_ISA < 686
ifunc_chk (__memmove, i386)
#  else
extern __typeof (__memmove_chk) __memmove_chk_i386 attribute_hidden;
#  endif

ifunc_chk (__memmove, i686)
ifunc_chk (__memmove, sse2_unaligned)
ifunc_chk (__memmove, ssse3)
ifunc_chk (__memmove, ssse3_rep)
# endif

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern void *memmove_chk_ifunc (void) __asm__ ("__memmove_chk");

void *
memmove_chk_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Unaligned_Load))
	return __memmove_chk_sse2_unaligned;
      else if (HAS_CPU_FEATURE (SSSE3))
	{
	  if (HAS_ARCH_FEATURE (Fast_Rep_String))
	    return __memmove_chk_ssse3_rep;
	  else
	    return __memmove_chk_ssse3;
	}
    }

  if (USE_I686)
    return __memmove_chk_i686;
  else
    return __memmove_chk_i386;
}
__asm__ (".type __memmove_chk, %gnu_indirect_function");
#endif
