/* Test concurrent dlopen and pthread_create: BZ 19329.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <atomic.h>

#include <support/xthread.h>

#define THREADS 10000

static volatile int done;

static void *
start (void *a)
{
  if (dlopen ("tst-tls7mod.so", RTLD_LAZY) == NULL)
    {
      printf ("dlopen failed: %s\n", dlerror ());
      exit (1);
    }
  atomic_store_relaxed (&done, 1);
  return 0;
}

static void *
nop (void *a)
{
  return 0;
}

static int
do_test (void)
{
  pthread_t t1, t2;
  int i;

  /* Load a module with lots of dependencies and TLS.  */
  t1 = xpthread_create (0, start, 0);

  /* Concurrently create lots of threads until dlopen is observably done.  */
  for (i = 0; i < THREADS && !atomic_load_relaxed (&done); i++)
    {
      t2 = xpthread_create (0, nop, 0);
      xpthread_join (t2);
    }

  xpthread_join (t1);
  printf ("threads created during dlopen: %d\n", i);
  return 0;
}

#include <support/test-driver.c>
