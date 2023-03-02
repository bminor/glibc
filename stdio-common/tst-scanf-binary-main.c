/* Test scanf functions with C2X binary integers.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <wchar.h>

#include <support/check.h>
#include <support/xstdio.h>

#define CONCAT_(X, Y, Z) X ## Y ## Z
#define CONCAT(X, Y, Z) CONCAT_ (X, Y, Z)
#define FNX(FN1, FN2) CONCAT (FN1, FNW, FN2)
#ifndef STDX
# define STDX ""
#endif

#define INFILE OBJPFX "/tst-" STDX "scanf-binary-" STD "-in"

static int
wrap_vfscanf (FILE *fp, const CHAR *format, ...)
{
  va_list ap;
  va_start (ap, format);
  int ret = FNX (vf, scanf) (fp, format, ap);
  va_end (ap);
  return ret;
}

static int
wrap_vscanf (const CHAR *format, ...)
{
  va_list ap;
  va_start (ap, format);
  int ret = FNX (v, scanf) (format, ap);
  va_end (ap);
  return ret;
}

static int
wrap_vsscanf (const CHAR *s, const CHAR *format, ...)
{
  va_list ap;
  va_start (ap, format);
  int ret = FNX (vs, scanf) (s, format, ap);
  va_end (ap);
  return ret;
}

static void
one_check (const CHAR *s, int expected, char expected_c)
{
  int ret;
  FILE *fp;
  int ret_i;
  long int ret_l;
  long long int ret_ll;
  char ret_c;
  fp = xfopen (INFILE, "w");
  ret = FNX (fput, s) (s, fp);
  TEST_VERIFY_EXIT (0 <= ret);
  xfclose (fp);

  if (!TEST_C2X)
    {
      expected = 0;
      expected_c = s[0] == L_('-') ? s[2] : s[1];
    }

  ret = FNX (s, scanf) (s, L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = FNX (f, scanf) (fp, L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = FNX (, scanf) (L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);
  ret = wrap_vsscanf (s, L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = wrap_vfscanf (fp, L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = wrap_vscanf (L_("%i %c"), &ret_i, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_i, expected);
  TEST_COMPARE (ret_c, expected_c);

  ret = FNX (s, scanf) (s, L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = FNX (f, scanf) (fp, L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = FNX (, scanf) (L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);
  ret = wrap_vsscanf (s, L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = wrap_vfscanf (fp, L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = wrap_vscanf (L_("%li %c"), &ret_l, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_l, expected);
  TEST_COMPARE (ret_c, expected_c);

  ret = FNX (s, scanf) (s, L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = FNX (f, scanf) (fp, L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = FNX (, scanf) (L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
  ret = wrap_vsscanf (s, L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
  fp = xfopen (INFILE, "r");
  ret = wrap_vfscanf (fp, L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
  xfclose (fp);
  fp = xfreopen (INFILE, "r", stdin);
  ret = wrap_vscanf (L_("%lli %c"), &ret_ll, &ret_c);
  TEST_COMPARE (ret, 2);
  TEST_COMPARE (ret_ll, expected);
  TEST_COMPARE (ret_c, expected_c);
}

static int
do_test (void)
{
  one_check (L_("0b101 x"), 5, 'x');
  one_check (L_("0B101 x"), 5, 'x');
  one_check (L_("-0b11111 y"), -31, 'y');
  one_check (L_("-0B11111 y"), -31, 'y');
  return 0;
}

#include <support/test-driver.c>
