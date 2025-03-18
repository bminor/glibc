/* Single-precision vector (SVE) cosh function

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
#include "sv_expf_inline.h"

static const struct data
{
  struct sv_expf_data expf_consts;
  float special_bound;
} data = {
  .expf_consts = SV_EXPF_DATA,
  /* 0x1.5a92d8p+6: expf overflows above this, so have to use special case.  */
  .special_bound = 0x1.5a92d8p+6,
};

static svfloat32_t NOINLINE
special_case (svfloat32_t x, svfloat32_t half_e, svfloat32_t half_over_e,
	      svbool_t pg)
{
  return sv_call_f32 (coshf, x, svadd_x (svptrue_b32 (), half_e, half_over_e),
		      pg);
}

/* Single-precision vector cosh, using vector expf.
   Maximum error is 2.56 +0.5 ULP:
   _ZGVsMxv_coshf(-0x1.5b40f4p+1) got 0x1.e47748p+2
				 want 0x1.e4774ep+2.  */
svfloat32_t SV_NAME_F1 (cosh) (svfloat32_t x, svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);

  svbool_t special = svacge (pg, x, d->special_bound);

  /* Calculate cosh by exp(x) / 2 + exp(-x) / 2.
     Note that x is passed to exp here, rather than |x|. This is to avoid using
     destructive unary ABS for better register usage. However it means the
     routine is not exactly symmetrical, as the exp helper is slightly less
     accurate in the negative range.  */
  svfloat32_t e = expf_inline (x, pg, &d->expf_consts);
  svfloat32_t half_e = svmul_x (svptrue_b32 (), e, 0.5);
  svfloat32_t half_over_e = svdivr_x (pg, e, 0.5);

  if (__glibc_unlikely (svptest_any (pg, special)))
    return special_case (x, half_e, half_over_e, special);

  return svadd_x (svptrue_b32 (), half_e, half_over_e);
}
