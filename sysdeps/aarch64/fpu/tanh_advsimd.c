/* Double-precision vector (Advanced SIMD) tanh function

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

#include "v_math.h"
#include "v_expm1_inline.h"

static const struct data
{
  struct v_expm1_data d;
  uint64x2_t thresh, tiny_bound;
} data = {
  .d = V_EXPM1_DATA,
  .tiny_bound = V2 (0x3e40000000000000), /* asuint64 (0x1p-27).  */
  /* asuint64(0x1.241bf835f9d5fp+4) - asuint64(tiny_bound).  */
  .thresh = V2 (0x01f241bf835f9d5f),
};

static float64x2_t NOINLINE VPCS_ATTR
special_case (float64x2_t x, float64x2_t q, float64x2_t qp2,
	      uint64x2_t special)
{
  return v_call_f64 (tanh, x, vdivq_f64 (q, qp2), special);
}

/* Vector approximation for double-precision tanh(x), using a simplified
   version of expm1. The greatest observed error is 2.70 ULP:
   _ZGVnN2v_tanh(-0x1.c59aa220cb177p-3) got -0x1.be5452a6459fep-3
				       want -0x1.be5452a6459fbp-3.  */
float64x2_t VPCS_ATTR V_NAME_D1 (tanh) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);

  uint64x2_t ia = vreinterpretq_u64_f64 (vabsq_f64 (x));

  float64x2_t u = x;

  /* Trigger special-cases for tiny, boring and infinity/NaN.  */
  uint64x2_t special = vcgtq_u64 (vsubq_u64 (ia, d->tiny_bound), d->thresh);
#if WANT_SIMD_EXCEPT
  /* To trigger fp exceptions correctly, set special lanes to a neutral value.
     They will be fixed up later by the special-case handler.  */
  if (__glibc_unlikely (v_any_u64 (special)))
    u = v_zerofy_f64 (u, special);
#endif

  u = vaddq_f64 (u, u);

  /* tanh(x) = (e^2x - 1) / (e^2x + 1).  */
  float64x2_t q = expm1_inline (u, &d->d);
  float64x2_t qp2 = vaddq_f64 (q, v_f64 (2.0));

  if (__glibc_unlikely (v_any_u64 (special)))
    return special_case (x, q, qp2, special);
  return vdivq_f64 (q, qp2);
}
