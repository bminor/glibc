/* Single-precision vector (Advanced SIMD) acosh function

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

#include "v_log1pf_inline.h"

#define SquareLim 0x1p64

const static struct data
{
  struct v_log1pf_data log1pf_consts;
  uint32x4_t one;
} data = { .log1pf_consts = V_LOG1PF_CONSTANTS_TABLE, .one = V4 (0x3f800000) };

#define Thresh vdup_n_u16 (0x2000) /* top(asuint(SquareLim) - asuint(1)).  */

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, float32x4_t y, uint16x4_t special,
	      const struct v_log1pf_data *d)
{
  return v_call_f32 (acoshf, x, log1pf_inline (y, d), vmovl_u16 (special));
}

/* Vector approximation for single-precision acosh, based on log1p. Maximum
   error depends on WANT_SIMD_EXCEPT. With SIMD fp exceptions enabled, it
   is 3.00 ULP:
   _ZGVnN4v_acoshf(0x1.01df3ap+0) got 0x1.ef0a82p-4
				 want 0x1.ef0a7cp-4.
   With exceptions disabled, we can compute u with a shorter dependency chain,
   which gives maximum error of 3.22 ULP:
   _ZGVnN4v_acoshf(0x1.007ef2p+0) got 0x1.fdcdccp-5
				 want 0x1.fdcdd2p-5.  */

VPCS_ATTR float32x4_t NOINLINE V_NAME_F1 (acosh) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);
  uint32x4_t ix = vreinterpretq_u32_f32 (x);
  uint16x4_t special = vcge_u16 (vsubhn_u32 (ix, d->one), Thresh);

#if WANT_SIMD_EXCEPT
  /* Mask special lanes with 1 to side-step spurious invalid or overflow. Use
     only xm1 to calculate u, as operating on x will trigger invalid for NaN.
     Widening sign-extend special predicate in order to mask with it.  */
  uint32x4_t p
      = vreinterpretq_u32_s32 (vmovl_s16 (vreinterpret_s16_u16 (special)));
  float32x4_t xm1 = v_zerofy_f32 (vsubq_f32 (x, v_f32 (1)), p);
  float32x4_t u = vfmaq_f32 (vaddq_f32 (xm1, xm1), xm1, xm1);
#else
  float32x4_t xm1 = vsubq_f32 (x, vreinterpretq_f32_u32 (d->one));
  float32x4_t u
      = vmulq_f32 (xm1, vaddq_f32 (x, vreinterpretq_f32_u32 (d->one)));
#endif

  float32x4_t y = vaddq_f32 (xm1, vsqrtq_f32 (u));

  if (__glibc_unlikely (v_any_u16h (special)))
    return special_case (x, y, special, &d->log1pf_consts);
  return log1pf_inline (y, &d->log1pf_consts);
}
libmvec_hidden_def (V_NAME_F1 (acosh))
HALF_WIDTH_ALIAS_F1 (acosh)
