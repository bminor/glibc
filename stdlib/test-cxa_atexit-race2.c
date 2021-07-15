/* Support file for atexit/exit, etc. race tests.
   Copyright (C) 2017-2021 Free Software Foundation, Inc.
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

/* This file must be run from within a directory called "stdlib".  */

/* The atexit/exit, at_quick_exit/quick_exit, __cxa_atexit/exit, etc. exhibited
   data race while calling destructors.

   This test registers destructors from the background thread, and checks that
   the same destructor is not called more than once.  */

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <support/xthread.h>
#include <sys/wait.h>
#include <unistd.h>

static atomic_int registered;
static atomic_int todo = 100000;

static void
atexit_cb (void *arg)
{
  atomic_fetch_sub (&registered, 1);
  static void *prev;
  if (arg == prev)
    {
      printf ("%p\n", arg);
      abort ();
    }
  prev = arg;

  while (atomic_load (&todo) > 0 && atomic_load (&registered) < 100)
    ;
}

int __cxa_atexit (void (*func) (void *), void *arg, void *d);

static void *cb_arg = NULL;
static void
add_handlers (void)
{
  int n = 10;
  for (int i = 0; i < n; ++i)
    __cxa_atexit (&atexit_cb, ++cb_arg, 0);
  atomic_fetch_add (&registered, n);
  atomic_fetch_sub (&todo, n);
}

static void *
thread_func (void *arg)
{
  while (atomic_load (&todo) > 0)
    if (atomic_load (&registered) < 10000)
      add_handlers ();
  return 0;
}

static void
test_and_exit (void)
{
  pthread_attr_t attr;

  xpthread_attr_init (&attr);
  xpthread_attr_setdetachstate (&attr, 1);

  xpthread_create (&attr, thread_func, NULL);
  xpthread_attr_destroy (&attr);
  while (!atomic_load (&registered))
    ;
  exit (0);
}

static int
do_test (void)
{
  for (int i = 0; i < 20; ++i)
    {
      for (int i = 0; i < 10; ++i)
        if (fork () == 0)
          test_and_exit ();

      int status;
      while (wait (&status) > 0)
        {
          if (!WIFEXITED (status))
            {
              printf ("Failed interation %d\n", i);
              abort ();
            }
        }
    }

  exit (0);
}

#define TEST_FUNCTION do_test
#include <support/test-driver.c>
