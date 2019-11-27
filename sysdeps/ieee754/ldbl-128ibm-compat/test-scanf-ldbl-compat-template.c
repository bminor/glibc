/* Test for the long double variants of *scanf functions.
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
   <https://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <support/check.h>

#define CLEAR								\
  va_start (args, format);						\
  ld = va_arg (args, long double *);					\
  *ld = 0;								\
  va_end (args);

#define CLEAR_VALUE value = 0;

#define CHECK								\
  va_start (args, format);						\
  ld = va_arg (args, long double *);					\
  va_end (args);							\
  if (*ld == -1.0L)							\
    printf ("OK");							\
  else									\
    printf ("ERROR (%.60Lf)", *ld);					\
  printf ("\n");

#define CHECK_VALUE							\
  if (value == -1.0L)							\
    printf ("OK");							\
  else									\
    printf ("ERROR (%.60Lf)", value);					\
  printf ("\n");

static void
do_test_call (FILE *stream, CHAR *string, const CHAR *format, ...)
{
  long double value;
  long double *ld;
  va_list args;

  CLEAR_VALUE
  printf ("fscanf: ");
  FSCANF (stream, format, &value);
  CHECK_VALUE

  CLEAR_VALUE
  printf ("scanf: ");
  SCANF (format, &value);
  CHECK_VALUE

  CLEAR_VALUE
  printf ("sscanf: ");
  SSCANF (string, format, &value);
  CHECK_VALUE

  CLEAR
  printf ("vfscanf: ");
  va_start (args, format);
  VFSCANF (stream, format, args);
  va_end (args);
  CHECK

  CLEAR
  printf ("vscanf: ");
  va_start (args, format);
  VSCANF (format, args);
  va_end (args);
  CHECK

  CLEAR
  printf ("vsscanf: ");
  va_start (args, format);
  VSSCANF (string, format, args);
  va_end (args);
  CHECK
}

static int
do_test (void)
{
  CHAR string[256];
  long double ld;

  /* Scan in decimal notation.  */
  STRCPY (string,
	  L ("-1.0\n")
	  L ("-1.0\n") );
  do_test_call (stdin, string, L("%Lf"), &ld);

  /* Scan in hexadecimal notation.  */
  STRCPY (string,
	  L ("-0x1.0p+0\n")
	  L ("-0x1.0p+0\n") );
  do_test_call (stdin, string, L("%La"), &ld);

  return 0;
}

#include <support/test-driver.c>
