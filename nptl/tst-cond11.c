/* Copyright (C) 2003-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>


#if defined _POSIX_CLOCK_SELECTION && _POSIX_CLOCK_SELECTION >= 0
static int
run_test (clockid_t cl)
{
  pthread_condattr_t condattr;
  pthread_cond_t cond;
  pthread_mutexattr_t mutattr;
  pthread_mutex_t mut;

  printf ("clock = %d\n", (int) cl);

  TEST_COMPARE (pthread_condattr_init (&condattr), 0);
  TEST_COMPARE (pthread_condattr_setclock (&condattr, cl), 0);

  clockid_t cl2;
  TEST_COMPARE (pthread_condattr_getclock (&condattr, &cl2), 0);
  TEST_COMPARE (cl, cl2);

  TEST_COMPARE (pthread_cond_init (&cond, &condattr), 0);
  TEST_COMPARE (pthread_condattr_destroy (&condattr), 0);

  xpthread_mutexattr_init (&mutattr);
  xpthread_mutexattr_settype (&mutattr, PTHREAD_MUTEX_ERRORCHECK);
  xpthread_mutex_init (&mut, &mutattr);
  xpthread_mutexattr_destroy (&mutattr);

  xpthread_mutex_lock (&mut);
  TEST_COMPARE (pthread_mutex_lock (&mut), EDEADLK);

  struct timespec ts_timeout;
  xclock_gettime (cl, &ts_timeout);

  /* Wait one second.  */
  ++ts_timeout.tv_sec;

  TEST_COMPARE (pthread_cond_timedwait (&cond, &mut, &ts_timeout), ETIMEDOUT);
  TEST_TIMESPEC_BEFORE_NOW (ts_timeout, cl);

  xpthread_mutex_unlock (&mut);
  xpthread_mutex_destroy (&mut);
  TEST_COMPARE (pthread_cond_destroy (&cond), 0);

  return 0;
}
#endif


static int
do_test (void)
{
#if !defined _POSIX_CLOCK_SELECTION || _POSIX_CLOCK_SELECTION == -1

  FAIL_UNSUPPORTED ("_POSIX_CLOCK_SELECTION not supported, test skipped");

#else

  run_test (CLOCK_REALTIME);

# if defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK >= 0
#  if _POSIX_MONOTONIC_CLOCK == 0
  int e = sysconf (_SC_MONOTONIC_CLOCK);
  if (e < 0)
    puts ("CLOCK_MONOTONIC not supported");
  else if (e == 0)
      FAIL_RET ("sysconf (_SC_MONOTONIC_CLOCK) must not return 0");
  else
#  endif
    run_test (CLOCK_MONOTONIC);
# else
  puts ("_POSIX_MONOTONIC_CLOCK not defined");
# endif

  return 0;
#endif
}

#include <support/test-driver.c>
