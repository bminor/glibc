/* Single-precision (Advanced SIMD) exp2m1 function

   Copyright (C) 2025-2026 Free Software Foundation, Inc.
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

/* Value of |x| above which scale overflows without special treatment.  */
#define SpecialBound 126.0f /* rint (log2 (2^127 / (1 + sqrt (2)))).  */

/* Value of n above which scale overflows even with special treatment.  */
#define ScaleBound 192.0f

static const struct data
{
  uint32x4_t exponent_bias, special_offset, special_bias;
  float32x4_t scale_thresh, special_bound;
  float32x4_t log2_hi, c1, c3, c5;
  float log2_lo, c2, c4, c6;
} data = {
  /* Coefficients generated using remez's algorithm for exp2m1f(x).  */
  .log2_hi = V4 (0x1.62e43p-1),
  .log2_lo = -0x1.05c610p-29,
  .c1 = V4 (0x1.ebfbep-3),
  .c2 = 0x1.c6b06ep-5,
  .c3 = V4 (0x1.3b2a5cp-7),
  .c4 = 0x1.5da59ep-10,
  .c5 = V4 (0x1.440dccp-13),
  .c6 = 0x1.e081d6p-17,
  .exponent_bias = V4 (0x3f800000),
  .special_offset = V4 (0x82000000),
  .special_bias = V4 (0x7f000000),
  .scale_thresh = V4 (ScaleBound),
  .special_bound = V4 (SpecialBound),
};

static float32x4_t VPCS_ATTR
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
  return vsubq_f32 (vbslq_f32 (cmp2, r2, r), v_f32 (1.0f));
}

/* Single-precision vector exp2(x) - 1 function.
   The maximum error is  1.76 + 0.5 ULP.
   _ZGVnN4v_exp2m1f (0x1.018af8p-1) got 0x1.ab2ebcp-2
				   want 0x1.ab2ecp-2.  */
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (exp2m1) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);

  /* exp2(x) = 2^n (1 + poly(r)), with 1 + poly(r) in [1/sqrt(2),sqrt(2)]
     x = n + r, with r in [-1/2, 1/2].  */
  float32x4_t n = vrndaq_f32 (x);
  float32x4_t r = vsubq_f32 (x, n);
  uint32x4_t e = vshlq_n_u32 (vreinterpretq_u32_s32 (vcvtaq_s32_f32 (x)), 23);
  float32x4_t scale = vreinterpretq_f32_u32 (vaddq_u32 (e, d->exponent_bias));

  uint32x4_t cmp = vcagtq_f32 (n, d->special_bound);

  float32x4_t log2lo_c246 = vld1q_f32 (&d->log2_lo);
  float32x4_t r2 = vmulq_f32 (r, r);

  /* Pairwise horner scheme.  */
  float32x4_t p56 = vfmaq_laneq_f32 (d->c5, r, log2lo_c246, 3);
  float32x4_t p34 = vfmaq_laneq_f32 (d->c3, r, log2lo_c246, 2);
  float32x4_t p12 = vfmaq_laneq_f32 (d->c1, r, log2lo_c246, 1);
  float32x4_t p36 = vfmaq_f32 (p34, r2, p56);
  float32x4_t p16 = vfmaq_f32 (p12, r2, p36);
  float32x4_t poly
      = vfmaq_laneq_f32 (vmulq_f32 (d->log2_hi, r), r, log2lo_c246, 0);
  poly = vfmaq_f32 (poly, p16, r2);

  float32x4_t y = vfmaq_f32 (vsubq_f32 (scale, v_f32 (1.0f)), poly, scale);

  if (__glibc_unlikely (v_any_u32 (cmp)))
    return vbslq_f32 (cmp, special_case (poly, n, e, cmp, scale, d), y);

  return y;
}
libmvec_hidden_def (V_NAME_F1 (exp2m1))
HALF_WIDTH_ALIAS_F1 (exp2m1)
