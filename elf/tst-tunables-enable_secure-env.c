/* Check enable_secure tunable handles removed ENV variables without
   assertions.
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

#include <support/capture_subprocess.h>
#include <support/check.h>

static int
do_test (int argc, char *argv[])
{
  /* Ensure that no assertions are hit when a dynamically linked application
     runs.  This test requires that GLIBC_TUNABLES=glibc.rtld.enable_secure=1
     is set. */
  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
