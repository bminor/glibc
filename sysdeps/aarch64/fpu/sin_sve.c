/* Double-precision vector (SVE) sin function.

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

#include "sv_math.h"

static const struct data
{
  double inv_pi, half_pi, inv_pi_over_2, pi_over_2_1, pi_over_2_2, pi_over_2_3,
      shift;
} data = {
  /* Polynomial coefficients are hard-wired in the FTMAD instruction.  */
  .inv_pi = 0x1.45f306dc9c883p-2,
  .half_pi = 0x1.921fb54442d18p+0,
  .inv_pi_over_2 = 0x1.45f306dc9c882p-1,
  .pi_over_2_1 = 0x1.921fb50000000p+0,
  .pi_over_2_2 = 0x1.110b460000000p-26,
  .pi_over_2_3 = 0x1.1a62633145c07p-54,
  .shift = 0x1.8p52
};

#define RangeVal 0x4160000000000000 /* asuint64 (0x1p23).  */

static svfloat64_t NOINLINE
special_case (svfloat64_t x, svfloat64_t y, svbool_t cmp)
{
  return sv_call_f64 (sin, x, y, cmp);
}

/* A fast SVE implementation of sin based on trigonometric
   instructions (FTMAD, FTSSEL, FTSMUL).
   Maximum observed error in 2.52 ULP:
   SV_NAME_D1 (sin)(0x1.2d2b00df69661p+19) got 0x1.10ace8f3e786bp-40
					  want 0x1.10ace8f3e7868p-40.  */
svfloat64_t SV_NAME_D1 (sin) (svfloat64_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);

  svfloat64_t r = svabs_f64_x (pg, x);
  svuint64_t sign
      = sveor_u64_x (pg, svreinterpret_u64_f64 (x), svreinterpret_u64_f64 (r));
  svbool_t cmp = svcmpge_n_u64 (pg, svreinterpret_u64_f64 (r), RangeVal);

  /* Load first two pio2-related constants to one vector.  */
  svfloat64_t invpio2_and_pio2_1
      = svld1rq_f64 (svptrue_b64 (), &d->inv_pi_over_2);

  /* n = rint(|x|/(pi/2)).  */
  svfloat64_t q = svmla_lane_f64 (sv_f64 (d->shift), r, invpio2_and_pio2_1, 0);
  svfloat64_t n = svsub_n_f64_x (pg, q, d->shift);

  /* r = |x| - n*(pi/2)  (range reduction into -pi/4 .. pi/4).  */
  r = svmls_lane_f64 (r, n, invpio2_and_pio2_1, 1);
  r = svmls_n_f64_x (pg, r, n, d->pi_over_2_2);
  r = svmls_n_f64_x (pg, r, n, d->pi_over_2_3);

  /* Final multiplicative factor: 1.0 or x depending on bit #0 of q.  */
  svfloat64_t f = svtssel_f64 (r, svreinterpret_u64_f64 (q));

  /* sin(r) poly approx.  */
  svfloat64_t r2 = svtsmul_f64 (r, svreinterpret_u64_f64 (q));
  svfloat64_t y = sv_f64 (0.0);
  y = svtmad_f64 (y, r2, 7);
  y = svtmad_f64 (y, r2, 6);
  y = svtmad_f64 (y, r2, 5);
  y = svtmad_f64 (y, r2, 4);
  y = svtmad_f64 (y, r2, 3);
  y = svtmad_f64 (y, r2, 2);
  y = svtmad_f64 (y, r2, 1);
  y = svtmad_f64 (y, r2, 0);

  /* Apply factor.  */
  y = svmul_f64_x (pg, f, y);

  /* sign = y^sign.  */
  y = svreinterpret_f64_u64 (
      sveor_u64_x (pg, svreinterpret_u64_f64 (y), sign));

  if (__glibc_unlikely (svptest_any (pg, cmp)))
    return special_case (x, y, cmp);
  return y;
}
