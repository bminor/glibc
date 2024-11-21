/* Smoke test to verify that pthread_getcpuclockid fails with ESRCH when the
   thread in question has exited.
   Copyright the GNU Toolchain Authors.
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
#include <pthread.h>
#include <sched.h>
#include <time.h>

#include <support/check.h>
#include <support/test-driver.h>
#include <support/xthread.h>

void *
thr (void *in)
{
  return in;
}

int
do_test (void)
{
  clockid_t c;
  pthread_t t = xpthread_create (NULL, thr, NULL);

  int ret = 0;
  while ((ret = pthread_getcpuclockid (t, &c)) == 0)
    sched_yield ();

  TEST_COMPARE (ret, ESRCH);

  return 0;
}

#include <support/test-driver.c>
