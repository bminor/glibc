/* Test for utimes
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/temp_file.h>

static int temp_fd = -1;
char *testfile;

/* struct timeval array with Y2038 threshold minus 2 and 1 seconds.  */
const static struct timeval t1[2] = { { 0x7FFFFFFE, 0 },  { 0x7FFFFFFF, 0 } };

/* struct timeval array with Y2038 threshold plus 1 and 2 seconds.  */
const static
struct timeval t2[2] = { { 0x80000001ULL, 0 },  { 0x80000002ULL, 0 } };

/* struct timeval array around Y2038 threshold.  */
const static
struct timeval t3[2] = { { 0x7FFFFFFE, 0 },  { 0x80000002ULL, 0 } };

#define PREPARE do_prepare
static void
do_prepare (int argc, char *argv[])
{
  temp_fd = create_temp_file ("utimes", &testfile);
  TEST_VERIFY_EXIT (temp_fd > 0);
}

static int
test_utime_helper (const struct timeval *tv)
{
  struct stat64 st;
  int result;
  time_t t;

  /* Check if we run on port with 32 bit time_t size */
  if (__builtin_add_overflow (tv->tv_sec, 0, &t))
    {
      printf("time_t overflow!");
      return 0;
    }

  result = utimes (testfile, tv);
  TEST_VERIFY_EXIT (result == 0);

  xfstat (temp_fd, &st);

  /* Check if seconds for atime match */
  TEST_COMPARE (st.st_atime, tv[0].tv_sec);

  /* Check if seconds for mtime match */
  TEST_COMPARE (st.st_mtime, tv[1].tv_sec);

  return 0;
}

static int
do_test (void)
{
  test_utime_helper (&t1[0]);
  test_utime_helper (&t2[0]);
  test_utime_helper (&t3[0]);

  return 0;
}

#include <support/test-driver.c>
