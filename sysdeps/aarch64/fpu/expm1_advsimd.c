/* Double-precision AdvSIMD expm1

   Copyright (C) 2023-2025 Free Software Foundation, Inc.
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

#include "v_math.h"
#include "v_expm1_inline.h"

static const struct data
{
  struct v_expm1_data d;
#if WANT_SIMD_EXCEPT
  uint64x2_t thresh, tiny_bound;
#else
  float64x2_t oflow_bound;
#endif
} data = {
  .d = V_EXPM1_DATA,
#if WANT_SIMD_EXCEPT
  /* asuint64(oflow_bound) - asuint64(0x1p-51), shifted left by 1 for abs
     compare.  */
  .thresh = V2 (0x78c56fa6d34b552),
  /* asuint64(0x1p-51) << 1.  */
  .tiny_bound = V2 (0x3cc0000000000000 << 1),
#else
  /* Value above which expm1(x) should overflow. Absolute value of the
     underflow bound is greater than this, so it catches both cases - there is
     a small window where fallbacks are triggered unnecessarily.  */
  .oflow_bound = V2 (0x1.62b7d369a5aa9p+9),
#endif
};

static float64x2_t VPCS_ATTR NOINLINE
special_case (float64x2_t x, uint64x2_t special, const struct data *d)
{
  return v_call_f64 (expm1, x, expm1_inline (v_zerofy_f64 (x, special), &d->d),
		     special);
}

/* Double-precision vector exp(x) - 1 function.
   The maximum error observed error is 2.05 ULP:
  _ZGVnN2v_expm1(0x1.634902eaff3adp-2) got 0x1.a8b636e2a9388p-2
				      want 0x1.a8b636e2a9386p-2.  */
float64x2_t VPCS_ATTR V_NAME_D1 (expm1) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);

#if WANT_SIMD_EXCEPT
  uint64x2_t ix = vreinterpretq_u64_f64 (x);
  /* If fp exceptions are to be triggered correctly, fall back to scalar for
     |x| < 2^-51, |x| > oflow_bound, Inf & NaN. Add ix to itself for
     shift-left by 1, and compare with thresh which was left-shifted offline -
     this is effectively an absolute compare.  */
  uint64x2_t special
      = vcgeq_u64 (vsubq_u64 (vaddq_u64 (ix, ix), d->tiny_bound), d->thresh);
#else
  /* Large input, NaNs and Infs.  */
  uint64x2_t special = vcageq_f64 (x, d->oflow_bound);
#endif

  if (__glibc_unlikely (v_any_u64 (special)))
    return special_case (x, special, d);

  /* expm1(x) ~= p * t + (t - 1).  */
  return expm1_inline (x, &d->d);
}
