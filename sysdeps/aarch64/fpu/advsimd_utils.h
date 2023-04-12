/* Helpers for Advanced SIMD vector math funtions.

   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <arm_neon.h>

#define VPCS_ATTR __attribute__ ((aarch64_vector_pcs))

#define V_NAME_F1(fun) _ZGVnN4v_##fun##f
#define V_NAME_D1(fun) _ZGVnN2v_##fun
#define V_NAME_F2(fun) _ZGVnN4vv_##fun##f
#define V_NAME_D2(fun) _ZGVnN2vv_##fun

static __always_inline float32x4_t
v_call_f32 (float (*f) (float), float32x4_t x)
{
  return (float32x4_t){ f (x[0]), f (x[1]), f (x[2]), f (x[3]) };
}

static __always_inline float64x2_t
v_call_f64 (double (*f) (double), float64x2_t x)
{
  return (float64x2_t){ f (x[0]), f (x[1]) };
}
