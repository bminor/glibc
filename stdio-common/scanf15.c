/* Copyright (C) 2007-2025 Free Software Foundation, Inc.
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
#include <string.h>
#include <wchar.h>

#if __GLIBC_USE_DEPRECATED_SCANF
# error "This file should not be compiled with deprecated scanf"
#endif

#define FAIL() \
  do {							\
    result = 1;						\
    printf ("test at line %d failed\n", __LINE__);	\
  } while (0)

int
main (void)
{
  float f;
  double d;
  char c[8];
  int result = 0;

  if (sscanf (" 0.25s x", "%e%3c", &f, c) != 2)
    FAIL ();
  else if (f != 0.25 || memcmp (c, "s x", 3) != 0)
    FAIL ();
  if (sscanf (" 1.25s x", "%as%2c", &f, c) != 2)
    FAIL ();
  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 2.25s x", "%las%2c", &d, c) != 2)
    FAIL ();
  else if (d != 2.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 3.25S x", "%4aS%2c", &f, c) != 2)
    FAIL ();
  else if (f != 3.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 4.25[0-9.] x", "%a[0-9.]%2c", &f, c) != 2)
    FAIL ();
  else if (f != 4.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 5.25[0-9.] x", "%la[0-9.]%2c", &d, c) != 2)
    FAIL ();
  else if (d != 5.25 || memcmp (c, " x", 2) != 0)
    FAIL ();

  const char *tmpdir = getenv ("TMPDIR");
  if (tmpdir == NULL || tmpdir[0] == '\0')
    tmpdir = "/tmp";

  char fname[strlen (tmpdir) + sizeof "/tst-scanf15.XXXXXX"];
  sprintf (fname, "%s/tst-scanf15.XXXXXX", tmpdir);

  /* Create a temporary file.   */
  int fd = mkstemp (fname);
  if (fd == -1)
    FAIL ();

  FILE *fp = fdopen (fd, "w+");
  if (fp == NULL)
    FAIL ();
  else
    {
      if (fputs (" 1.25s x", fp) == EOF)
	FAIL ();
      if (fseek (fp, 0, SEEK_SET) != 0)
	FAIL ();
      if (fscanf (fp, "%as%2c", &f, c) != 2)
	FAIL ();
      else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	FAIL ();

      if (freopen (fname, "r", stdin) == NULL)
	FAIL ();
      else
	{
	  if (scanf ("%as%2c", &f, c) != 2)
	    FAIL ();
	  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	    FAIL ();
	}

      fclose (fp);
    }

  remove (fname);

  return result;
}
