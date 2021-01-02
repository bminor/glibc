/* Join with a terminated thread using an specific clock.
   Copyright (C) 2019-2021 Free Software Foundation, Inc.
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

#include <time.h>
#include <futex-internal.h>
#include "pthreadP.h"

int
__pthread_clockjoin_np64 (pthread_t threadid, void **thread_return,
                          clockid_t clockid, const struct __timespec64 *abstime)
{
  if (!futex_abstimed_supported_clockid (clockid))
    return EINVAL;

  return __pthread_clockjoin_ex (threadid, thread_return,
                                 clockid, abstime, true);
}

#if __TIMESIZE != 64
libc_hidden_def (__pthread_clockjoin_np64)

int
__pthread_clockjoin_np (pthread_t threadid, void **thread_return,
                        clockid_t clockid, const struct timespec *abstime)
{
  if (abstime != NULL)
    {
      struct __timespec64 ts64 = valid_timespec_to_timespec64 (*abstime);
      return __pthread_clockjoin_np64 (threadid, thread_return, clockid,
				       &ts64);
    }
  else
      return __pthread_clockjoin_np64 (threadid, thread_return, clockid,
				       NULL);
}
#endif
weak_alias (__pthread_clockjoin_np, pthread_clockjoin_np)
