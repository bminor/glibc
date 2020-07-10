/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <kernel-features.h>
#include <errno.h>
#include <time.h>
#include "kernel-posix-cpu-timers.h"
#include <sysdep-vdso.h>
#include <time64-support.h>
#include <shlib-compat.h>

/* Get current value of CLOCK and store it in TP.  */
int
__clock_gettime64 (clockid_t clock_id, struct __timespec64 *tp)
{
  int r;

#ifndef __NR_clock_gettime64
# define __NR_clock_gettime64 __NR_clock_gettime
#endif

  if (supports_time64 ())
    {
#ifdef HAVE_CLOCK_GETTIME64_VSYSCALL
      r = INLINE_VSYSCALL (clock_gettime64, 2, clock_id, tp);
#else
      r = INLINE_SYSCALL_CALL (clock_gettime64, clock_id, tp);
#endif

      if (r == 0 || errno != ENOSYS)
	return r;

      mark_time64_unsupported ();
   }

#ifndef __ASSUME_TIME64_SYSCALLS
  /* Fallback code that uses 32-bit support.  */
  struct timespec tp32;
# ifdef HAVE_CLOCK_GETTIME_VSYSCALL
  r = INLINE_VSYSCALL (clock_gettime, 2, clock_id, &tp32);
# else
  r = INLINE_SYSCALL_CALL (clock_gettime, clock_id, &tp32);
# endif
  if (r == 0)
    *tp = valid_timespec_to_timespec64 (tp32);
#endif

  return r;
}

#if __TIMESIZE != 64
libc_hidden_def (__clock_gettime64)

int
__clock_gettime (clockid_t clock_id, struct timespec *tp)
{
  int ret;
  struct __timespec64 tp64;

  ret = __clock_gettime64 (clock_id, &tp64);

  if (ret == 0)
    {
      if (! in_time_t_range (tp64.tv_sec))
        {
          __set_errno (EOVERFLOW);
          return -1;
        }

      *tp = valid_timespec64_to_timespec (tp64);
    }

  return ret;
}
#endif
libc_hidden_def (__clock_gettime)

versioned_symbol (libc, __clock_gettime, clock_gettime, GLIBC_2_17);
/* clock_gettime moved to libc in version 2.17;
   old binaries may expect the symbol version it had in librt.  */
#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_17)
strong_alias (__clock_gettime, __clock_gettime_2);
compat_symbol (libc, __clock_gettime_2, clock_gettime, GLIBC_2_2);
#endif
