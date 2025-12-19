/* Ccheck if math.h optimizations to call compiler builtin
   does not trigger FE_INVALID on x86 denormal numbers.
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

#include <array_length.h>
#include <fenv.h>
#include <math.h>
#include <math_ldbl.h>
#include <support/check.h>

#define pseudo_inf { .parts = { 0x00000000, 0x00000000, 0x7fff }}
#define pseudo_zero { .parts = { 0x00000000, 0x00000000, 0x0100 }}
#define pseudo_qnan { .parts = { 0x00000001, 0x00000000, 0x7fff }}
#define pseudo_snan { .parts = { 0x00000001, 0x40000000, 0x7fff }}
#define pseudo_unnormal { .parts = { 0x00000001, 0x40000000, 0x0100 }}

static const ieee_long_double_shape_type inputs[] = {
  pseudo_inf,
  pseudo_zero,
  pseudo_qnan,
  pseudo_snan,
  pseudo_unnormal
};

static int
do_test (void)
{
  for (int i = 0; i < array_length (inputs); i++)
    {
      TEST_COMPARE (feclearexcept (FE_INVALID), 0);
      TEST_COMPARE (fpclassify (inputs[i].value), FP_NAN);
      TEST_COMPARE (fetestexcept (FE_INVALID), 0);
    }

  return 0;
}

#include <support/test-driver.c>
