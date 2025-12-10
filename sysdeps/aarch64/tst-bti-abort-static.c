/* Test to check that static binary without PT_GNU_PROPERTY can
   still be enforced to have BTI marking (and abort since it
   doesn't have any).
   Copyright (C) 2025 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

static int
do_test (void)
{
  return 0;
}

#include <support/test-driver.c>
