/* Test getdelim conforming to POSIX specifications.

   Note: Most getdelim use cases are covered by stdio-common/tst-getline.

   Copyright (C) 2023-2025 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

static int
do_test (void)
{
  /* Check that getdelim sets error indicator on error (BZ #29917) */
  clearerr (stdin);
  TEST_VERIFY (getdelim (0, 0, '\n', stdin) == -1);
  TEST_VERIFY (ferror (stdin) != 0);
  TEST_VERIFY (errno == EINVAL);

  /* Test getdelim with NUL as delimiter */
  verbose_printf ("Testing NUL delimiter\n");
  char *lineptr = NULL;
  size_t linelen = 0;
  char membuf[] = "abc\0d\nef\0";
  FILE *memstream = fmemopen (membuf, sizeof (membuf), "r");
  TEST_VERIFY_EXIT (memstream != NULL);
  TEST_VERIFY (getdelim (&lineptr, &linelen, '\0', memstream) != -1);
  TEST_COMPARE_BLOB (lineptr, 4, "abc\0", 4);
  TEST_VERIFY (getdelim (&lineptr, &linelen, '\0', memstream) != -1);
  TEST_COMPARE_BLOB (lineptr, 5, "d\nef\0", 5);
  fclose (memstream);
  free (lineptr);

  return 0;
}

#include <support/test-driver.c>
