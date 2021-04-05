/* Euclidean distance function.  Float/Binary32 version.
   Copyright (C) 2012-2021 Free Software Foundation, Inc.
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

#include <libm-alias-finite.h>
#include <math.h>
#include <math-narrow-eval.h>
#include <math_private.h>

float
__ieee754_hypotf (float x, float y)
{
  if (!isfinite (x) || !isfinite (y))
    {
      if ((isinf (x) || isinf (y))
	  && !issignaling (x) && !issignaling (y))
	return INFINITY;
      return x + y;
    }

  return math_narrow_eval ((float) sqrt ((double) x * (double) x
					 + (double) y * (double) y));
}
#ifndef __ieee754_hypotf
libm_alias_finite (__ieee754_hypotf, __hypotf)
#endif
