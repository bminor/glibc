/* Single-precision vector (AdvSIMD) exp10 function.

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

#define ScaleBound 192.0f

static const struct data
{
  float32x4_t c0, c1, c3;
  float log10_2_high, log10_2_low, c2, c4;
  float32x4_t inv_log10_2, special_bound;
  uint32x4_t exponent_bias, special_offset, special_bias;
  float32x4_t scale_thresh;
} data = {
  /* Coefficients generated using Remez algorithm with minimisation of relative
     error.
     rel error: 0x1.89dafa3p-24
     abs error: 0x1.167d55p-23 in [-log10(2)/2, log10(2)/2]
     maxerr: 1.85943 +0.5 ulp.  */
  .c0 = V4 (0x1.26bb16p+1f),
  .c1 = V4 (0x1.5350d2p+1f),
  .c2 = 0x1.04744ap+1f,
  .c3 = V4 (0x1.2d8176p+0f),
  .c4 = 0x1.12b41ap-1f,
  .inv_log10_2 = V4 (0x1.a934fp+1),
  .log10_2_high = 0x1.344136p-2,
  .log10_2_low = 0x1.ec10cp-27,
  /* rint (log2 (2^127 / (1 + sqrt (2)))).  */
  .special_bound = V4 (126.0f),
  .exponent_bias = V4 (0x3f800000),
  .special_offset = V4 (0x82000000),
  .special_bias = V4 (0x7f000000),
  .scale_thresh = V4 (ScaleBound)
};

# define SpecialBound 126.0f

static float32x4_t VPCS_ATTR NOINLINE
special_case (float32x4_t poly, float32x4_t n, uint32x4_t e, uint32x4_t cmp1,
	      float32x4_t scale, const struct data *d)
{
  /* 2^n may overflow, break it up into s1*s2.  */
  uint32x4_t b = vandq_u32 (vclezq_f32 (n), d->special_offset);
  float32x4_t s1 = vreinterpretq_f32_u32 (vaddq_u32 (b, d->special_bias));
  float32x4_t s2 = vreinterpretq_f32_u32 (vsubq_u32 (e, b));
  uint32x4_t cmp2 = vcagtq_f32 (n, d->scale_thresh);
  float32x4_t r2 = vmulq_f32 (s1, s1);
  float32x4_t r1 = vmulq_f32 (vfmaq_f32 (s2, poly, s2), s1);
  /* Similar to r1 but avoids double rounding in the subnormal range.  */
  float32x4_t r0 = vfmaq_f32 (scale, poly, scale);
  float32x4_t r = vbslq_f32 (cmp1, r1, r0);
  return vbslq_f32 (cmp2, r2, r);
}

/* Fast vector implementation of single-precision exp10.
   Algorithm is accurate to 2.36 ULP.
   _ZGVnN4v_exp10f(0x1.be2b36p+1) got 0x1.7e79c4p+11
				 want 0x1.7e79cp+11.  */
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (exp10) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);

  /* exp10(x) = 2^n * 10^r = 2^n * (1 + poly (r)),
     with poly(r) in [1/sqrt(2), sqrt(2)] and
     x = r + n * log10 (2), with r in [-log10(2)/2, log10(2)/2].  */
  float32x4_t log10_2_c24 = vld1q_f32 (&d->log10_2_high);
  float32x4_t n = vrndaq_f32 (vmulq_f32 (x, d->inv_log10_2));
  float32x4_t r = vfmsq_laneq_f32 (x, n, log10_2_c24, 0);
  r = vfmaq_laneq_f32 (r, n, log10_2_c24, 1);
  uint32x4_t e = vshlq_n_u32 (vreinterpretq_u32_s32 (vcvtaq_s32_f32 (n)), 23);

  float32x4_t scale = vreinterpretq_f32_u32 (vaddq_u32 (e, d->exponent_bias));

  uint32x4_t cmp = vcagtq_f32 (n, d->special_bound);

  float32x4_t r2 = vmulq_f32 (r, r);
  float32x4_t p12 = vfmaq_laneq_f32 (d->c1, r, log10_2_c24, 2);
  float32x4_t p34 = vfmaq_laneq_f32 (d->c3, r, log10_2_c24, 3);
  float32x4_t p14 = vfmaq_f32 (p12, r2, p34);
  float32x4_t poly = vfmaq_f32 (vmulq_f32 (r, d->c0), p14, r2);

  if (__glibc_unlikely (v_any_u32 (cmp)))
    return special_case (poly, n, e, cmp, scale, d);
  return vfmaq_f32 (scale, poly, scale);
}
libmvec_hidden_def (V_NAME_F1 (exp10))
HALF_WIDTH_ALIAS_F1 (exp10)
