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
__futex_abstimed_wait_cancelable32 (unsigned int* futex_word,
                                    unsigned int expected, clockid_t clockid,
                                    const struct __timespec64* abstime,
                                    int private)
{
  struct timespec ts32, *pts32 = NULL;
  if (abstime != NULL)
    {
      if (! in_time_t_range (abstime->tv_sec))
	return -EOVERFLOW;

      ts32 = valid_timespec64_to_timespec (*abstime);
      pts32 = &ts32;
    }

  unsigned int clockbit = (clockid == CLOCK_REALTIME)
	  ? FUTEX_CLOCK_REALTIME : 0;
  int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  return INTERNAL_SYSCALL_CANCEL (futex, futex_word, op, expected,
                                  pts32, NULL /* Unused.  */,
                                  FUTEX_BITSET_MATCH_ANY);
}

static int
__futex_abstimed_wait32 (unsigned int* futex_word,
                         unsigned int expected, clockid_t clockid,
                         const struct __timespec64* abstime,
                         int private)
{
  struct timespec ts32;

  if (abstime != NULL && ! in_time_t_range (abstime->tv_sec))
    return -EOVERFLOW;

  unsigned int clockbit = (clockid == CLOCK_REALTIME) ?
    FUTEX_CLOCK_REALTIME : 0;
  int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  if (abstime != NULL)
    ts32 = valid_timespec64_to_timespec (*abstime);

  return INTERNAL_SYSCALL_CALL (futex, futex_word, op, expected,
                                abstime != NULL ? &ts32 : NULL,
                                NULL /* Unused.  */, FUTEX_BITSET_MATCH_ANY);
}

static int
__futex_clock_wait_bitset32 (int *futexp, int val, clockid_t clockid,
                             const struct __timespec64 *abstime, int private)
{
  struct timespec ts32;

  if (abstime != NULL && ! in_time_t_range (abstime->tv_sec))
    return -EOVERFLOW;

  const unsigned int clockbit =
    (clockid == CLOCK_REALTIME) ? FUTEX_CLOCK_REALTIME : 0;
  const int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  if (abstime != NULL)
    ts32 = valid_timespec64_to_timespec (*abstime);

  return INTERNAL_SYSCALL_CALL (futex, futexp, op, val,
                                abstime != NULL ? &ts32 : NULL,
                                NULL /* Unused.  */, FUTEX_BITSET_MATCH_ANY);
}
#endif /* ! __ASSUME_TIME64_SYSCALLS */

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
    err = __futex_abstimed_wait_cancelable32 (futex_word, expected,
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

int
__futex_abstimed_wait64 (unsigned int* futex_word, unsigned int expected,
                         clockid_t clockid,
                         const struct __timespec64* abstime, int private)
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

  err = INTERNAL_SYSCALL_CALL (futex_time64, futex_word, op, expected,
                               abstime, NULL /* Unused.  */,
                               FUTEX_BITSET_MATCH_ANY);
#ifndef __ASSUME_TIME64_SYSCALLS
  if (err == -ENOSYS)
    err = __futex_abstimed_wait32 (futex_word, expected,
                                   clockid, abstime, private);
#endif
  switch (err)
    {
    case 0:
    case -EAGAIN:
    case -EINTR:
    case -ETIMEDOUT:
      return -err;

    case -EFAULT: /* Must have been caused by a glibc or application bug.  */
    case -EINVAL: /* Either due to wrong alignment, unsupported
		     clockid or due to the timeout not being
		     normalized. Must have been caused by a glibc or
		     application bug.  */
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      futex_fatal_error ();
    }
}

int
__futex_clocklock_wait64 (int *futex, int val, clockid_t clockid,
                          const struct __timespec64 *abstime, int private)
{
  struct __timespec64 ts, *tsp = NULL;

  if (abstime != NULL)
    {
      /* Reject invalid timeouts.  */
      if (! valid_nanoseconds (abstime->tv_nsec))
        return EINVAL;

      /* Get the current time. This can only fail if clockid is not valid.  */
      if (__glibc_unlikely (__clock_gettime64 (clockid, &ts) != 0))
        return EINVAL;

      /* Compute relative timeout.  */
      ts.tv_sec = abstime->tv_sec - ts.tv_sec;
      ts.tv_nsec = abstime->tv_nsec - ts.tv_nsec;
      if (ts.tv_nsec < 0)
        {
	  ts.tv_nsec += 1000000000;
	  --ts.tv_sec;
        }

      if (ts.tv_sec < 0)
        return ETIMEDOUT;

      tsp = &ts;
    }

  int err = INTERNAL_SYSCALL_CALL (futex_time64, futex,
                                   __lll_private_flag (FUTEX_WAIT, private),
                                   val, tsp);
#ifndef __ASSUME_TIME64_SYSCALLS
  if (err == -ENOSYS)
    {
      if (tsp != NULL && ! in_time_t_range (tsp->tv_sec))
        return EOVERFLOW;

      struct timespec ts32;
      if (tsp != NULL)
        ts32 = valid_timespec64_to_timespec (*tsp);

      err = INTERNAL_SYSCALL_CALL (futex, futex,
                                   __lll_private_flag (FUTEX_WAIT, private),
                                   val, tsp != NULL ? &ts32 : NULL);
    }
#endif

  return -err;
}

int
__futex_clock_wait_bitset64 (int *futexp, int val, clockid_t clockid,
                             const struct __timespec64 *abstime,
                             int private)
{
  int ret;
  if (! lll_futex_supported_clockid (clockid))
    {
      return -EINVAL;
    }

  const unsigned int clockbit =
    (clockid == CLOCK_REALTIME) ? FUTEX_CLOCK_REALTIME : 0;
  const int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, private);

  ret = INTERNAL_SYSCALL_CALL (futex_time64, futexp, op, val,
                               abstime, NULL /* Unused.  */,
                               FUTEX_BITSET_MATCH_ANY);
#ifndef __ASSUME_TIME64_SYSCALLS
  if (ret == -ENOSYS)
    ret = __futex_clock_wait_bitset32 (futexp, val, clockid, abstime, private);
#endif
  return ret;
}
