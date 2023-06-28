/* Single-precision vector (SVE) cos function.

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
  float neg_pio2_1, neg_pio2_2, neg_pio2_3, inv_pio2, shift;
} data = {
  /* Polynomial coefficients are hard-wired in FTMAD instructions.  */
  .neg_pio2_1 = -0x1.921fb6p+0f,
  .neg_pio2_2 = 0x1.777a5cp-25f,
  .neg_pio2_3 = 0x1.ee59dap-50f,
  .inv_pio2 = 0x1.45f306p-1f,
  /* Original shift used in AdvSIMD cosf,
     plus a contribution to set the bit #0 of q
     as expected by trigonometric instructions.  */
  .shift = 0x1.800002p+23f
};

#define RangeVal 0x49800000 /* asuint32(0x1p20f).  */

static svfloat32_t NOINLINE
special_case (svfloat32_t x, svfloat32_t y, svbool_t out_of_bounds)
{
  return sv_call_f32 (cosf, x, y, out_of_bounds);
}

/* A fast SVE implementation of cosf based on trigonometric
   instructions (FTMAD, FTSSEL, FTSMUL).
   Maximum measured error: 2.06 ULPs.
   SV_NAME_F1 (cos)(0x1.dea2f2p+19) got 0x1.fffe7ap-6
				   want 0x1.fffe76p-6.  */
svfloat32_t SV_NAME_F1 (cos) (svfloat32_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);

  svfloat32_t r = svabs_f32_x (pg, x);
  svbool_t out_of_bounds
    = svcmpge_n_u32 (pg, svreinterpret_u32_f32 (r), RangeVal);

  /* Load some constants in quad-word chunks to minimise memory access.  */
  svfloat32_t negpio2_and_invpio2
      = svld1rq_f32 (svptrue_b32 (), &d->neg_pio2_1);

  /* n = rint(|x|/(pi/2)).  */
  svfloat32_t q
      = svmla_lane_f32 (sv_f32 (d->shift), r, negpio2_and_invpio2, 3);
  svfloat32_t n = svsub_n_f32_x (pg, q, d->shift);

  /* r = |x| - n*(pi/2)  (range reduction into -pi/4 .. pi/4).  */
  r = svmla_lane_f32 (r, n, negpio2_and_invpio2, 0);
  r = svmla_lane_f32 (r, n, negpio2_and_invpio2, 1);
  r = svmla_lane_f32 (r, n, negpio2_and_invpio2, 2);

  /* Final multiplicative factor: 1.0 or x depending on bit #0 of q.  */
  svfloat32_t f = svtssel_f32 (r, svreinterpret_u32_f32 (q));

  /* cos(r) poly approx.  */
  svfloat32_t r2 = svtsmul_f32 (r, svreinterpret_u32_f32 (q));
  svfloat32_t y = sv_f32 (0.0f);
  y = svtmad_f32 (y, r2, 4);
  y = svtmad_f32 (y, r2, 3);
  y = svtmad_f32 (y, r2, 2);
  y = svtmad_f32 (y, r2, 1);
  y = svtmad_f32 (y, r2, 0);

  /* Apply factor.  */
  y = svmul_f32_x (pg, f, y);

  if (__glibc_unlikely (svptest_any (pg, out_of_bounds)))
    return special_case (x, y, out_of_bounds);
  return y;
}
