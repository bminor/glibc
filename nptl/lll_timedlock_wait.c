/* Timed low level locking for pthread library.  Generic futex-using version.
   Copyright (C) 2003-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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

#include <atomic.h>
#include <errno.h>
#include <lowlevellock.h>
#include <sys/time.h>


int
__lll_clocklock_wait (int *futex, clockid_t clockid,
		      const struct timespec *abstime, int private)
{
  /* Reject invalid timeouts.  */
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  /* Try locking.  */
  while (atomic_exchange_acq (futex, 2) != 0)
    {
      struct timespec ts;

      /* Get the current time. This can only fail if clockid is not
         valid.  */
      if (__glibc_unlikely (__clock_gettime (clockid, &ts) != 0))
        return EINVAL;

      /* Compute relative timeout.  */
      struct timespec rt;
      rt.tv_sec = abstime->tv_sec - ts.tv_sec;
      rt.tv_nsec = abstime->tv_nsec - ts.tv_nsec;
      if (rt.tv_nsec < 0)
        {
          rt.tv_nsec += 1000000000;
          --rt.tv_sec;
        }

      if (rt.tv_sec < 0)
        return ETIMEDOUT;

      /* If *futex == 2, wait until woken or timeout.  */
      lll_futex_timed_wait (futex, 2, &rt, private);
    }

  return 0;
}
