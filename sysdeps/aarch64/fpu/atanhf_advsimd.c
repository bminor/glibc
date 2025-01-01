/* Single-precision vector (Advanced SIMD) atanh function

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
  uint32x4_t one;
#if WANT_SIMD_EXCEPT
  uint32x4_t tiny_bound;
#endif
} data = {
  .log1pf_consts = V_LOG1PF_CONSTANTS_TABLE,
  .one = V4 (0x3f800000),
#if WANT_SIMD_EXCEPT
  /* 0x1p-12, below which atanhf(x) rounds to x.  */
  .tiny_bound = V4 (0x39800000),
#endif
};

#define AbsMask v_u32 (0x7fffffff)
#define Half v_u32 (0x3f000000)

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, float32x4_t halfsign, float32x4_t y,
	      uint32x4_t special)
{
  return v_call_f32 (atanhf, vbslq_f32 (AbsMask, x, halfsign),
		     vmulq_f32 (halfsign, y), special);
}

/* Approximation for vector single-precision atanh(x) using modified log1p.
   The maximum error is 2.93 ULP:
   _ZGVnN4v_atanhf(0x1.f43d7p-5) got 0x1.f4dcfep-5
				want 0x1.f4dcf8p-5.  */
VPCS_ATTR float32x4_t NOINLINE V_NAME_F1 (atanh) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);

  float32x4_t halfsign = vbslq_f32 (AbsMask, v_f32 (0.5), x);
  float32x4_t ax = vabsq_f32 (x);
  uint32x4_t iax = vreinterpretq_u32_f32 (ax);

#if WANT_SIMD_EXCEPT
  uint32x4_t special
      = vorrq_u32 (vcgeq_u32 (iax, d->one), vcltq_u32 (iax, d->tiny_bound));
  /* Side-step special cases by setting those lanes to 0, which will trigger no
     exceptions. These will be fixed up later.  */
  if (__glibc_unlikely (v_any_u32 (special)))
    ax = v_zerofy_f32 (ax, special);
#else
  uint32x4_t special = vcgeq_u32 (iax, d->one);
#endif

  float32x4_t y = vdivq_f32 (vaddq_f32 (ax, ax),
			     vsubq_f32 (vreinterpretq_f32_u32 (d->one), ax));
  y = log1pf_inline (y, &d->log1pf_consts);

  /* If exceptions not required, pass ax to special-case for shorter dependency
     chain. If exceptions are required ax will have been zerofied, so have to
     pass x.  */
  if (__glibc_unlikely (v_any_u32 (special)))
#if WANT_SIMD_EXCEPT
    return special_case (x, halfsign, y, special);
#else
    return special_case (ax, halfsign, y, special);
#endif
  return vmulq_f32 (halfsign, y);
}
libmvec_hidden_def (V_NAME_F1 (atanh))
HALF_WIDTH_ALIAS_F1 (atanh)
