/* Test for timerfd related functions
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
#include <support/check.h>
#include <support/xunistd.h>
#include <support/timespec.h>
#include <sys/time.h>
#include <sys/timerfd.h>

static int
do_test (void)
{
  struct itimerspec settings = { { 0, 0 }, { 2, 0 } };
  struct itimerspec val;
  int fd, ret;

  fd = timerfd_create (CLOCK_REALTIME, 0);
  if (fd < 0)
    FAIL_EXIT1 ("*** timerfd_create failed: %m");

  /* Set the timer.  */
  ret = timerfd_settime (fd, 0, &settings, NULL);
  if (ret != 0)
    FAIL_EXIT1 ("*** timerfd_settime failed: %m\n");

  /* Sleep for 1 second.  */
  ret = usleep (1000000);
  if (ret != 0)
    FAIL_EXIT1 ("*** usleep failed: %m\n");

  /* Read the timer just after sleep.  */
  ret = timerfd_gettime (fd, &val);
  if (ret != 0)
    FAIL_EXIT1 ("*** timerfd_gettime failed: %m\n");

  /* Check difference between timerfd_gettime calls.  */
  TEST_COMPARE (support_timespec_check_in_range
                ((struct timespec) { 1, 0 }, val.it_value, 0.9, 1.0), 1);
  return 0;
}

#include <support/test-driver.c>
