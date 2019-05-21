/* Test that non-built-in gconv modules do not cause memory leak (bug 24583).
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

#include <locale.h>
#include <mcheck.h>
#include <support/check.h>
#include <support/xstdio.h>

static int
do_test (void)
{
  mtrace ();

  TEST_VERIFY_EXIT (setlocale (LC_ALL, "ja_JP.EUC-JP") != NULL);
  xfclose (xfopen ("/etc/passwd", "r,ccs=UTF-8"));
  xfclose (xfopen ("/etc/passwd", "r"));

  return 0;
}

#include <support/test-driver.c>
