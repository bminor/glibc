/* Single-precision vector (SVE) acosh function

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

#include "sv_math.h"
#include "sv_log1pf_inline.h"

#define One 0x3f800000
#define Thres 0x20000000 /* asuint(0x1p64) - One.  */

static svfloat32_t NOINLINE
special_case (svfloat32_t xm1, svfloat32_t tmp, svbool_t special)
{
  svfloat32_t x = svadd_x (svptrue_b32 (), xm1, 1.0f);
  svfloat32_t y = sv_log1pf_inline (tmp, svptrue_b32 ());
  return sv_call_f32 (acoshf, x, y, special);
}

/* Single-precision SVE acosh(x) routine. Implements the same algorithm as
   vector acoshf and log1p.

   Maximum error is 2.47 ULPs:
   SV_NAME_F1 (acosh) (0x1.01ca76p+0) got 0x1.e435a6p-4
				     want 0x1.e435a2p-4.  */
svfloat32_t SV_NAME_F1 (acosh) (svfloat32_t x, const svbool_t pg)
{
  svuint32_t ix = svreinterpret_u32 (x);
  svbool_t special = svcmpge (pg, svsub_x (pg, ix, One), Thres);

  svfloat32_t xm1 = svsub_x (pg, x, 1.0f);
  svfloat32_t u = svmul_x (pg, xm1, svadd_x (pg, x, 1.0f));
  svfloat32_t tmp = svadd_x (pg, xm1, svsqrt_x (pg, u));

  if (__glibc_unlikely (svptest_any (pg, special)))
    return special_case (xm1, tmp, special);
  return sv_log1pf_inline (tmp, pg);
}
