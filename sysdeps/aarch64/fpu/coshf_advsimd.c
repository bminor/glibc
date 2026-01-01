/* Single-precision vector (AdvSIMD) cosh function

   Copyright (C) 2024-2026 Free Software Foundation, Inc.
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

#include "v_expf_inline.h"
#include "v_math.h"

static const struct data
{
  struct v_expf_data expf_consts;
  float32x4_t bound;
} data = {
  .expf_consts = V_EXPF_DATA,
  /* 0x1.5a92d8p+6: expf overflows above this, so have to use special case.  */
  .bound = V4 (0x1.5a92d8p+6),
};

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, float32x4_t half_t, float32x4_t half_over_t,
	      uint32x4_t special)
{
  return v_call_f32 (coshf, x, vaddq_f32 (half_t, half_over_t), special);
}

/* Single-precision vector cosh, using vector expf.
   Maximum error is 2.38 ULP:
   _ZGVnN4v_coshf (0x1.e8001ep+1) got 0x1.6a491ep+4
				 want 0x1.6a4922p+4.  */
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (cosh) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);

  uint32x4_t special = vcageq_f32 (x, d->bound);
  float32x4_t t = v_expf_inline (x, &d->expf_consts);

  /* Calculate cosh by exp(x) / 2 + exp(-x) / 2.  */
  float32x4_t half_t = vmulq_n_f32 (t, 0.5);
  float32x4_t half_over_t = vdivq_f32 (v_f32 (0.5), t);

  if (__glibc_unlikely (v_any_u32 (special)))
    return special_case (x, half_t, half_over_t, special);

  return vaddq_f32 (half_t, half_over_t);
}
libmvec_hidden_def (V_NAME_F1 (cosh))
HALF_WIDTH_ALIAS_F1 (cosh)
