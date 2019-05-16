/* Test wide output conversion for warn with TSCII.
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

#include <err.h>
#include <locale.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/temp_file.h>
#include <support/xstdio.h>
#include <support/xunistd.h>
#include <wchar.h>

static int
do_test (void)
{
  if (setlocale (LC_ALL, "en_US.TSCII") == NULL)
    FAIL_EXIT1 ("setlocale");

  char *path;
  xclose (create_temp_file ("tst-warn-wide-tscii-", &path));
  FILE *fp = xfopen (path, "w,ccs=UTF-8");
  TEST_COMPARE (fwide (fp, 0), 1);

  FILE *old_stderr = stderr;
  stderr = fp;

  /* Special character that expands to four wide characters.  */
  warnx ("[[\x8C]]");

  stderr = old_stderr;

  /* Verify that the expected data was written.  */
  xfclose (fp);
  fp = xfopen (path, "rb");
  char line[1024];
  TEST_VERIFY (fgets (line, sizeof (line), fp) != NULL);
  TEST_COMPARE_STRING (line, "tst-warn-wide-tscii: [["
                       "\xe0\xae\x95\xe0\xaf\x8d\xe0\xae\xb7\xe0\xaf\x8d]]\n");
  TEST_COMPARE (fgetc (fp), EOF);
  TEST_VERIFY (feof (fp));
  TEST_VERIFY (!ferror (fp));

  free (path);

  return 0;
}

#include <support/test-driver.c>
