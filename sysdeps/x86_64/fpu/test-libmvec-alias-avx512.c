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

extern double log_vlen8 (double);
extern double exp_vlen8 (double);
extern double pow_vlen8 (double, double);

extern float logf_vlen16 (float);
extern float expf_vlen16 (float);
extern float powf_vlen16 (float, float);

int main(void)
{
  if (!HAS_ARCH_FEATURE (AVX512F_Usable)) return 0;

  if (log_vlen8(1.0) != 0.0
      || logf_vlen16(1.0) != 0.0) abort();

  if (exp_vlen8(0.0) != 1.0
      || expf_vlen16(0.0) != 1.0) abort();

  if (pow_vlen8(1.0, 1.0) != 1.0
      || powf_vlen16(1.0, 1.0) != 1.0) abort();

  return 0;
}
