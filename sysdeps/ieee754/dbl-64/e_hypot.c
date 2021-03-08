/* Euclidean distance function.  Double/Binary64 version.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

/* The implementation uses a correction based on 'An Improved Algorithm for
   hypot(a,b)' by Carlos F. Borges [1] usingthe MyHypot3 with the following
   changes:

   - Handle qNaN and sNaN.
   - Tune the 'widely varying operands' to avoid spurious underflow
     due the multiplication and fix the return value for upwards
     rounding mode.
   - Handle required underflow exception for subnormal results.

   The expected ULP is ~0.792.

   [1] https://arxiv.org/pdf/1904.09481.pdf  */

#include <math.h>
#include <math_private.h>
#include <math-underflow.h>
#include <math-narrow-eval.h>
#include <libm-alias-finite.h>
#include "math_config.h"

#define SCALE     0x1p-600
#define LARGE_VAL 0x1p+511
#define TINY_VAL  0x1p-459
#define EPS       0x1p-54

/* Hypot kernel. The inputs must be adjusted so that ax >= ay >= 0
   and squaring ax, ay and (ax - ay) does not overflow or underflow.  */
static inline double
kernel (double ax, double ay)
{
  double t1, t2;
  double h = sqrt (ax * ax + ay * ay);
  if (h <= 2.0 * ay)
    {
      double delta = h - ay;
      t1 = ax * (2.0 * delta - ax);
      t2 = (delta - 2.0 * (ax - ay)) * delta;
    }
  else
    {
      double delta = h - ax;
      t1 = 2.0 * delta * (ax - 2.0 * ay);
      t2 = (4.0 * delta - ay) * ay + delta * delta;
    }

  h -= (t1 + t2) / (2.0 * h);
  return h;
}

double
__ieee754_hypot (double x, double y)
{
  if (!isfinite(x) || !isfinite(y))
    {
      if ((isinf (x) || isinf (y))
	  && !issignaling_inline (x) && !issignaling_inline (y))
	return INFINITY;
      return x + y;
    }

  x = fabs (x);
  y = fabs (y);

  double ax = x < y ? y : x;
  double ay = x < y ? x : y;

  /* If ax is huge, scale both inputs down.  */
  if (__glibc_unlikely (ax > LARGE_VAL))
    {
      if (__glibc_unlikely (ay <= ax * EPS))
	return math_narrow_eval (ax + ay);

      return math_narrow_eval (kernel (ax * SCALE, ay * SCALE) / SCALE);
    }

  /* If ay is tiny, scale both inputs up.  */
  if (__glibc_unlikely (ay < TINY_VAL))
    {
      if (__glibc_unlikely (ax >= ay / EPS))
	return math_narrow_eval (ax + ay);

      ax = math_narrow_eval (kernel (ax / SCALE, ay / SCALE) * SCALE);
      math_check_force_underflow_nonneg (ax);
      return ax;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (__glibc_unlikely (ay <= ax * EPS))
    return ax + ay;

  return kernel (ax, ay);
}
#ifndef __ieee754_hypot
libm_alias_finite (__ieee754_hypot, __hypot)
#endif
