/* Double-precision vector (Advanced SIMD) atanh function

   Copyright (C) 2024-2025 Free Software Foundation, Inc.
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

#define WANT_V_LOG1P_K0_SHORTCUT 0
#include "v_log1p_inline.h"

const static struct data
{
  struct v_log1p_data log1p_consts;
  uint64x2_t one;
  uint64x2_t sign_mask;
} data = { .log1p_consts = V_LOG1P_CONSTANTS_TABLE,
	   .one = V2 (0x3ff0000000000000),
	   .sign_mask = V2 (0x8000000000000000) };

static float64x2_t VPCS_ATTR NOINLINE
special_case (float64x2_t x, float64x2_t halfsign, float64x2_t y,
	      uint64x2_t special, const struct data *d)
{
  y = log1p_inline (y, &d->log1p_consts);
  return v_call_f64 (atanh, vbslq_f64 (d->sign_mask, halfsign, x),
		     vmulq_f64 (halfsign, y), special);
}

/* Approximation for vector double-precision atanh(x) using modified log1p.
   The greatest observed error is 3.31 ULP:
   _ZGVnN2v_atanh(0x1.ffae6288b601p-6) got 0x1.ffd8ff31b5019p-6
				      want 0x1.ffd8ff31b501cp-6.  */
VPCS_ATTR
float64x2_t V_NAME_D1 (atanh) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);

  float64x2_t halfsign = vbslq_f64 (d->sign_mask, x, v_f64 (0.5));
  float64x2_t ax = vabsq_f64 (x);
  uint64x2_t ia = vreinterpretq_u64_f64 (ax);
  uint64x2_t special = vcgeq_u64 (ia, d->one);

#if WANT_SIMD_EXCEPT
  ax = v_zerofy_f64 (ax, special);
#endif

  float64x2_t y;
  y = vaddq_f64 (ax, ax);
  y = vdivq_f64 (y, vsubq_f64 (vreinterpretq_f64_u64 (d->one), ax));

  if (__glibc_unlikely (v_any_u64 (special)))
#if WANT_SIMD_EXCEPT
    return special_case (x, halfsign, y, special, d);
#else
    return special_case (ax, halfsign, y, special, d);
#endif

  y = log1p_inline (y, &d->log1p_consts);
  return vmulq_f64 (y, halfsign);
}
