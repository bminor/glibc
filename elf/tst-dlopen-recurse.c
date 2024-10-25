/* Test that recursive dlopen runs constructors before return (bug 31986).
   Copyright (C) 2024 Free Software Foundation, Inc.
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
#include <support/check.h>
#include <support/xdlfcn.h>

static int
do_test (void)
{
  void *handle = xdlopen ("tst-dlopen-recursemod1.so", RTLD_NOW);
  int *status = dlsym (handle, "recursemod1_status");
  printf ("info: recursemod1_status == %d (from main)\n", *status);
  TEST_COMPARE (*status, 2);
  xdlclose (handle);
  return 0;
}

#include <support/test-driver.c>
