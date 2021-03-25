/* Test for select timeout.
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
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xunistd.h>
#include <support/xtime.h>

struct child_args
{
  int fds[2][2];
  struct timeval tmo;
};

static void
do_test_child (void *clousure)
{
  struct child_args *args = (struct child_args *) clousure;

  close (args->fds[0][1]);
  close (args->fds[1][0]);

  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET (args->fds[0][0], &rfds);

  struct timespec ts = xclock_now (CLOCK_REALTIME);
  ts = timespec_add (ts, (struct timespec) { args->tmo.tv_sec, 0 });

  int r = select (args->fds[0][0] + 1, &rfds, NULL, NULL, &args->tmo);
  TEST_COMPARE (r, 0);

  TEST_TIMESPEC_NOW_OR_AFTER (CLOCK_REALTIME, ts);

  xwrite (args->fds[1][1], "foo", 3);
}

static int
do_test (void)
{
  struct child_args args;

  xpipe (args.fds[0]);
  xpipe (args.fds[1]);

  /* The child select should timeout and write on its pipe end.  */
  args.tmo = (struct timeval) { .tv_sec = 0, .tv_usec = 250000 };
  {
    struct support_capture_subprocess result;
    result = support_capture_subprocess (do_test_child, &args);
    support_capture_subprocess_check (&result, "tst-select-child", 0,
				      sc_allow_none);
  }

  /* Same as before, but simulating polling.  */
  args.tmo = (struct timeval) { .tv_sec = 0, .tv_usec = 0 };
  {
    struct support_capture_subprocess result;
    result = support_capture_subprocess (do_test_child, &args);
    support_capture_subprocess_check (&result, "tst-select-child", 0,
				      sc_allow_none);
  }

  xclose (args.fds[0][0]);
  xclose (args.fds[1][1]);

  {
    fd_set rfds;
    FD_ZERO (&rfds);
    FD_SET (args.fds[1][0], &rfds);

    int r = select (args.fds[1][0] + 1, &rfds, NULL, NULL, &args.tmo);
    TEST_COMPARE (r, 1);
  }

  return 0;
}

#include <support/test-driver.c>
