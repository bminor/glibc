/* Copyright (C) 2002-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xtime.h>


static int
do_test (void)
{
  sem_t s;
  struct timespec ts;

  TEST_COMPARE (sem_init (&s, 0, 1), 0);
  TEST_COMPARE (TEMP_FAILURE_RETRY (sem_wait (&s)), 0);

  /* We wait for half a second.  */
  xclock_gettime (CLOCK_REALTIME, &ts);
  ts = timespec_add (ts, make_timespec (0, TIMESPEC_HZ/2));

  errno = 0;
  TEST_COMPARE (TEMP_FAILURE_RETRY (sem_timedwait (&s, &ts)), -1);
  TEST_COMPARE (errno, ETIMEDOUT);
  TEST_TIMESPEC_NOW_OR_AFTER (CLOCK_REALTIME, ts);

  return 0;
}

#include <support/test-driver.c>
