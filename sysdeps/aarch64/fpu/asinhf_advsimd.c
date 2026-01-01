/* Single-precision vector (Advanced SIMD) asinh function

   Copyright (C) 2024-2026 Free Software Foundation, Inc.
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
#include "v_log1pf_inline.h"

const static struct data
{
  struct v_log1pf_data log1pf_consts;
  float32x4_t one;
  uint32x4_t square_lim;
  float32x4_t pinf, nan;
} data = {
  .one = V4 (1),
  .log1pf_consts = V_LOG1PF_CONSTANTS_TABLE,
  .square_lim = V4 (0x5f800000), /* asuint(sqrt(FLT_MAX)).  */
  .pinf = V4 (INFINITY),
  .nan = V4 (NAN),
};

static inline float32x4_t VPCS_ATTR
inline_asinhf (float32x4_t ax, uint32x4_t sign, const struct data *d)
{
  /* Consider the identity asinh(x) = log(x + sqrt(x^2 + 1)).
    Then, for x>0, asinh(x) = log1p(x + x^2 / (1 + sqrt(x^2 + 1))).  */
  float32x4_t t
      = vaddq_f32 (v_f32 (1.0f), vsqrtq_f32 (vfmaq_f32 (d->one, ax, ax)));
  float32x4_t y = vaddq_f32 (ax, vdivq_f32 (vmulq_f32 (ax, ax), t));

  return vreinterpretq_f32_u32 (veorq_u32 (
      sign, vreinterpretq_u32_f32 (log1pf_inline (y, &d->log1pf_consts))));
}

static float32x4_t VPCS_ATTR NOINLINE
special_case (float32x4_t ax, uint32x4_t sign, uint32x4_t special,
	      const struct data *d)
{
  /* To avoid overflow in x^2 (so the x < sqrt(FLT_MAX) constraint), we
    reduce the input of asinh to a narrower interval by relying on the
    identity: 2asinh(t) = +-acosh(2t^2 + 1)
    If we set t=sqrt((x-1)/2), then
    2asinh(sqrt((x-1)/2)) = acosh(x).
    Found that, for a high input x, asinh(x) very closely approximates
    acosh(x), so implemented it with this function instead.  */
  float32x4_t r = vsubq_f32 (ax, d->one);
  r = vmulq_f32 (r, v_f32 (0.5f));
  r = vbslq_f32 (special, vsqrtq_f32 (r), ax);

  float32x4_t y = inline_asinhf (r, sign, d);

  y = vbslq_f32 (special, vmulq_f32 (y, v_f32 (2.0f)), y);

  /* Check whether x is inf or nan.  */
  uint32x4_t ret_inf = vceqq_f32 (ax, d->pinf);
  uint32x4_t ret_nan = vmvnq_u32 (vcleq_f32 (ax, d->pinf));
  y = vbslq_f32 (ret_inf, d->pinf, y);
  y = vbslq_f32 (ret_nan, d->nan, y);
  /* Put sign back in for minf, as it doesn't happen in log1pf_inline call.  */
  y = vbslq_f32 (
      ret_inf,
      vreinterpretq_f32_u32 (veorq_u32 (vreinterpretq_u32_f32 (y), sign)), y);
  return y;
}

/* Single-precision implementation of vector asinh(x), using vector log1p.
   Worst-case error is 2.59 ULP:
   _ZGVnN4v_asinhf(0x1.d86124p-3) got 0x1.d449bep-3
				 want 0x1.d449c4p-3.  */
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (asinh) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);
  float32x4_t ax = vabsq_f32 (x);
  uint32x4_t iax = vreinterpretq_u32_f32 (ax);
  uint32x4_t sign = veorq_u32 (vreinterpretq_u32_f32 (x), iax);

  /* Inputs greater than or equal to square_lim will cause the output to
    overflow. This is because there is a square operation in the log1pf_inline
    call. Also captures inf and nan. Does not capture negative numbers as we
    separate the sign bit from the rest of the input.  */
  uint32x4_t special = vcgeq_u32 (iax, d->square_lim);

  if (__glibc_unlikely (v_any_u32 (special)))
    return special_case (ax, sign, special, d);
  return inline_asinhf (ax, sign, d);
}
libmvec_hidden_def (V_NAME_F1 (asinh))
HALF_WIDTH_ALIAS_F1 (asinh)
