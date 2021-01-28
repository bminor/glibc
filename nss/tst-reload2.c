/* Test that reloading is disabled after a chroot.
   Copyright (C) 2020-2021 Free Software Foundation, Inc.
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

#include <nss.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <support/support.h>
#include <support/check.h>
#include <support/xunistd.h>

#include "nss_test.h"

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

static struct passwd pwd_table1[] =
  {
   PWD_N (1234, "test1"),
   PWD_N (4321, "test2"),
   PWD_LAST ()
  };

static const char *group_4[] = {
  "alpha", "beta", "gamma", "fred", NULL
};

static struct group group_table_data[] =
  {
   GRP (4),
   GRP_LAST ()
  };

void
_nss_test1_init_hook (test_tables *t)
{
  t->pwd_table = pwd_table1;
  t->grp_table = group_table_data;
}

static struct passwd pwd_table2[] =
  {
   PWD_N (5, "test1"),
   PWD_N (2468, "test2"),
   PWD_LAST ()
  };

void
_nss_test2_init_hook (test_tables *t)
{
  t->pwd_table = pwd_table2;
}

static int
do_test (void)
{
  struct passwd *pw;
  struct group *gr;
  char buf1[PATH_MAX];
  char buf2[PATH_MAX];

  sprintf (buf1, "/subdir%s", support_slibdir_prefix);
  xmkdirp (buf1, 0777);

  /* Copy this DSO into the chroot so it *could* be loaded.  */
  sprintf (buf1, "%s/libnss_files.so.2", support_slibdir_prefix);
  sprintf (buf2, "/subdir%s/libnss_files.so.2", support_slibdir_prefix);
  support_copy_file (buf1, buf2);

  /* Check we're using the "outer" nsswitch.conf.  */

  /* This uses the test1 DSO.  */
  pw = getpwnam ("test1");
  TEST_VERIFY (pw != NULL);
  if (pw)
    TEST_COMPARE (pw->pw_uid, 1234);

  /* This just loads the test2 DSO.  */
  gr = getgrnam ("name4");

  /* Change the root dir.  */

  TEST_VERIFY (chroot ("/subdir") == 0);
  chdir ("/");

  /* Check we're NOT using the "inner" nsswitch.conf.  */

  /* Both DSOs are loaded, which is used?  */
  pw = getpwnam ("test2");
  TEST_VERIFY (pw != NULL);
  if (pw)
    TEST_VERIFY (pw->pw_uid != 2468);

  /* The "files" DSO should not be loaded.  */
  gr = getgrnam ("test3");
  TEST_VERIFY (gr == NULL);

  /* We should still be using the old configuration.  */
  pw = getpwnam ("test1");
  TEST_VERIFY (pw != NULL);
  if (pw)
    TEST_COMPARE (pw->pw_uid, 1234);

  return 0;
}

#include <support/test-driver.c>
