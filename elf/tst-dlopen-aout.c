/* Test case for BZ #16634.

   Verify that incorrectly dlopen()ing an executable without
   __RTLD_OPENEXEC does not cause assertion in ld.so.

   Copyright (C) 2014-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   Note: this test currently only fails when glibc is configured with
   --enable-hardcoded-path-in-tests.  */

#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <pthread.h>
#include <support/xthread.h>

__thread int x;

void *
fn (void *p)
{
  return p;
}

static int
do_test (int argc, char *argv[])
{
  int j;

  for (j = 0; j < 100; ++j)
    {
      pthread_t thr;
      void *p;

      p = dlopen (argv[0], RTLD_LAZY);
      if (p != NULL)
        {
          fprintf (stderr, "dlopen unexpectedly succeeded\n");
          return 1;
        }
      /* We create threads to force TLS allocation, which triggers
	 the original bug i.e. running out of surplus slotinfo entries
	 for TLS.  */
      thr = xpthread_create (NULL, fn, NULL);
      xpthread_join (thr);
    }

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
