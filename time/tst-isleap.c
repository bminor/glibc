/* Test leap year processing.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <time.h>
#include <support/check.h>

static int
do_test (void)
{
  TEST_VERIFY (!__time_isleap (-100));
  TEST_VERIFY (__time_isleap (0));
  TEST_VERIFY (!__time_isleap (100));
  TEST_VERIFY (!__time_isleap (200));
  TEST_VERIFY (!__time_isleap (300));
  TEST_VERIFY (__time_isleap (400));
  TEST_VERIFY (!__time_isleap (1900));
  TEST_VERIFY (__time_isleap (1996));
  TEST_VERIFY (__time_isleap (2000));
  TEST_VERIFY (__time_isleap (2004));
  TEST_VERIFY (__time_isleap (2008));
  TEST_VERIFY (!__time_isleap (2100));
  return 0;
}

#include <support/test-driver.c>
