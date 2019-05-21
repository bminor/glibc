/* Test ASCII gconv module followed by cache initialization.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xstdio.h>
#include <wchar.h>

static int
do_test (void)
{
  /* The test-in-container framework sets these environment variables.
     The presence of GCONV_PATH invalidates this test.  */
  unsetenv ("GCONV_PATH");
  unsetenv ("LOCPATH");

  /* Create the gconv module cache.  iconvconfig is in /sbin, which is
     not on PATH.  */
  {
    char *iconvconfig = xasprintf ("%s/iconvconfig", support_sbindir_prefix);
    TEST_COMPARE (system (iconvconfig), 0);
  }

  /* Use built-in ASCII gconv module, without triggering cache
     initialization.  */
  FILE *fp1 = xfopen ("/dev/zero", "r");
  TEST_COMPARE (fwide (fp1, 1), 1);

  /* Use non-ASCII gconv module and trigger gconv cache
     initialization.  */
  FILE *fp2 = xfopen ("/dev/zero", "r,ccs=UTF-8");
  TEST_COMPARE (fwide (fp2, 0), 1);

  xfclose (fp1);
  xfclose (fp2);

  return 0;
}

#include <support/test-driver.c>
