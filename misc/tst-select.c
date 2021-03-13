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

#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/select.h>
#include <support/check.h>
#include <support/xtime.h>
#include <support/timespec.h>

#define TST_SELECT_TIMEOUT 1
#define TST_SELECT_FD_ERR 2

static int
test_select_timeout (bool zero_tmo)
{
  const int fds = TST_SELECT_FD_ERR;
  int timeout = TST_SELECT_TIMEOUT;
  struct timeval to = { 0, 0 };
  struct timespec ts;
  fd_set rfds;

  FD_ZERO (&rfds);
  FD_SET (fds, &rfds);

  if (zero_tmo)
    timeout = 0;

  to.tv_sec = timeout;
  ts = xclock_now (CLOCK_REALTIME);
  ts = timespec_add (ts, (struct timespec) { timeout, 0 });

  /* Wait for timeout.  */
  int ret = select (fds + 1, &rfds, NULL, NULL, &to);
  if (ret == -1)
    FAIL_EXIT1 ("select failed: %m\n");

  TEST_TIMESPEC_NOW_OR_AFTER (CLOCK_REALTIME, ts);

  return 0;
}

static int
do_test (void)
{
  /* Check if select exits immediately.  */
  test_select_timeout (true);

  /* Check if select exits after specified timeout.  */
  test_select_timeout (false);

  return 0;
}

#include <support/test-driver.c>
