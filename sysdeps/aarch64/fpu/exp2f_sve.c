/* Single-precision vector (SVE) exp2 function

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

#define Thres 0x1.5d5e2ap+6f

static const struct data
{
  float c0, c1, shift, thres;
} data = {
  /* Coefficients generated using Remez algorithm with minimisation of relative
     error.  */
  .c0 = 0x1.62e485p-1,
  .c1 = 0x1.ebfbe0p-3,
  /* 1.5*2^17 + 127.  */
  .shift = 0x1.803f8p17f,
  /* Roughly 87.3. For x < -Thres, the result is subnormal and not handled
     correctly by FEXPA.  */
  .thres = Thres,
};

static inline svfloat32_t
sv_exp2f_inline (svfloat32_t x, const svbool_t pg, const struct data *d)
{
  /* exp2(x) = 2^n (1 + poly(r)), with 1 + poly(r) in [1/sqrt(2),sqrt(2)]
    x = n + r, with r in [-1/2, 1/2].  */
  svfloat32_t z = svadd_x (svptrue_b32 (), x, d->shift);
  svfloat32_t n = svsub_x (svptrue_b32 (), z, d->shift);
  svfloat32_t r = svsub_x (svptrue_b32 (), x, n);

  svfloat32_t scale = svexpa (svreinterpret_u32 (z));

  svfloat32_t poly = svmla_x (pg, sv_f32 (d->c0), r, sv_f32 (d->c1));
  poly = svmul_x (svptrue_b32 (), poly, r);

  return svmla_x (pg, scale, scale, poly);
}

static svfloat32_t NOINLINE
special_case (svfloat32_t x, svbool_t special, const struct data *d)
{
  return sv_call_f32 (exp2f, x, sv_exp2f_inline (x, svptrue_b32 (), d),
		      special);
}

/* Single-precision SVE exp2f routine, based on the FEXPA instruction.
   Worst case error is 1.09 ULPs.
   _ZGVsMxv_exp2f (0x1.9a2a94p-1) got 0x1.be1054p+0
				 want 0x1.be1052p+0.  */
svfloat32_t SV_NAME_F1 (exp2) (svfloat32_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);
  svbool_t special = svacgt (pg, x, d->thres);
  if (__glibc_unlikely (svptest_any (special, special)))
    return special_case (x, special, d);
  return sv_exp2f_inline (x, pg, d);
}
