/* Check re-use timer id for SIGEV_THREAD (BZ 32833)
   Copyright (C) 2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   not, see <https://www.gnu.org/licenses/>.  */

#include <signal.h>
#include <time.h>
#include <support/check.h>

/* The test depends of the system load and scheduler pressure, so the
   number of iteration is arbitrary to not take too much time.  */
enum { niters = 1<<13 };

static void
on_good_timer (union sigval sv)
{
}

static void
on_bad_timer (union sigval sv)
{
  FAIL_EXIT1 ("triggered bad timer");
}

static int
do_test (void)
{
  struct itimerspec its_long =  {. it_value = { .tv_sec = 180 } };
  struct itimerspec its_short = { .it_value = { .tv_nsec = 1000 } };
  struct itimerspec its_zero =  { .it_interval = { .tv_sec = 0} };

  struct sigevent ev_short =
    {
      .sigev_notify = SIGEV_THREAD,
      .sigev_notify_function = on_good_timer,
    };

  struct sigevent ev_long =
    {
      .sigev_notify = SIGEV_THREAD,
      .sigev_notify_function = on_bad_timer,
    };

  for (int which = 0; which < niters; which++)
    {
      struct sigevent * ev = which & 0x1 ? &ev_short : &ev_long;
      struct itimerspec * its = which & 0x1? &its_short : &its_long;

      timer_t timerid;
      if (timer_create (CLOCK_REALTIME, ev, &timerid) == -1)
	FAIL_EXIT1 ("timer_create: %m");

      if (timer_settime (timerid, 0, its, NULL) == -1)
	FAIL_EXIT1 ("timer_settime: %m");

      if (timer_settime (timerid, 0, &its_zero, NULL) == -1)
	FAIL_EXIT1 ("timer_settime: %m");

      if (timer_delete (timerid) == -1)
	FAIL_EXIT1 ("time_delete: %m");
    }

  return 0;
}

#include <support/test-driver.c>
