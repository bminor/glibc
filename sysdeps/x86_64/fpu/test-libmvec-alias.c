/* Part of test to ensure link against *_finite aliases from libmvec.
   Copyright (C) 2016 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <init-arch.h>

extern double log_vlen2 (double);
extern double exp_vlen2 (double);
extern double pow_vlen2 (double, double);

extern double log_vlen4 (double);
extern double exp_vlen4 (double);
extern double pow_vlen4 (double, double);

extern double log_vlen4_avx2 (double);
extern double exp_vlen4_avx2 (double);
extern double pow_vlen4_avx2 (double, double);

extern float logf_vlen4 (float);
extern float expf_vlen4 (float);
extern float powf_vlen4 (float, float);

extern float logf_vlen8 (float);
extern float expf_vlen8 (float);
extern float powf_vlen8 (float, float);

extern float logf_vlen8_avx2 (float);
extern float expf_vlen8_avx2 (float);
extern float powf_vlen8_avx2 (float, float);

int main(void)
{
  if (log_vlen2(1.0) != 0.0
      || logf_vlen4(1.0) != 0.0) abort();

  if (exp_vlen2(0.0) != 1.0
      || expf_vlen4(0.0) != 1.0) abort();

  if (pow_vlen2(1.0, 1.0) != 1.0
      || powf_vlen4(1.0, 1.0) != 1.0) abort();

  if (HAS_ARCH_FEATURE (AVX_Usable))
    {
      if (log_vlen4(1.0) != 0.0
          || logf_vlen8(1.0) != 0.0) abort();

      if (exp_vlen4(0.0) != 1.0
          || expf_vlen8(0.0) != 1.0) abort();

      if (pow_vlen4(1.0, 1.0) != 1.0
          || powf_vlen8(1.0, 1.0) != 1.0) abort();
    }

  if (HAS_ARCH_FEATURE (AVX2_Usable))
    {
      if (log_vlen4_avx2(1.0) != 0.0
          || logf_vlen8_avx2(1.0) != 0.0) abort();

      if (exp_vlen4_avx2(0.0) != 1.0
          || expf_vlen8_avx2(0.0) != 1.0) abort();

      if (pow_vlen4_avx2(1.0, 1.0) != 1.0
          || powf_vlen8_avx2(1.0, 1.0) != 1.0) abort();
    }

  return 0;
}
