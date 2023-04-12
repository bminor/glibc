/* Helpers for SVE vector math functions.

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

#include <arm_sve.h>

#define SV_NAME_F1(fun) _ZGVsMxv_##fun##f
#define SV_NAME_D1(fun) _ZGVsMxv_##fun
#define SV_NAME_F2(fun) _ZGVsMxvv_##fun##f
#define SV_NAME_D2(fun) _ZGVsMxvv_##fun

static __always_inline svfloat32_t
sv_call_f32 (float (*f) (float), svfloat32_t x, svfloat32_t y, svbool_t cmp)
{
  svbool_t p = svpfirst (cmp, svpfalse ());
  while (svptest_any (cmp, p))
    {
      float elem = svclastb_n_f32 (p, 0, x);
      elem = (*f) (elem);
      svfloat32_t y2 = svdup_n_f32 (elem);
      y = svsel_f32 (p, y2, y);
      p = svpnext_b32 (cmp, p);
    }
  return y;
}

static __always_inline svfloat64_t
sv_call_f64 (double (*f) (double), svfloat64_t x, svfloat64_t y, svbool_t cmp)
{
  svbool_t p = svpfirst (cmp, svpfalse ());
  while (svptest_any (cmp, p))
    {
      double elem = svclastb_n_f64 (p, 0, x);
      elem = (*f) (elem);
      svfloat64_t y2 = svdup_n_f64 (elem);
      y = svsel_f64 (p, y2, y);
      p = svpnext_b64 (cmp, p);
    }
  return y;
}
