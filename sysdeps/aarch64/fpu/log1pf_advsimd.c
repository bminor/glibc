/* Single-precision AdvSIMD log1p

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

#include "v_math.h"
#include "v_log1pf_inline.h"

#if WANT_SIMD_EXCEPT

const static struct data
{
  uint32x4_t minus_one, thresh;
  struct v_log1pf_data d;
} data = {
  .d = V_LOG1PF_CONSTANTS_TABLE,
  .thresh = V4 (0x4b800000), /* asuint32(INFINITY) - TinyBound.  */
  .minus_one = V4 (0xbf800000),
};

/* asuint32(0x1p-23). ulp=0.5 at 0x1p-23.  */
#  define TinyBound v_u32 (0x34000000)

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, uint32x4_t cmp, const struct data *d)
{
  /* Side-step special lanes so fenv exceptions are not triggered
     inadvertently.  */
  float32x4_t x_nospecial = v_zerofy_f32 (x, cmp);
  return v_call_f32 (log1pf, x, log1pf_inline (x_nospecial, &d->d), cmp);
}

/* Vector log1pf approximation using polynomial on reduced interval. Worst-case
   error is 1.69 ULP:
   _ZGVnN4v_log1pf(0x1.04418ap-2) got 0x1.cfcbd8p-3
				 want 0x1.cfcbdcp-3.  */
VPCS_ATTR float32x4_t V_NAME_F1 (log1p) (float32x4_t x)
{
  const struct data *d = ptr_barrier (&data);
  uint32x4_t ix = vreinterpretq_u32_f32 (x);
  uint32x4_t ia = vreinterpretq_u32_f32 (vabsq_f32 (x));

  uint32x4_t special_cases
      = vorrq_u32 (vcgeq_u32 (vsubq_u32 (ia, TinyBound), d->thresh),
		   vcgeq_u32 (ix, d->minus_one));

  if (__glibc_unlikely (v_any_u32 (special_cases)))
    return special_case (x, special_cases, d);

  return log1pf_inline (x, &d->d);
}

#else

const static struct v_log1pf_data data = V_LOG1PF_CONSTANTS_TABLE;

static float32x4_t NOINLINE VPCS_ATTR
special_case (float32x4_t x, uint32x4_t cmp)
{
  return v_call_f32 (log1pf, x, log1pf_inline (x, ptr_barrier (&data)), cmp);
}

/* Vector log1pf approximation using polynomial on reduced interval. Worst-case
   error is 1.63 ULP:
   _ZGVnN4v_log1pf(0x1.216d12p-2) got 0x1.fdcb12p-3
				 want 0x1.fdcb16p-3.  */
VPCS_ATTR float32x4_t V_NAME_F1 (log1p) (float32x4_t x)
{
  uint32x4_t special_cases = vornq_u32 (vcleq_f32 (x, v_f32 (-1)),
					vcaleq_f32 (x, v_f32 (0x1p127f)));

  if (__glibc_unlikely (v_any_u32 (special_cases)))
    return special_case (x, special_cases);

  return log1pf_inline (x, ptr_barrier (&data));
}

#endif

libmvec_hidden_def (V_NAME_F1 (log1p))
HALF_WIDTH_ALIAS_F1 (log1p)
strong_alias (V_NAME_F1 (log1p), V_NAME_F1 (logp1))
libmvec_hidden_def (V_NAME_F1 (logp1))
HALF_WIDTH_ALIAS_F1 (logp1)
