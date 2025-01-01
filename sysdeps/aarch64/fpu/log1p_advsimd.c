/* Double-precision AdvSIMD log1p

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

#define WANT_V_LOG1P_K0_SHORTCUT 0
#include "v_log1p_inline.h"

const static struct data
{
  struct v_log1p_data d;
  uint64x2_t inf, minus_one;
} data = { .d = V_LOG1P_CONSTANTS_TABLE,
	   .inf = V2 (0x7ff0000000000000),
	   .minus_one = V2 (0xbff0000000000000) };

#define BottomMask v_u64 (0xffffffff)

static float64x2_t NOINLINE VPCS_ATTR
special_case (float64x2_t x, uint64x2_t cmp, const struct data *d)
{
  /* Side-step special lanes so fenv exceptions are not triggered
     inadvertently.  */
  float64x2_t x_nospecial = v_zerofy_f64 (x, cmp);
  return v_call_f64 (log1p, x, log1p_inline (x_nospecial, &d->d), cmp);
}

/* Vector log1p approximation using polynomial on reduced interval. Routine is
   a modification of the algorithm used in scalar log1p, with no shortcut for
   k=0 and no narrowing for f and k. Maximum observed error is 2.45 ULP:
   _ZGVnN2v_log1p(0x1.658f7035c4014p+11) got 0x1.fd61d0727429dp+2
					want 0x1.fd61d0727429fp+2 .  */
VPCS_ATTR float64x2_t V_NAME_D1 (log1p) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);
  uint64x2_t ix = vreinterpretq_u64_f64 (x);
  uint64x2_t ia = vreinterpretq_u64_f64 (vabsq_f64 (x));

  uint64x2_t special_cases
      = vorrq_u64 (vcgeq_u64 (ia, d->inf), vcgeq_u64 (ix, d->minus_one));

  if (__glibc_unlikely (v_any_u64 (special_cases)))
    return special_case (x, special_cases, d);

  return log1p_inline (x, &d->d);
}

strong_alias (V_NAME_D1 (log1p), V_NAME_D1 (logp1))
