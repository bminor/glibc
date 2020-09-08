/* futex helper functions for glibc-internal use.
   Copyright (C) 2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <time.h>
#include <futex-internal.h>
#include <kernel-features.h>

#ifndef __ASSUME_TIME64_SYSCALLS
static int
__futex_abstimed_wait_cancellable32 (unsigned int* futex_word,
                                     unsigned int expected, clockid_t clockid,
                                     const struct __timespec64* abstime,
                                     int private)
{
  if (! in_time_t_range (abstime->tv_sec))
    return -EOVERFLOW;

  unsigned int clockbit = (clockid == CLOCK_REALTIME)
	  ? FUTEX_CLOCK_REALTIME : 0;
  int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  struct timespec ts32 = valid_timespec64_to_timespec (*abstime);
  return INTERNAL_SYSCALL_CANCEL (futex, futex_word, op, expected,
                                  &ts32, NULL /* Unused.  */,
                                  FUTEX_BITSET_MATCH_ANY);
}
#endif

int
__futex_abstimed_wait_cancelable64 (unsigned int* futex_word,
                                    unsigned int expected, clockid_t clockid,
                                    const struct __timespec64* abstime,
                                    int private)
{
  unsigned int clockbit;
  int err;

  /* Work around the fact that the kernel rejects negative timeout values
     despite them being valid.  */
  if (__glibc_unlikely ((abstime != NULL) && (abstime->tv_sec < 0)))
    return ETIMEDOUT;

  if (! lll_futex_supported_clockid (clockid))
    return EINVAL;

  clockbit = (clockid == CLOCK_REALTIME) ? FUTEX_CLOCK_REALTIME : 0;
  int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  err = INTERNAL_SYSCALL_CANCEL (futex_time64, futex_word, op, expected,
                                 abstime, NULL /* Unused.  */,
                                 FUTEX_BITSET_MATCH_ANY);
#ifndef __ASSUME_TIME64_SYSCALLS
  if (err == -ENOSYS)
    err = __futex_abstimed_wait_cancellable32 (futex_word, expected,
                                               clockid, abstime, private);
#endif

  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
    case -EOVERFLOW:  /* Passed absolute timeout uses 64 bit time_t type, but
                         underlying kernel does not support 64 bit time_t futex
                         syscalls.  */
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment or due to the timeout not
		     being normalized.  Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}
