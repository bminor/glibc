/* Single-precision vector (Advanced SIMD) asinh function

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
#include "v_log1pf_inline.h"

const static struct data
{
  struct v_log1pf_data log1pf_consts;
  float32x4_t one;
  uint32x4_t big_bound;
#if WANT_SIMD_EXCEPT
  uint32x4_t tiny_bound;
#endif
} data = {
  .one = V4 (1),
  .log1pf_consts = V_LOG1PF_CONSTANTS_TABLE,
  .big_bound = V4 (0x5f800000), /* asuint(0x1p64).  */
#if WANT_SIMD_EXCEPT
  .tiny_bound = V4 (0x30800000) /* asuint(0x1p-30).  */
#endif
};

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, uint32x4_t sign, float32x4_t y,
	      uint32x4_t special, const struct data *d)
{
  return v_call_f32 (
      asinhf, x,
      vreinterpretq_f32_u32 (veorq_u32 (
	  sign, vreinterpretq_u32_f32 (log1pf_inline (y, &d->log1pf_consts)))),
      special);
}

/* Single-precision implementation of vector asinh(x), using vector log1p.
   Worst-case error is 2.59 ULP:
   _ZGVnN4v_asinhf(0x1.d86124p-3) got 0x1.d449bep-3
				 want 0x1.d449c4p-3.  */
VPCS_ATTR float32x4_t NOINLINE V_NAME_F1 (asinh) (float32x4_t x)
{
  const struct data *dat = ptr_barrier (&data);
  float32x4_t ax = vabsq_f32 (x);
  uint32x4_t iax = vreinterpretq_u32_f32 (ax);
  uint32x4_t special = vcgeq_u32 (iax, dat->big_bound);
  uint32x4_t sign = veorq_u32 (vreinterpretq_u32_f32 (x), iax);
  float32x4_t special_arg = x;

#if WANT_SIMD_EXCEPT
  /* Sidestep tiny and large values to avoid inadvertently triggering
     under/overflow.  */
  special = vorrq_u32 (special, vcltq_u32 (iax, dat->tiny_bound));
  if (__glibc_unlikely (v_any_u32 (special)))
    {
      ax = v_zerofy_f32 (ax, special);
      x = v_zerofy_f32 (x, special);
    }
#endif

  /* asinh(x) = log(x + sqrt(x * x + 1)).
     For positive x, asinh(x) = log1p(x + x * x / (1 + sqrt(x * x + 1))).  */
  float32x4_t d
      = vaddq_f32 (v_f32 (1), vsqrtq_f32 (vfmaq_f32 (dat->one, ax, ax)));
  float32x4_t y = vaddq_f32 (ax, vdivq_f32 (vmulq_f32 (ax, ax), d));

  if (__glibc_unlikely (v_any_u32 (special)))
    return special_case (special_arg, sign, y, special, dat);
  return vreinterpretq_f32_u32 (veorq_u32 (
      sign, vreinterpretq_u32_f32 (log1pf_inline (y, &dat->log1pf_consts))));
}
libmvec_hidden_def (V_NAME_F1 (asinh))
HALF_WIDTH_ALIAS_F1 (asinh)
