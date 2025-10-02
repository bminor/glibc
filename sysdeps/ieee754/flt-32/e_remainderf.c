/* Remainder function, float version.
   Copyright (C) 2008-2025 Free Software Foundation, Inc.
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

#include <math.h>
#include <libm-alias-finite.h>
#include "math_config.h"

float
__ieee754_remainderf(float x, float p)
{
  uint32_t hx = asuint (x);
  uint32_t hp = asuint (p);
  uint32_t sx = hx >> 31;

  hp &= ~SIGN_MASK;
  hx &= ~SIGN_MASK;

  /* |y| < DBL/MAX / 2 ? */
  p = fabsf (p);
  if (__glibc_likely (hp < 0x7f000000))
    {
      /* |x| not finite, |y| equal 0 is handled by fmod.  */
      if (__glibc_unlikely (hx >= EXPONENT_MASK))
	return (x * p) / (x * p);

      x = fabs (__ieee754_fmodf (x, p + p)); /* now x < 2p */
      if (x + x > p)
	{
	  x -= p;
	  if (x + x >= p)
	    x -= p;
	  /* Make sure x is not -0. This can occur only when x = p
	     and rounding direction is towards negative infinity. */
	  else if (x == 0.0)
	    x = 0.0;
	}
    }
  else
    {
      /* |x| not finite or |y| is NaN or 0 */
      if ((hx >= EXPONENT_MASK || (hp - 1) >= EXPONENT_MASK))
	return (x * p) / (x * p);

      x = fabsf (x);
      float p_half = 0.5f * p;
      if (x > p_half)
	{
	  x -= p;
	  if (x >= p_half)
	    x -= p;
	  else if (x == 0.0)
	    x = 0.0;
	}
    }

  return sx ? -x : x;
}
libm_alias_finite (__ieee754_remainderf, __remainderf)
