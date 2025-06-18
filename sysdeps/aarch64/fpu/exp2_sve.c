/* Double-precision vector (SVE) exp2 function

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

#include "sv_math.h"

#define BigBound 1022
#define UOFlowBound 1280

static const struct data
{
  double c2, c4;
  double c0, c1, c3;
  double shift, big_bound, uoflow_bound;
} data = {
  /* Coefficients are computed using Remez algorithm with
     minimisation of the absolute error.  */
  .c0 = 0x1.62e42fefa39efp-1,  .c1 = 0x1.ebfbdff82a31bp-3,
  .c2 = 0x1.c6b08d706c8a5p-5,  .c3 = 0x1.3b2ad2ff7d2f3p-7,
  .c4 = 0x1.5d8761184beb3p-10, .shift = 0x1.800000000ffc0p+46,
  .uoflow_bound = UOFlowBound, .big_bound = BigBound,
};

#define SpecialOffset 0x6000000000000000 /* 0x1p513.  */
/* SpecialBias1 + SpecialBias1 = asuint(1.0).  */
#define SpecialBias1 0x7000000000000000 /* 0x1p769.  */
#define SpecialBias2 0x3010000000000000 /* 0x1p-254.  */

/* Update of both special and non-special cases, if any special case is
   detected.  */
static inline svfloat64_t
special_case (svbool_t pg, svfloat64_t s, svfloat64_t y, svfloat64_t n,
	      const struct data *d)
{
  /* s=2^n may overflow, break it up into s=s1*s2,
     such that exp = s + s*y can be computed as s1*(s2+s2*y)
     and s1*s1 overflows only if n>0.  */

  /* If n<=0 then set b to 0x6, 0 otherwise.  */
  svbool_t p_sign = svcmple (pg, n, 0.0); /* n <= 0.  */
  svuint64_t b = svdup_u64_z (p_sign, SpecialOffset);

  /* Set s1 to generate overflow depending on sign of exponent n.  */
  svfloat64_t s1 = svreinterpret_f64 (svsubr_x (pg, b, SpecialBias1));
  /* Offset s to avoid overflow in final result if n is below threshold.  */
  svfloat64_t s2 = svreinterpret_f64 (
      svadd_x (pg, svsub_x (pg, svreinterpret_u64 (s), SpecialBias2), b));

  /* |n| > 1280 => 2^(n) overflows.  */
  svbool_t p_cmp = svacle (pg, n, d->uoflow_bound);

  svfloat64_t r1 = svmul_x (svptrue_b64 (), s1, s1);
  svfloat64_t r2 = svmla_x (pg, s2, s2, y);
  svfloat64_t r0 = svmul_x (svptrue_b64 (), r2, s1);

  return svsel (p_cmp, r0, r1);
}

/* Fast vector implementation of exp2.
   Maximum measured error is 0.52 + 0.5 ulp.
   _ZGVsMxv_exp2 (0x1.3b72ad5b701bfp-1) got 0x1.8861641b49e08p+0
				       want 0x1.8861641b49e07p+0.  */
svfloat64_t SV_NAME_D1 (exp2) (svfloat64_t x, svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);
  svbool_t special = svacge (pg, x, d->big_bound);

  svfloat64_t z = svadd_x (svptrue_b64 (), x, d->shift);
  svfloat64_t n = svsub_x (svptrue_b64 (), z, d->shift);
  svfloat64_t r = svsub_x (svptrue_b64 (), x, n);

  svfloat64_t scale = svexpa (svreinterpret_u64 (z));

  svfloat64_t r2 = svmul_x (svptrue_b64 (), r, r);
  svfloat64_t c24 = svld1rq (svptrue_b64 (), &d->c2);

  /* Approximate exp2(r) using polynomial.  */
  /* y = exp2(r) - 1 ~= r * (C0 + C1 r + C2 r^2 + C3 r^3 + C4 r^4).  */
  svfloat64_t p12 = svmla_lane (sv_f64 (d->c1), r, c24, 0);
  svfloat64_t p34 = svmla_lane (sv_f64 (d->c3), r, c24, 1);
  svfloat64_t p = svmla_x (pg, p12, p34, r2);
  p = svmad_x (pg, p, r, d->c0);
  svfloat64_t y = svmul_x (svptrue_b64 (), r, p);

  /* Assemble exp2(x) = exp2(r) * scale.  */
  if (__glibc_unlikely (svptest_any (pg, special)))
    {
      /* FEXPA zeroes the sign bit, however the sign is meaningful to the
          special case function so needs to be copied.
          e = sign bit of u << 46.  */
      svuint64_t e = svand_x (pg, svlsl_x (pg, svreinterpret_u64 (z), 46),
            0x8000000000000000);
      scale = svreinterpret_f64 (svadd_x (pg, e, svreinterpret_u64 (scale)));
      return special_case (pg, scale, y, n, d);
    }

  return svmla_x (pg, scale, scale, y);
}
