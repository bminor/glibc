/* Copyright (C) 2002-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include "pthreadP.h"

int
__pthread_timedjoin_np64 (pthread_t threadid, void **thread_return,
                          const struct __timespec64 *abstime)
{
  return __pthread_clockjoin_ex (threadid, thread_return,
                                 CLOCK_REALTIME, abstime, true);
}

#if __TIMESIZE != 64
libc_hidden_def (__pthread_timedjoin_np64)

int
__pthread_timedjoin_np (pthread_t threadid, void **thread_return,
                        const struct timespec *abstime)
{
  if (abstime != NULL)
    {
      struct __timespec64 ts64 = valid_timespec_to_timespec64 (*abstime);
      return __pthread_timedjoin_np64 (threadid, thread_return, &ts64);
    }
  else
    return __pthread_timedjoin_np64 (threadid, thread_return, NULL);
}
#endif
weak_alias (__pthread_timedjoin_np, pthread_timedjoin_np)
