/* Copyright (C) 2005-2019 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <time.h>
#include <sysdep.h>

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

static int
__gettimeofday_syscall (struct timeval *restrict tv, void *restrict tz)
{
  if (__glibc_unlikely (tz != 0))
    memset (tz, 0, sizeof *tz);

  return INLINE_VSYSCALL (gettimeofday, 2, tv, tz);
}

#ifdef SHARED
# include <dl-vdso.h>
# include <libc-vdso.h>

# define INIT_ARCH() \
  void *vdso_gettimeofday = get_vdso_symbol (HAVE_GETTIMEOFDAY_VSYSCALL)

/* If the vDSO is not available we fall back syscall.  */
libc_ifunc (__gettimeofday,
	    vdso_gettimeofday
	    ? VDSO_IFUNC_RET (vdso_gettimeofday)
	    : (void *) __gettimeofday_syscall);
#else
int
__gettimeofday (struct timeval *restrict tv, void *restrict tz)
{
  return __gettimeofday_syscall (tv, tz);
}
#endif
weak_alias (__gettimeofday, gettimeofday)
