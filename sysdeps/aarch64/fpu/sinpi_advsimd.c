/* Double-precision (Advanced SIMD) sinpi function

   Copyright (C) 2024 Free Software Foundation, Inc.
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
#include "poly_advsimd_f64.h"

static const struct data
{
  float64x2_t poly[10];
} data = {
  /* Polynomial coefficients generated using Remez algorithm,
     see sinpi.sollya for details.  */
  .poly = { V2 (0x1.921fb54442d184p1), V2 (-0x1.4abbce625be53p2),
	    V2 (0x1.466bc6775ab16p1), V2 (-0x1.32d2cce62dc33p-1),
	    V2 (0x1.507834891188ep-4), V2 (-0x1.e30750a28c88ep-8),
	    V2 (0x1.e8f48308acda4p-12), V2 (-0x1.6fc0032b3c29fp-16),
	    V2 (0x1.af86ae521260bp-21), V2 (-0x1.012a9870eeb7dp-25) },
};

#if WANT_SIMD_EXCEPT
# define TinyBound v_u64 (0x3bf0000000000000) /* asuint64(0x1p-64).  */
/* asuint64(0x1p64) - TinyBound.  */
# define Thresh v_u64 (0x07f0000000000000)

static float64x2_t VPCS_ATTR NOINLINE
special_case (float64x2_t x, float64x2_t y, uint64x2_t odd, uint64x2_t cmp)
{
  /* Fall back to scalar code.  */
  y = vreinterpretq_f64_u64 (veorq_u64 (vreinterpretq_u64_f64 (y), odd));
  return v_call_f64 (sinpi, x, y, cmp);
}
#endif

/* Approximation for vector double-precision sinpi(x).
   Maximum Error 3.05 ULP:
  _ZGVnN2v_sinpi(0x1.d32750db30b4ap-2) got 0x1.fb295878301c7p-1
				      want 0x1.fb295878301cap-1.  */
float64x2_t VPCS_ATTR V_NAME_D1 (sinpi) (float64x2_t x)
{
  const struct data *d = ptr_barrier (&data);

#if WANT_SIMD_EXCEPT
  uint64x2_t ir = vreinterpretq_u64_f64 (vabsq_f64 (x));
  uint64x2_t cmp = vcgeq_u64 (vsubq_u64 (ir, TinyBound), Thresh);

  /* When WANT_SIMD_EXCEPT = 1, special lanes should be set to 0
     to avoid them under/overflowing and throwing exceptions.  */
  float64x2_t r = v_zerofy_f64 (x, cmp);
#else
  float64x2_t r = x;
#endif

  /* If r is odd, the sign of the result should be inverted.  */
  uint64x2_t odd
      = vshlq_n_u64 (vreinterpretq_u64_s64 (vcvtaq_s64_f64 (r)), 63);

  /* r = x - rint(x). Range reduction to -1/2 .. 1/2.  */
  r = vsubq_f64 (r, vrndaq_f64 (r));

  /* y = sin(r).  */
  float64x2_t r2 = vmulq_f64 (r, r);
  float64x2_t r4 = vmulq_f64 (r2, r2);
  float64x2_t y = vmulq_f64 (v_pw_horner_9_f64 (r2, r4, d->poly), r);

#if WANT_SIMD_EXCEPT
  if (__glibc_unlikely (v_any_u64 (cmp)))
    return special_case (x, y, odd, cmp);
#endif

  return vreinterpretq_f64_u64 (veorq_u64 (vreinterpretq_u64_f64 (y), odd));
}
