/* Copyright (C) 2018-2020 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/* Get the current time of day and timezone information,
   putting it into *tv and *tz.  If tz is null, *tz is not filled.
   Returns 0 on success, -1 on errors.  */

#include <time.h>
#include <sysdep.h>

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

/* Used as a fallback in the ifunc resolver if VDSO is not available
   and for libc.so internal __gettimeofday calls.  */
static int
__gettimeofday_vsyscall (struct timeval *restrict tv, void *restrict tz)
{
  if (__glibc_unlikely (tz != 0))
    memset (tz, 0, sizeof *tz);

  return INLINE_VSYSCALL (gettimeofday, 2, tv, tz);
}

#ifdef SHARED
# include <dl-vdso.h>
# include <sysdep-vdso.h>

# define INIT_ARCH()
libc_ifunc (__gettimeofday,
	    (get_vdso_symbol (HAVE_GETTIMEOFDAY_VSYSCALL)
	    ?: __gettimeofday_vsyscall))

#else
int
__gettimeofday (struct timeval *restrict tv, void *restrict tz)
{
  return __gettimeofday_vsyscall (tv, tz);
}
#endif
weak_alias (__gettimeofday, gettimeofday)
