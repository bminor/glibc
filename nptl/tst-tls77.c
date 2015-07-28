/* Copyright (C) 2015 Free Software Foundation, Inc.
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
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthreaddef.h>

static pthread_t th;

pthread_barrier_t b;

void *(*dfp)(void*) = NULL;

void *
caller(void *x)
{
  pthread_barrier_wait (&b);

  if (dfp)
    dfp(NULL);

  return NULL;
}

int
do_test (void)
{
  if (pthread_barrier_init (&b, NULL, 2) != 0)
    {
      puts ("barrier_init failed");
      exit (1);
    }

  pthread_attr_t a;

  if (pthread_attr_init (&a) != 0)
    {
      puts ("attr_init failed");
      exit (1);
    }

  if (pthread_attr_setstacksize (&a, 1 * 1024 * 1024) != 0)
    {
      puts ("attr_setstacksize failed");
      return 1;
    }

  if (pthread_create (&th, &a, caller, 0) != 0)
    {
      puts ("pthread_create failed");
      exit (1);
    }

  void *h = dlopen ("tst-tls77mod.so", RTLD_LAZY);
  if (h == NULL)
    {
      puts ("dlopen failed");
      exit (1);
    }

  dfp = dlsym (h, "toucher");
  if (dfp == NULL)
    {
      printf ("dlsym for toucher failed %s\n", dlerror ());
      return 1;
    }

  pthread_barrier_wait (&b);

  if (pthread_join (th, NULL) != 0)
    {
      puts ("join failed");
      exit (1);
    }

  if (pthread_attr_destroy (&a) != 0)
    {
      puts ("attr_destroy failed");
      exit (1);
    }

  return 0;
}


#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
