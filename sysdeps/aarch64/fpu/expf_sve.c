/* Single-precision vector (SVE) exp function.

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
#include "sv_expf_inline.h"

/* Roughly 87.3. For x < -Thres, the result is subnormal and not handled
   correctly by FEXPA.  */
#define Thres 0x1.5d5e2ap+6f

static const struct data
{
  struct sv_expf_data d;
  float thres;
} data = {
  .d = SV_EXPF_DATA,
  .thres = Thres,
};

static svfloat32_t NOINLINE
special_case (svfloat32_t x, svbool_t special, const struct sv_expf_data *d)
{
  return sv_call_f32 (expf, x, expf_inline (x, svptrue_b32 (), d), special);
}

/* Optimised single-precision SVE exp function.
   Worst-case error is 0.88 +0.50 ULP:
   _ZGVsMxv_expf(-0x1.bba276p-6) got 0x1.f25288p-1
				want 0x1.f2528ap-1.  */
svfloat32_t SV_NAME_F1 (exp) (svfloat32_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);
  svbool_t is_special_case = svacgt (pg, x, d->thres);
  if (__glibc_unlikely (svptest_any (pg, is_special_case)))
    return special_case (x, is_special_case, &d->d);
  return expf_inline (x, pg, &d->d);
}
