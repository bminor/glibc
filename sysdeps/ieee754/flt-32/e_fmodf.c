/* Floating-point remainder function.
   Copyright (C) 2023 Free Software Foundation, Inc.
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
#include <libm-alias-float.h>
#include <math-svid-compat.h>
#include <math.h>
#include "math_config.h"

/* With x = mx * 2^ex and y = my * 2^ey (mx, my, ex, ey being integers), the
   simplest implementation is:

   mx * 2^ex == 2 * mx * 2^(ex - 1)

   or

   while (ex > ey)
     {
       mx *= 2;
       --ex;
       mx %= my;
     }

   With the mathematical equivalence of:

   r == x % y == (x % (N * y)) % y

   And with mx/my being mantissa of double floating point number (which uses
   less bits than the storage type), on each step the argument reduction can
   be improved by 8 (which is the size of uint32_t minus MANTISSA_WIDTH plus
   the signal bit):

   mx * 2^ex == 2^8 * mx * 2^(ex - 8)

   or

   while (ex > ey)
     {
       mx << 8;
       ex -= 8;
       mx %= my;
     }  */

float
__fmodf (float x, float y)
{
  uint32_t hx = asuint (x);
  uint32_t hy = asuint (y);

  uint32_t sx = hx & SIGN_MASK;
  /* Get |x| and |y|.  */
  hx ^= sx;
  hy &= ~SIGN_MASK;

  /* Special cases:
     - If x or y is a Nan, NaN is returned.
     - If x is an inifinity, a NaN is returned.
     - If y is zero, Nan is returned.
     - If x is +0/-0, and y is not zero, +0/-0 is returned.  */
  if (__glibc_unlikely (hy == 0
			|| hx >= EXPONENT_MASK || hy > EXPONENT_MASK))
    {
      if (is_nan (hx) || is_nan (hy))
	return (x * y) / (x * y);
      return __math_edomf ((x * y) / (x * y));
    }

  if (__glibc_unlikely (hx <= hy))
    {
      if (hx < hy)
	return x;
      return asfloat (sx);
    }

  int ex = hx >> MANTISSA_WIDTH;
  int ey = hy >> MANTISSA_WIDTH;

  /* Common case where exponents are close: ey >= -103 and |x/y| < 2^8,  */
  if (__glibc_likely (ey > MANTISSA_WIDTH && ex - ey <= EXPONENT_WIDTH))
    {
      uint64_t mx = (hx & MANTISSA_MASK) | (MANTISSA_MASK + 1);
      uint64_t my = (hy & MANTISSA_MASK) | (MANTISSA_MASK + 1);

      uint32_t d = (ex == ey) ? (mx - my) : (mx << (ex - ey)) % my;
      return make_float (d, ey - 1, sx);
    }

  /* Special case, both x and y are subnormal.  */
  if (__glibc_unlikely (ex == 0 && ey == 0))
    return asfloat (sx | hx % hy);

  /* Convert |x| and |y| to 'mx + 2^ex' and 'my + 2^ey'.  Assume that hx is
     not subnormal by conditions above.  */
  uint32_t mx = get_mantissa (hx) | (MANTISSA_MASK + 1);
  ex--;

  uint32_t my = get_mantissa (hy) | (MANTISSA_MASK + 1);
  int lead_zeros_my = EXPONENT_WIDTH;
  if (__glibc_likely (ey > 0))
    ey--;
  else
    {
      my = hy;
      lead_zeros_my = __builtin_clz (my);
    }

  int tail_zeros_my = __builtin_ctz (my);
  int sides_zeroes = lead_zeros_my + tail_zeros_my;
  int exp_diff = ex - ey;

  int right_shift = exp_diff < tail_zeros_my ? exp_diff : tail_zeros_my;
  my >>= right_shift;
  exp_diff -= right_shift;
  ey += right_shift;

  int left_shift = exp_diff < EXPONENT_WIDTH ? exp_diff : EXPONENT_WIDTH;
  mx <<= left_shift;
  exp_diff -= left_shift;

  mx %= my;

  if (__glibc_unlikely (mx == 0))
    return asfloat (sx);

  if (exp_diff == 0)
    return make_float (mx, ey, sx);

  /* Assume modulo/divide operation is slow, so use multiplication with invert
     values.  */
  uint32_t inv_hy = UINT32_MAX / my;
  while (exp_diff > sides_zeroes) {
    exp_diff -= sides_zeroes;
    uint32_t hd = (mx * inv_hy) >> (BIT_WIDTH - sides_zeroes);
    mx <<= sides_zeroes;
    mx -= hd * my;
    while (__glibc_unlikely (mx > my))
      mx -= my;
  }
  uint32_t hd = (mx * inv_hy) >> (BIT_WIDTH - exp_diff);
  mx <<= exp_diff;
  mx -= hd * my;
  while (__glibc_unlikely (mx > my))
    mx -= my;

  return make_float (mx, ey, sx);
}
strong_alias (__fmodf, __ieee754_fmodf)
#if LIBM_SVID_COMPAT
versioned_symbol (libm, __fmodf, fmodf, GLIBC_2_38);
libm_alias_float_other (__fmod, fmod)
#else
libm_alias_float (__fmod, fmod)
#endif
libm_alias_finite (__ieee754_fmodf, __fmodf)
