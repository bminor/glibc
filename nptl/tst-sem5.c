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


static int
do_test (void)
{
  sem_t s;
  struct timespec ts;
  struct timeval tv;

  TEST_COMPARE (sem_init (&s, 0, 1), 0);
  TEST_COMPARE (TEMP_FAILURE_RETRY (sem_wait (&s)), 0);
  TEST_COMPARE (gettimeofday (&tv, NULL), 0);

  TIMEVAL_TO_TIMESPEC (&tv, &ts);

  /* We wait for half a second.  */
  ts.tv_nsec += 500000000;
  if (ts.tv_nsec >= 1000000000)
    {
      ++ts.tv_sec;
      ts.tv_nsec -= 1000000000;
    }

  errno = 0;
  TEST_COMPARE (TEMP_FAILURE_RETRY (sem_timedwait (&s, &ts)), -1);
  TEST_COMPARE (errno, ETIMEDOUT);

  struct timespec ts2;
  TEST_COMPARE (clock_gettime (CLOCK_REALTIME, &ts2), 0);

  TEST_VERIFY (ts2.tv_sec > ts.tv_sec
               || (ts2.tv_sec == ts.tv_sec && ts2.tv_nsec > ts.tv_nsec));

  return 0;
}

#include <support/test-driver.c>
