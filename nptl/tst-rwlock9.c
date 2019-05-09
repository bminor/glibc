/* Test program for timedout read/write lock functions.
   Copyright (C) 2000-2019 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <error.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <support/check.h>
#include <support/timespec.h>


#define NWRITERS 15
#define WRITETRIES 10
#define NREADERS 15
#define READTRIES 15

static const struct timespec timeout = { 0,1000000 };
static const struct timespec delay = { 0, 1000000 };

#ifndef KIND
# define KIND PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
#endif

static pthread_rwlock_t lock;


static void *
writer_thread (void *nr)
{
  struct timespec ts;
  int n;

  for (n = 0; n < WRITETRIES; ++n)
    {
      int e;
      do
	{
	  xclock_gettime (CLOCK_REALTIME, &ts);

          ts = timespec_add (ts, timeout);
          ts = timespec_add (ts, timeout);

	  printf ("writer thread %ld tries again\n", (long int) nr);

	  e = pthread_rwlock_timedwrlock (&lock, &ts);
	  if (e != 0 && e != ETIMEDOUT)
            FAIL_EXIT1 ("timedwrlock failed");
	}
      while (e == ETIMEDOUT);

      printf ("writer thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      if (pthread_rwlock_unlock (&lock) != 0)
        FAIL_EXIT1 ("unlock for writer failed");

      printf ("writer thread %ld released\n", (long int) nr);
    }

  return NULL;
}


static void *
reader_thread (void *nr)
{
  struct timespec ts;
  int n;

  for (n = 0; n < READTRIES; ++n)
    {
      int e;
      do
	{
	  xclock_gettime (CLOCK_REALTIME, &ts);

          ts = timespec_add (ts, timeout);

	  printf ("reader thread %ld tries again\n", (long int) nr);

	  e = pthread_rwlock_timedrdlock (&lock, &ts);
	  if (e != 0 && e != ETIMEDOUT)
            FAIL_EXIT1 ("timedrdlock failed");
	}
      while (e == ETIMEDOUT);

      printf ("reader thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      if (pthread_rwlock_unlock (&lock) != 0)
        FAIL_EXIT1 ("unlock for reader failed");

      printf ("reader thread %ld released\n", (long int) nr);
    }

  return NULL;
}


static int
do_test (void)
{
  pthread_t thwr[NWRITERS];
  pthread_t thrd[NREADERS];
  int n;
  void *res;
  pthread_rwlockattr_t a;

  if (pthread_rwlockattr_init (&a) != 0)
    FAIL_EXIT1 ("rwlockattr_t failed");

  if (pthread_rwlockattr_setkind_np (&a, KIND) != 0)
    FAIL_EXIT1 ("rwlockattr_setkind failed");

  if (pthread_rwlock_init (&lock, &a) != 0)
    FAIL_EXIT1 ("rwlock_init failed");

  /* Make standard error the same as standard output.  */
  dup2 (1, 2);

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  for (n = 0; n < NWRITERS; ++n)
    if (pthread_create (&thwr[n], NULL, writer_thread,
			(void *) (long int) n) != 0)
      FAIL_EXIT1 ("writer create failed");

  for (n = 0; n < NREADERS; ++n)
    if (pthread_create (&thrd[n], NULL, reader_thread,
			(void *) (long int) n) != 0)
      FAIL_EXIT1 ("reader create failed");

  /* Wait for all the threads.  */
  for (n = 0; n < NWRITERS; ++n)
    if (pthread_join (thwr[n], &res) != 0)
      FAIL_EXIT1 ("writer join failed");
  for (n = 0; n < NREADERS; ++n)
    if (pthread_join (thrd[n], &res) != 0)
      FAIL_EXIT1 ("reader join failed");

  return 0;
}

#define TIMEOUT 30
#include <support/test-driver.c>
