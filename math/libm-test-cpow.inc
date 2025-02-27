/* Test cpow.
   Copyright (C) 1997-2025 Free Software Foundation, Inc.
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

#include "libm-test-driver.c"

static const struct test_cc_c_data cpow_test_data[] =
  {
    TEST_cc_c (cpow, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value),

    AUTO_TESTS_cc_c (cpow),
  };

static void
cpow_test (void)
{
  ALL_RM_TEST (cpow, 0, cpow_test_data, RUN_TEST_LOOP_cc_c, END_COMPLEX);
}

static void
do_test (void)
{
  cpow_test ();
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
