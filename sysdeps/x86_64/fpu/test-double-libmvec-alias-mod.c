/* Part of test to build shared library to ensure link against
   *_finite aliases from libmvec.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdlib.h>
#include <math-tests-arch.h>

#define N 3000
double log_arg[N];
double log_res[N];
double exp_arg[N];
double exp_res[N];
double pow_res[N];

static void init(void)
{
  int i;

  for (i = 0; i < N; i += 1)
    {
      log_arg[i] = 1.0;
      exp_arg[i] = 0.0;
    }
}

static void check(void)
{
  if (log_res[0] != 0.0) abort ();
  if (exp_res[0] != 1.0) abort ();
  if (pow_res[0] != 1.0) abort ();
}

void test_finite_alias (void)
{
  int i;

  CHECK_ARCH_EXT;

  init();

#pragma omp simd
  for (i = 0; i < N; i += 1)
    {
      log_res[i] = log (log_arg[i]);
      exp_res[i] = exp (exp_arg[i]);
      pow_res[i] = pow (log_arg[i], log_arg[i]);
    }

  check();
}
