/* Double-precision vector (SVE) cosh function

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

static const struct data
{
  double c0, c2;
  double c1, c3;
  float64_t inv_ln2, ln2_hi, ln2_lo, shift;
  uint64_t special_bound;
} data = {
  /* Generated using Remez, in [-log(2)/128, log(2)/128].  */
  .c0 = 0x1.fffffffffdbcdp-2,
  .c1 = 0x1.555555555444cp-3,
  .c2 = 0x1.555573c6a9f7dp-5,
  .c3 = 0x1.1111266d28935p-7,
  .ln2_hi = 0x1.62e42fefa3800p-1,
  .ln2_lo = 0x1.ef35793c76730p-45,
  /* 1/ln2.  */
  .inv_ln2 = 0x1.71547652b82fep+0,
  .shift = 0x1.800000000ff80p+46, /* 1.5*2^46+1022.  */

  /* asuint(ln(2^(1024 - 1/128))), the value above which exp overflows.  */
  .special_bound = 0x40862e37e7d8ba72,
};

/* Helper for approximating exp(x)/2.
   Functionally identical to FEXPA exp(x), but an adjustment in
   the shift value which leads to a reduction in the exponent of scale by 1,
   thus halving the result at no cost.  */
static inline svfloat64_t
exp_over_two_inline (const svbool_t pg, svfloat64_t x, const struct data *d)
{
  /* Calculate exp(x).  */
  svfloat64_t z = svmla_x (pg, sv_f64 (d->shift), x, d->inv_ln2);
  svuint64_t u = svreinterpret_u64 (z);
  svfloat64_t n = svsub_x (pg, z, d->shift);

  svfloat64_t c13 = svld1rq (svptrue_b64 (), &d->c1);
  svfloat64_t ln2 = svld1rq (svptrue_b64 (), &d->ln2_hi);

  svfloat64_t r = x;
  r = svmls_lane (r, n, ln2, 0);
  r = svmls_lane (r, n, ln2, 1);

  svfloat64_t r2 = svmul_x (svptrue_b64 (), r, r);
  svfloat64_t p01 = svmla_lane (sv_f64 (d->c0), r, c13, 0);
  svfloat64_t p23 = svmla_lane (sv_f64 (d->c2), r, c13, 1);
  svfloat64_t p04 = svmla_x (pg, p01, p23, r2);
  svfloat64_t p = svmla_x (pg, r, p04, r2);

  svfloat64_t scale = svexpa (u);

  return svmla_x (pg, scale, scale, p);
}

/* Vectorised special case to handle values past where exp_inline overflows.
   Halves the input value and uses the identity exp(x) = exp(x/2)^2 to double
   the valid range of inputs, and returns inf for anything past that.  */
static svfloat64_t NOINLINE
special_case (svbool_t pg, svbool_t special, svfloat64_t ax, svfloat64_t t,
	      const struct data *d)
{
  /* Finish fast path to compute values for non-special cases.  */
  svfloat64_t inv_twoexp = svdivr_x (pg, t, 0.25);
  svfloat64_t y = svadd_x (pg, t, inv_twoexp);

  /* Halves input value, and then check if any cases
     are still going to overflow.  */
  ax = svmul_x (special, ax, 0.5);
  svbool_t is_safe
      = svcmplt (special, svreinterpret_u64 (ax), d->special_bound);

  /* Computes exp(x/2), and sets any overflowing lanes to inf.  */
  svfloat64_t half_exp = exp_over_two_inline (special, ax, d);
  half_exp = svsel (is_safe, half_exp, sv_f64 (INFINITY));

  /* Construct special case cosh(x) = (exp(x/2)^2)/2.  */
  svfloat64_t exp = svmul_x (svptrue_b64 (), half_exp, 2);
  svfloat64_t special_y = svmul_x (special, exp, half_exp);

  /* Select correct return values for special and non-special cases.  */
  special_y = svsel (special, special_y, y);

  /* Ensure an input of nan is correctly propagated.  */
  svbool_t is_nan
      = svcmpgt (special, svreinterpret_u64 (ax), sv_u64 (0x7ff0000000000000));
  return svsel (is_nan, ax, svsel (special, special_y, y));
}

/* Approximation for SVE double-precision cosh(x) using exp_inline.
   cosh(x) = (exp(x) + exp(-x)) / 2.
   The greatest observed error in special case region is 2.66 + 0.5 ULP:
   _ZGVsMxv_cosh (0x1.633b532ffbc1ap+9) got 0x1.f9b2d3d22399ep+1023
				       want 0x1.f9b2d3d22399bp+1023

  The greatest observed error in the non-special region is 1.01 + 0.5 ULP:
  _ZGVsMxv_cosh (0x1.998ecbb3c1f81p+1) got 0x1.890b225657f84p+3
				      want 0x1.890b225657f82p+3.  */
svfloat64_t SV_NAME_D1 (cosh) (svfloat64_t x, const svbool_t pg)
{
  const struct data *d = ptr_barrier (&data);

  svfloat64_t ax = svabs_x (pg, x);
  svbool_t special = svcmpgt (pg, svreinterpret_u64 (ax), d->special_bound);

  /* Up to the point that exp overflows, we can use it to calculate cosh by
     (exp(|x|)/2 + 1) / (2 * exp(|x|)).  */
  svfloat64_t half_exp = exp_over_two_inline (pg, ax, d);

  /* Falls back to entirely standalone vectorized special case.  */
  if (__glibc_unlikely (svptest_any (pg, special)))
    return special_case (pg, special, ax, half_exp, d);

  svfloat64_t inv_twoexp = svdivr_x (pg, half_exp, 0.25);
  return svadd_x (pg, half_exp, inv_twoexp);
}
