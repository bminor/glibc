/* Single-precision SVE log1p

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

#include "sv_math.h"
#include "sv_log1pf_inline.h"

static svfloat32_t NOINLINE
special_case (svfloat32_t x, svbool_t special)
{
  return sv_call_f32 (log1pf, x, sv_log1pf_inline (x, svptrue_b32 ()),
		      special);
}

/* Vector log1pf approximation using polynomial on reduced interval. Worst-case
   error is 1.27 ULP very close to 0.5.
   _ZGVsMxv_log1pf(0x1.fffffep-2) got 0x1.9f324p-2
				 want 0x1.9f323ep-2.  */
svfloat32_t SV_NAME_F1 (log1p) (svfloat32_t x, svbool_t pg)
{
  /* x < -1, Inf/Nan.  */
  svbool_t special = svcmpeq (pg, svreinterpret_u32 (x), 0x7f800000);
  special = svorn_z (pg, special, svcmpge (pg, x, -1));

  if (__glibc_unlikely (svptest_any (pg, special)))
    return special_case (x, special);

  return sv_log1pf_inline (x, pg);
}

strong_alias (SV_NAME_F1 (log1p), SV_NAME_F1 (logp1))
