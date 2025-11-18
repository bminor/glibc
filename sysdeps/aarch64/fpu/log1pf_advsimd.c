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
float32x4_t VPCS_ATTR NOINLINE V_NAME_F1 (log1p) (float32x4_t x)
{
  uint32x4_t special_cases = vornq_u32 (vcleq_f32 (x, v_f32 (-1)),
					vcaleq_f32 (x, v_f32 (0x1p127f)));

  if (__glibc_unlikely (v_any_u32 (special_cases)))
    return special_case (x, special_cases);

  return log1pf_inline (x, ptr_barrier (&data));
}
libmvec_hidden_def (V_NAME_F1 (log1p))
HALF_WIDTH_ALIAS_F1 (log1p)
strong_alias (V_NAME_F1 (log1p), V_NAME_F1 (logp1))
libmvec_hidden_def (V_NAME_F1 (logp1))
HALF_WIDTH_ALIAS_F1 (logp1)
