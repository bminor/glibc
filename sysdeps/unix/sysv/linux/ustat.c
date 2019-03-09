/* Get filesystem statistics (deprecated).  Linux version.
   Copyright (C) 1997-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
   <https://www.gnu.org/licenses/>.  */

#include <shlib-compat.h>

/* This deprecated syscall is no longer used (replaced with {f}statfs).  */
#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_28)

# include <sysdep.h>
# include <errno.h>

# ifndef DEV_TO_KDEV
#  define DEV_TO_KDEV(__dev)					\
  ({								\
    unsigned long long int k_dev;				\
    k_dev = dev & ((1ULL << 32) - 1);				\
    if (k_dev != dev)						\
     return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);		\
    (unsigned int) k_dev;					\
  })
# endif

/* INLINE_SYSCALL_CALL requires all pointer arguments to point to
   complete types, but we do not need to access any of the fields of
   this structure.  It was formerly 20, 24, or 32 bytes, depending on
   architecture and _FILE_OFFSET_BITS.  */
struct ustat
{
  char dummy[32];
};

int
__old_ustat (dev_t dev, struct ustat *ubuf)
{
# ifdef __NR_ustat
  return INLINE_SYSCALL_CALL (ustat, DEV_TO_KDEV (dev), ubuf);
# else
  return INLINE_SYSCALL_ERROR_RETURN_VALUE (ENOSYS);
# endif
}
compat_symbol (libc, __old_ustat, ustat, GLIBC_2_0);
#endif /* SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_28)  */
