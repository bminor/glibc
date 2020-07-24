/* Verify that ftime is sane.
   Copyright (C) 2014-2020 Free Software Foundation, Inc.
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


#include <shlib-compat.h>
#if TEST_COMPAT (libc, GLIBC_2_0, GLIBC_2_33)
#include <time.h>
#include <support/check.h>

compat_symbol_reference (libc, ftime, ftime, GLIBC_2_0);

struct timeb
  {
    time_t time;
    unsigned short int millitm;
    short int timezone;
    short int dstflag;
  };

extern int ftime (struct timeb *__timebuf);

static int
do_test (void)
{
  struct timeb prev, curr = {.time = 0, .millitm = 0};
  int sec = 0;

  while (sec != 3)
    {
      prev = curr;

      /* ftime was deprecated on 2.31 and removed on 2.33.  */
      TEST_COMPARE (ftime (&curr), 0);
      TEST_VERIFY_EXIT (curr.time >= prev.time);
      if (curr.time == prev.time)
	TEST_VERIFY_EXIT (curr.millitm >= prev.millitm);

      if (curr.time > prev.time)
        sec ++;
    }
  return 0;
}
#else
static int
do_test (void)
{
  return EXIT_UNSUPPORTED;
}
#endif

#include <support/test-driver.c>
