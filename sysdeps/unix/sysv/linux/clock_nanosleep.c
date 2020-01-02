/* Copyright (C) 2003-2020 Free Software Foundation, Inc.
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
#include <kernel-features.h>
#include <errno.h>

#include <sysdep-cancel.h>
#include "kernel-posix-cpu-timers.h"

#include <shlib-compat.h>

/* We can simply use the syscall.  The CPU clocks are not supported
   with this function.  */
int
__clock_nanosleep_time64 (clockid_t clock_id, int flags, const struct __timespec64 *req,
                          struct __timespec64 *rem)
{
  int r;

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID)
    clock_id = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);

  /* If the call is interrupted by a signal handler or encounters an error,
     it returns a positive value similar to errno.  */
  INTERNAL_SYSCALL_DECL (err);

#ifdef __ASSUME_TIME64_SYSCALLS
# ifndef __NR_clock_nanosleep_time64
#  define __NR_clock_nanosleep_time64 __NR_clock_nanosleep
# endif
  r = INTERNAL_SYSCALL_CANCEL (clock_nanosleep_time64, err, clock_id,
                               flags, req, rem);
#else
# ifdef __NR_clock_nanosleep_time64
  r = INTERNAL_SYSCALL_CANCEL (clock_nanosleep_time64, err, clock_id,
                               flags, req, rem);

  if (! INTERNAL_SYSCALL_ERROR_P (r, err))
    return 0;
  if (INTERNAL_SYSCALL_ERRNO (r, err) != ENOSYS)
    return INTERNAL_SYSCALL_ERRNO (r, err);
# endif /* __NR_clock_nanosleep_time64 */

  if (! in_time_t_range (req->tv_sec))
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  struct timespec tr32;
  struct timespec ts32 = valid_timespec64_to_timespec (*req);
  r = INTERNAL_SYSCALL_CANCEL (clock_nanosleep, err, clock_id, flags,
                               &ts32, &tr32);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    {
      if (INTERNAL_SYSCALL_ERRNO (r, err) == EINTR && rem != NULL
	  && (flags & TIMER_ABSTIME) == 0)
	*rem = valid_timespec_to_timespec64 (tr32);
    }
#endif /* __ASSUME_TIME64_SYSCALLS */

  return (INTERNAL_SYSCALL_ERROR_P (r, err)
	  ? INTERNAL_SYSCALL_ERRNO (r, err) : 0);
}

#if __TIMESIZE != 64
int
__clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
                   struct timespec *rem)
{
  int r;
  struct __timespec64 treq64, trem64;

  treq64 = valid_timespec_to_timespec64 (*req);
  r = __clock_nanosleep_time64 (clock_id, flags, &treq64, &trem64);

  if (r == EINTR && rem != NULL && (flags & TIMER_ABSTIME) == 0)
    *rem = valid_timespec64_to_timespec (trem64);

  return r;
}
#endif
libc_hidden_def (__clock_nanosleep)
versioned_symbol (libc, __clock_nanosleep, clock_nanosleep, GLIBC_2_17);
/* clock_nanosleep moved to libc in version 2.17;
   old binaries may expect the symbol version it had in librt.  */
#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_17)
strong_alias (__clock_nanosleep, __clock_nanosleep_2);
compat_symbol (libc, __clock_nanosleep_2, clock_nanosleep, GLIBC_2_2);
#endif
