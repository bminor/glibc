/* low level locking for pthread library.  SPARC version.
   Copyright (C) 2003-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Paul Mackerras <paulus@au.ibm.com>, 2003.

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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <sys/time.h>


void
__lll_lock_wait_private (int *futex)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_24_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, LLL_PRIVATE);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);
}


/* These functions don't get included in libc.so  */
#if IS_IN (libpthread)
void
__lll_lock_wait (int *futex, int private)
{
  do
    {
      int oldval = atomic_compare_and_exchange_val_24_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_wait (futex, 2, private);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);
}


int
__lll_clocklock_wait (int *futex, clockid_t clockid,
                      const struct timespec *abstime, int private)
{
  /* Reject invalid timeouts.  */
  if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000)
    return EINVAL;

  do
    {
      struct timespec ts;
      struct timespec rt;

      /* Get the current time. This can only fail if clockid is not
         valid. */
      if (__glibc_unlikely (__clock_gettime (clockid, &ts) != 0))
        return EINVAL;

      /* Compute relative timeout.  */
      rt.tv_sec = abstime->tv_sec - ts.tv_sec;
      rt.tv_nsec = abstime->tv_nsec - ts.tv_nsec;
      if (rt.tv_nsec < 0)
	{
	  rt.tv_nsec += 1000000000;
	  --rt.tv_sec;
	}

      /* Already timed out?  */
      if (rt.tv_sec < 0)
	return ETIMEDOUT;

      /* Wait.  */
      int oldval = atomic_compare_and_exchange_val_24_acq (futex, 2, 1);
      if (oldval != 0)
	lll_futex_timed_wait (futex, 2, &rt, private);
    }
  while (atomic_compare_and_exchange_val_24_acq (futex, 2, 0) != 0);

  return 0;
}
#endif
