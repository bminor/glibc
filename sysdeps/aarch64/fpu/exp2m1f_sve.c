/* Single-precision (SVE) exp2m1 function

   Copyright (C) 2025 Free Software Foundation, Inc.
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

#include "sv_math.h"

/* Value of |x| above which scale overflows without special treatment.  */
#define SpecialBound 126.0f /* rint (log2 (2^127 / (1 + sqrt (2)))).  */

/* Value of n above which scale overflows even with special treatment.  */
#define ScaleBound 192.0f

static const struct data
{
  uint32_t exponent_bias, special_offset, special_bias;
  float32_t scale_thresh, special_bound;
  float log2_lo, c2, c4, c6;
  float log2_hi, c1, c3, c5, shift;
} data = {
  /* Coefficients generated using remez's algorithm for exp2m1f(x).  */
  .log2_hi = 0x1.62e43p-1,
  .log2_lo = -0x1.05c610p-29,
  .c1 = 0x1.ebfbep-3,
  .c2 = 0x1.c6b06ep-5,
  .c3 = 0x1.3b2a5cp-7,
  .c4 = 0x1.5da59ep-10,
  .c5 = 0x1.440dccp-13,
  .c6 = 0x1.e081d6p-17,
  .exponent_bias = 0x3f800000,
  .special_offset = 0x82000000,
  .special_bias = 0x7f000000,
  .scale_thresh = ScaleBound,
  .special_bound = SpecialBound,
};

static svfloat32_t NOINLINE
special_case (svfloat32_t poly, svfloat32_t n, svuint32_t e, svbool_t cmp1,
	      svfloat32_t scale, const struct data *d)
{
  svbool_t b = svcmple (svptrue_b32 (), n, 0.0f);
  svfloat32_t s1 = svreinterpret_f32 (
      svsel (b, sv_u32 (d->special_offset + d->special_bias),
	     sv_u32 (d->special_bias)));
  svfloat32_t s2
      = svreinterpret_f32 (svsub_m (b, e, sv_u32 (d->special_offset)));
  svbool_t cmp2 = svacgt (svptrue_b32 (), n, d->scale_thresh);
  svfloat32_t r2 = svmul_x (svptrue_b32 (), s1, s1);
  svfloat32_t r1
      = svmul_x (svptrue_b32 (), svmla_x (svptrue_b32 (), s2, poly, s2), s1);
  svfloat32_t r0 = svmla_x (svptrue_b32 (), scale, poly, scale);
  svfloat32_t r = svsel (cmp1, r1, r0);
  return svsub_x (svptrue_b32 (), svsel (cmp2, r2, r), 1.0f);
}

/* Single-precision vector exp2(x) - 1 function.
   The maximum error is  1.76 + 0.5 ULP.
   _ZGVsMxv_exp2m1f (0x1.018af8p-1) got 0x1.ab2ebcp-2
				   want 0x1.ab2ecp-2.  */
svfloat32_t SV_NAME_F1 (exp2m1) (svfloat32_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);

  svfloat32_t n = svrinta_x (pg, x);
  svfloat32_t r = svsub_x (pg, x, n);

  svuint32_t e = svlsl_x (pg, svreinterpret_u32 (svcvt_s32_x (pg, n)), 23);
  svfloat32_t scale
      = svreinterpret_f32 (svadd_n_u32_x (pg, e, d->exponent_bias));

  svbool_t cmp = svacgt_n_f32 (pg, n, d->special_bound);

  svfloat32_t r2 = svmul_x (pg, r, r);

  svfloat32_t log2lo_c246 = svld1rq (svptrue_b32 (), &d->log2_lo);
  svfloat32_t p56 = svmla_lane (sv_f32 (d->c5), r, log2lo_c246, 3);
  svfloat32_t p34 = svmla_lane (sv_f32 (d->c3), r, log2lo_c246, 2);
  svfloat32_t p12 = svmla_lane (sv_f32 (d->c1), r, log2lo_c246, 1);

  svfloat32_t p36 = svmla_x (pg, p34, p56, r2);
  svfloat32_t p16 = svmla_x (pg, p12, p36, r2);

  svfloat32_t poly = svmla_lane (
      svmul_x (svptrue_b32 (), r, sv_f32 (d->log2_hi)), r, log2lo_c246, 0);
  poly = svmla_x (pg, poly, p16, r2);

  svfloat32_t y = svmla_x (pg, svsub_x (pg, scale, 1.0f), poly, scale);

  /* Fallback to special case for lanes with overflow.  */
  if (__glibc_unlikely (svptest_any (pg, cmp)))
    return svsel_f32 (cmp, special_case (poly, n, e, cmp, scale, d), y);

  return y;
}
