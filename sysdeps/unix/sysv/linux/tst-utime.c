/* Test for utime.
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
#include <sys/types.h>
#include <utime.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/temp_file.h>

static int temp_fd = -1;
char *testfile;

/* struct utimbuf with Y2038 threshold minus 2 and 1 seconds.  */
const static struct utimbuf t1 = { 0x7FFFFFFE, 0x7FFFFFFF };

/* struct utimbuf with Y2038 threshold plus 1 and 2 seconds.  */
const static struct utimbuf t2 = { 0x80000001ULL, 0x80000002ULL };

/* struct utimbuf around Y2038 threshold.  */
const static struct utimbuf t3 = { 0x7FFFFFFE, 0x80000002ULL };

#define PREPARE do_prepare
static void
do_prepare (int argc, char *argv[])
{
  temp_fd = create_temp_file ("utime", &testfile);
  TEST_VERIFY_EXIT (temp_fd > 0);
}

static int
test_utime_helper (const struct utimbuf *ut)
{
  struct stat64 st;
  int result;
  time_t t;

  /* Check if we run on port with 32 bit time_t size */
  if (__builtin_add_overflow (ut->actime, 0, &t))
    {
      printf("time_t overflow!");
      return 0;
    }

  result = utime (testfile, ut);
  TEST_VERIFY_EXIT (result == 0);

  xfstat (temp_fd, &st);

  /* Check if seconds for actime match */
  TEST_COMPARE (st.st_atime, ut->actime);

  /* Check if seconds for modtime match */
  TEST_COMPARE (st.st_mtime, ut->modtime);

  return 0;
}

static int
do_test (void)
{
  test_utime_helper (&t1);
  test_utime_helper (&t2);
  test_utime_helper (&t3);

  return 0;
}

#include <support/test-driver.c>
