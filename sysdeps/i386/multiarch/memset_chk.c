/* Multiple versions of __memset_chk
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
# include <stddef.h>
extern void * __memset_chk (void *, int, size_t, size_t);

# ifdef SHARED
extern __typeof (__memset_chk) __memset_chk_i386 attribute_hidden;
extern __typeof (__memset_chk) __memset_chk_i586 attribute_hidden;
extern __typeof (__memset_chk) __memset_chk_i686 attribute_hidden;
extern __typeof (__memset_chk) __memset_chk_sse2 attribute_hidden;
extern __typeof (__memset_chk) __memset_chk_sse2_rep attribute_hidden;

#  include <init-arch.h>
# else
/* Redefine memset so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
#  undef memset
#  define memset __redirect_memset
#  include <string.h>
#  undef memset

extern __typeof (__redirect_memset) __memset_i386 attribute_hidden;
extern __typeof (__redirect_memset) __memset_i586 attribute_hidden;
extern __typeof (__redirect_memset) __memset_i686 attribute_hidden;
extern __typeof (__redirect_memset) __memset_sse2 attribute_hidden;
extern __typeof (__redirect_memset) __memset_sse2_rep attribute_hidden;

#  include <init-arch.h>

/* Due to
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=10837
   noreturn attribute disable tail call optimization.  Removes noreturn
   attribute to enable tail call optimization.  */
extern void *chk_fail (void) __asm__ ("__chk_fail") attribute_hidden;

#define ifunc_chk(func, arch) \
static void *							\
func##_chk_##arch (void *dstpp, int c, size_t len,		\
		   size_t dstlen)				\
{								\
  if (__glibc_unlikely (dstlen < len))				\
    return chk_fail ();						\
  return func##_##arch (dstpp, c, len);				\
}

#  if MINIMUM_ISA < 686
ifunc_chk (__memset, i386)
ifunc_chk (__memset, i586)
#  else
extern __typeof (__redirect_memset) __memset_chk_i386 attribute_hidden;
extern __typeof (__redirect_memset) __memset_chk_i586 attribute_hidden;
#  endif
ifunc_chk (__memset, i686)
ifunc_chk (__memset, sse2)
ifunc_chk (__memset, sse2_rep)
# endif

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern void *memset_chk_ifunc (void) __asm__ ("__memset_chk");

void *
memset_chk_ifunc (void)
{
  if (HAS_CPU_FEATURE (SSE2))
    {
      if (HAS_ARCH_FEATURE (Fast_Unaligned_Load))
	return __memset_chk_sse2_rep;
      else
	return __memset_chk_sse2;
    }

  if (USE_I686)
    return __memset_chk_i686;
  else if (USE_I586)
    return __memset_chk_i586;
  else
    return __memset_chk_i386;
}
__asm__ (".type __memset_chk, %gnu_indirect_function");
#endif
