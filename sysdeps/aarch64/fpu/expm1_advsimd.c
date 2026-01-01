/* Double-precision AdvSIMD expm1

   Copyright (C) 2023-2026 Free Software Foundation, Inc.
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
  float64x2_t oflow_bound;
} data = {
  .d = V_EXPM1_DATA,
  /* Value above which expm1(x) should overflow. Absolute value of the
     underflow bound is greater than this, so it catches both cases - there is
     a small window where fallbacks are triggered unnecessarily.  */
  .oflow_bound = V2 (0x1.62b7d369a5aa9p+9),
};

static float64x2_t VPCS_ATTR NOINLINE
special_case (float64x2_t x, uint64x2_t special, const struct data *d)
{
  return v_call_f64 (expm1, x, expm1_inline (x, &d->d), special);
}

/* Double-precision vector exp(x) - 1 function.
   The maximum error observed error is 2.05 ULP:
  _ZGVnN2v_expm1(0x1.6329669eb8c87p-2) got 0x1.a8897eef87b34p-2
				      want 0x1.a8897eef87b32p-2.  */
float64x2_t VPCS_ATTR V_NAME_D1 (expm1) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);

  /* Large input, NaNs and Infs.  */
  uint64x2_t special = vcageq_f64 (x, d->oflow_bound);
  if (__glibc_unlikely (v_any_u64 (special)))
    return special_case (x, special, d);

  /* expm1(x) ~= p * t + (t - 1).  */
  return expm1_inline (x, &d->d);
}
