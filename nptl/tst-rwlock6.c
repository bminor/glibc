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
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>


static int kind[] =
  {
    PTHREAD_RWLOCK_PREFER_READER_NP,
    PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
    PTHREAD_RWLOCK_PREFER_WRITER_NP,
  };


static void *
tf (void *arg)
{
  pthread_rwlock_t *r = arg;

  /* Timeout: 0.3 secs.  */
  struct timespec ts_start;
  xclock_gettime (CLOCK_REALTIME, &ts_start);

  struct timespec ts_timeout = timespec_add (ts_start,
                                             make_timespec (0, 300000000));

  puts ("child calling timedrdlock");

  TEST_COMPARE (pthread_rwlock_timedrdlock (r, &ts_timeout), ETIMEDOUT);

  puts ("1st child timedrdlock done");

  TEST_TIMESPEC_NOW_OR_AFTER (CLOCK_REALTIME, ts_timeout);

  xclock_gettime (CLOCK_REALTIME, &ts_timeout);
  ts_timeout.tv_sec += 10;
  /* Note that the following operation makes ts invalid.  */
  ts_timeout.tv_nsec += 1000000000;

  TEST_COMPARE (pthread_rwlock_timedrdlock (r, &ts_timeout), EINVAL);

  puts ("2nd child timedrdlock done");

  return NULL;
}


static int
do_test (void)
{
  size_t cnt;
  for (cnt = 0; cnt < sizeof (kind) / sizeof (kind[0]); ++cnt)
    {
      pthread_rwlock_t r;
      pthread_rwlockattr_t a;

      if (pthread_rwlockattr_init (&a) != 0)
        FAIL_EXIT1 ("round %Zu: rwlockattr_t failed\n", cnt);

      if (pthread_rwlockattr_setkind_np (&a, kind[cnt]) != 0)
        FAIL_EXIT1 ("round %Zu: rwlockattr_setkind failed\n", cnt);

      if (pthread_rwlock_init (&r, &a) != 0)
        FAIL_EXIT1 ("round %Zu: rwlock_init failed\n", cnt);

      if (pthread_rwlockattr_destroy (&a) != 0)
        FAIL_EXIT1 ("round %Zu: rwlockattr_destroy failed\n", cnt);

      struct timespec ts;
      xclock_gettime (CLOCK_REALTIME, &ts);
      ++ts.tv_sec;

      /* Get a write lock.  */
      int e = pthread_rwlock_timedwrlock (&r, &ts);
      if (e != 0)
        FAIL_EXIT1 ("round %Zu: rwlock_timedwrlock failed (%d)\n", cnt, e);

      puts ("1st timedwrlock done");

      xclock_gettime (CLOCK_REALTIME, &ts);
      ++ts.tv_sec;
      TEST_COMPARE (pthread_rwlock_timedrdlock (&r, &ts), EDEADLK);

      puts ("1st timedrdlock done");

      xclock_gettime (CLOCK_REALTIME, &ts);
      ++ts.tv_sec;
      TEST_COMPARE (pthread_rwlock_timedwrlock (&r, &ts), EDEADLK);

      puts ("2nd timedwrlock done");

      pthread_t th;
      if (pthread_create (&th, NULL, tf, &r) != 0)
        FAIL_EXIT1 ("round %Zu: create failed\n", cnt);

      puts ("started thread");

      void *status;
      if (pthread_join (th, &status) != 0)
        FAIL_EXIT1 ("round %Zu: join failed\n", cnt);
      if (status != NULL)
        FAIL_EXIT1 ("failure in round %Zu\n", cnt);

      puts ("joined thread");

      if (pthread_rwlock_destroy (&r) != 0)
        FAIL_EXIT1 ("round %Zu: rwlock_destroy failed\n", cnt);
    }

  return 0;
}

#include <support/test-driver.c>
