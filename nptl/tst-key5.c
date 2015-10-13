/* Copyright (C) 2014 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>


pthread_key_t key;
void *value;
size_t r;
static void
handler (int signo)
{
  void *ret = pthread_getspecific (key);
  /* We race with the setspecific--either result is fine, just not junk. */
  assert (ret == value || ret == NULL);
  r++;
}


int
do_test (void)
{
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = handler;

  assert (0 == sigaction (SIGUSR1, &sa, NULL));

  timer_t timer;
  struct sigevent sevp;
  sevp.sigev_notify = SIGEV_SIGNAL;
  sevp.sigev_signo = SIGUSR1;
  assert (0 == timer_create(CLOCK_MONOTONIC, &sevp, &timer));
  struct itimerspec spec;
  spec.it_value.tv_sec = 0;
  spec.it_value.tv_nsec = 500;
  spec.it_interval = spec.it_value;
  timer_settime(timer, 0, &spec, NULL);
#define NITERS (1000 * 1000)
  for (int i = 0; i < NITERS; ++i)
    {
      value = (void *)((intptr_t)i + 1);
      assert (0 == pthread_key_create(&key, NULL));
      assert (0 == pthread_setspecific(key, value));
      if (value != pthread_getspecific(key))
        {
          printf ("Lost a value\n");
          return 1;
        }
      assert (0 == pthread_key_delete(key));
    }
  timer_delete(timer);
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
