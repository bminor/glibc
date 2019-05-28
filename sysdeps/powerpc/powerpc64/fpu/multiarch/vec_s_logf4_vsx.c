/* Single-precision vector log function.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdint.h>
#include <shlib-compat.h>
#include <libm-alias-float.h>
#include "math_config_flt.h"

/*
LOGF_TABLE_BITS = 4
LOGF_POLY_ORDER = 4.  */

#define T __logf_data.tab
#define A __logf_data.poly
#define Ln2 __logf_data.ln2
#define N (1 << LOGF_TABLE_BITS)
#define OFF 0x3f330000

/* The vec_float built-in was added to GCC 8.  */
#if !__GNUC_PREREQ (8, 0)
vector float
vec_float (vector signed int xb)
{
  vector float tmp;
  __asm__ volatile ("xvcvsxwsp %x0,%x1" : "=wa" (tmp) : "wa" (xb));
  return tmp;
}
#endif

vector float
_ZGVbN4v_logf (vector float x)
{
  vector float z, r, r2, y, y0, invc, logc;
  vector unsigned int ix, iz, tmp, x_e, x_s, x_n, lsh;
  vector signed int k, i;

  ix = (vector unsigned int) x;
  x_e = ix & vec_splats ((unsigned int) 0x7f800000);
  x_s = ix & vec_splats ((unsigned int) 0x007fffff);

  x_n = (vector unsigned int) (x * vec_splats (0x1p23f));
  lsh = vec_splats ((unsigned int) 23);
  lsh = lsh << lsh;
  x_n = x_n - lsh;

  for (int m=0; m<4; m++)
  {
    /* x is subnormal, normalize it.  */
    if ((x_e[m] == 1) && (x_s[m] != 0))
    {
      ix[m] = x_n[m];
    }
  }
  /* x = 2^k z; where z is in range [OFF,2*OFF] and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - vec_splats((unsigned int) OFF);
  i = (vector signed int)
      (tmp >> vec_splats ((unsigned int) (23 - LOGF_TABLE_BITS)))
      % vec_splats ((unsigned int) N);
  k = ((vector signed int) tmp) >> vec_splats (23); /* Arithmetic shift.  */
  iz = ix - (tmp & vec_splats (0x1ff) << vec_splats (23));

  for (int m=0; m<4; m++)
  {
    invc[m] = T[i[m]].invc;
    logc[m] = T[i[m]].logc;
  }

  z = (vector float) iz;

  /* log(x) = log1p(z/c-1) + log(c) + k*Ln2 */
  r = z * invc - vec_splats((float) 1.0);
  y0 = logc + vec_float (k) * Ln2;

  /* Pipelined polynomial evaluation to approximate log1p(r).  */
  r2 = r * r;
  y = A[1] * r + A[2];
  y = A[0] * r2 + y;
  y = y * r2 + (y0 + r);

  for (int m=0; m<4; m++)
  {
    /* x < 0x1p-126 or inf or nan.  */
    if (__glibc_unlikely (ix[m] - 0x00800000 >= 0x7f800000 - 0x00800000))
      y[m] = logf (x[m]);

#if WANT_ROUNDING
    /* Fix sign of zero with downward rounding when x==1.  */
    if (__glibc_unlikely (ix[m] == 0x3f800000))
      y[m] = 0;
#endif
  }

  return y;
}
