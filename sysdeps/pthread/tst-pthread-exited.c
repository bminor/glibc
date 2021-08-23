/* Test pthread interface which should return ESRCH when issues along
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
#include <time.h>

static void *
noop_thread (void *closure)
{
  return NULL;
}

static int
do_test (void)
{
  pthread_t thr = xpthread_create (NULL, noop_thread, NULL);

  support_wait_for_thread_exit ();

  {
    cpu_set_t cpuset;
    int r = pthread_getaffinity_np (thr, sizeof (cpuset), &cpuset);
    TEST_COMPARE (r, EINVAL);
  }

  {
    clockid_t clkid;
    int r = pthread_getcpuclockid (thr, &clkid);
    TEST_COMPARE (r, EINVAL);
  }

  {
    struct sched_param sch = { 0 };
    int r = pthread_setschedparam (thr, SCHED_FIFO, &sch);
    TEST_COMPARE (r, EINVAL);
  }

  xpthread_join (thr);

  return 0;
}

#include <support/test-driver.c>
