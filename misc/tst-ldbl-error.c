/* Test for the long double conversions in *err* functions.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
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
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <string.h>

#include <support/capture_subprocess.h>
#include <support/check.h>

struct tests
{
  void *callback;
  const char *expected;
};

va_list args;

static void
callback_err (void *closure)
{
  errno = 0;
  err (0, "%Lf", (long double) -1);
}

static void
callback_errx (void *closure)
{
  errno = 0;
  errx (0, "%Lf", (long double) -1);
}

static void
callback_verr (void *closure)
{
  errno = 0;
  verr (0, "%Lf", args);
}

static void
callback_verrx (void *closure)
{
  errno = 0;
  verrx (0, "%Lf", args);
}

static void
callback_error (void *closure)
{
  errno = 0;
  error (0, 0, "%Lf", (long double) -1);
}

static void
callback_error_at_line (void *closure)
{
  errno = 0;
  error_at_line (0, 0, "", 0, "%Lf", (long double) -1);
}

static void
do_one_test (void *callback, const char *expected, ...)
{
  struct support_capture_subprocess result;

  va_start (args, expected);

  /* Call 'callback', which fills in the output and error buffers.  */
  result = support_capture_subprocess (callback, NULL);

  /* The functions err, errx, verr, and verrx print just the program
     name followed by a colon, whereas error and error_at_line print the
     whole path to the program.  Since the whole path depends on the
     working directory used to build and test glibc, remove it from the
     comparison against the expected result.  */
  const char *needle = "tst-ldbl-error:";
  char *message;
  message = strstr (result.err.buffer, needle);

  /* Verify that the output message is as expected.  */
  TEST_COMPARE_STRING (message, expected);

  va_end (args);
}

static int
do_test (void)
{
  struct tests tests[] = {
    { &callback_err, "tst-ldbl-error: -1.000000: Success\n" },
    { &callback_errx, "tst-ldbl-error: -1.000000\n" },
    { &callback_verr, "tst-ldbl-error: -1.000000: Success\n" },
    { &callback_verrx, "tst-ldbl-error: -1.000000\n" },
    { &callback_error, "tst-ldbl-error: -1.000000\n" },
    { &callback_error_at_line, "tst-ldbl-error::0: -1.000000\n" }
  };

  for (int i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
    {
      do_one_test (tests[i].callback, tests[i].expected, (long double) -1);
    }

  return 0;
}

#include <support/test-driver.c>
