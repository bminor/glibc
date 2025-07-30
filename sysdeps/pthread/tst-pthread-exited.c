/* Test pthread interface which should return ESRCH when issued
   with a terminated pthread_t.

   Copyright (C) 2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xthread.h>

static void *
noop_thread (void *closure)
{
  return NULL;
}

enum { nthreads = 1 };

static int
do_test_default (void)
{
  pthread_t thrs[nthreads];
  for (int i = 0; i < nthreads; i++)
   thrs[i] = xpthread_create (NULL, noop_thread, NULL);

  support_wait_for_thread_exit ();

  for (int i = 0; i < nthreads; i++)
    {
      clockid_t clk;
      TEST_COMPARE (pthread_getcpuclockid (thrs[i], &clk), ESRCH);

      struct sched_param sch = { 0 };
      int policy;
      TEST_COMPARE (pthread_getschedparam (thrs[i], &policy, &sch), ESRCH);

      TEST_COMPARE (pthread_setschedparam (thrs[i], SCHED_FIFO, &sch), ESRCH);

      TEST_COMPARE (pthread_setschedprio (thrs[i], 0), ESRCH);
    }

  for (int i = 0; i < nthreads; i++)
    xpthread_join (thrs[i]);

  return 0;
}


static void *
detached_pause_thread (void *closure)
{
  pthread_detach (pthread_self ());
  pause ();
  return NULL;
}

static void
do_test_detached (void)
{
  pthread_t thrs[nthreads];
  for (int i = 0; i < nthreads; i++)
    thrs[i] = xpthread_create (NULL, detached_pause_thread, NULL);

  for (int i = 0; i < nthreads; i++)
    {
      clockid_t clk;
      TEST_COMPARE (pthread_getcpuclockid (thrs[i], &clk), 0);

      struct sched_param sch = { 0 };
      int policy;
      TEST_COMPARE (pthread_getschedparam (thrs[i], &policy, &sch), 0);

      sch.sched_priority = 8;
      TEST_COMPARE (pthread_setschedparam (thrs[i], SCHED_FIFO, &sch), EPERM);

      TEST_COMPARE (pthread_setschedprio (thrs[i], 0), 0);
    }
}

static int
do_test (void)
{
  do_test_default ();
  do_test_detached ();

  return 0;
}

#include <support/test-driver.c>
