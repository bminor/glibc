/* Double-precision vector log(x) function.
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
#include <math/math-svid-compat.h>
#include <shlib-compat.h>
#include <libm-alias-double.h>
#include "math_config_dbl.h"
#include <altivec.h>

#define T __log_data.tab
#define T2 __log_data.tab2
#define B __log_data.poly1
#define A __log_data.poly
#define Ln2hi __log_data.ln2hi
#define Ln2lo __log_data.ln2lo
#define N (1 << LOG_TABLE_BITS)
#define OFF 0x3fe6000000000000
#define LO asuint64 (1.0 - 0x1p-4)
#define HI asuint64 (1.0 + 0x1.09p-4)

/* Top 16 bits of a double.  */
static inline uint32_t
top16 (double x)
{
  return asuint64 (x) >> 48;
}

vector double
_ZGVbN2v_log (vector double x)
{
  vector double w, z, r, r2, r3, y, invc, logc, kd, hi, lo;
  vector double rhi, rlo;
  vector unsigned long long ix, iz, tmp, top;
  vector signed long long k, i;

  ix[0] = asuint64 (x[0]);
  ix[1] = asuint64 (x[1]);
  top[0] = top16 (x[0]);
  top[1] = top16 (x[1]);

  if (__glibc_unlikely ((ix[0] - LO < HI - LO) && (ix[1] - LO < HI - LO)))
    {
      /* Handle close to 1.0 inputs separately.  */
      /* Fix sign of zero with downward rounding when x==1.  */
      if (WANT_ROUNDING && __glibc_unlikely ((ix[0] == asuint64 (1.0))
                                             && (ix[1] == asuint64 (1.0))))
        {
          y[0] = 0.0;
          y[1] = 0.0;
          return y;
        }
      else if (WANT_ROUNDING && __glibc_unlikely (ix[0] == asuint64 (1.0)))
        {
          y[0] = 0.0;
          y[1] = log (x[1]);
          return y;
        }
      else if (WANT_ROUNDING && __glibc_unlikely (ix[1] == asuint64 (1.0)))
        {
          y[0] = log (x[0]);
          y[1] = 0.0;
          return y;
        }
      r = x - (vector double) {1.0, 1.0};
      r2 = r * r;
      r3 = r * r2;
      y = r3 * (B[1] + r * B[2] + r2 * B[3]
                + r3 * (B[4] + r * B[5] + r2 * B[6]
                        + r3 * (B[7] + r * B[8] + r2 * B[9] + r3 * B[10])));
      /* Worst-case error is around 0.507 ULP.  */
      w = r * (vector double) {0x1p27, 0x1p27};
      rhi = r + w - w;
      rlo = r - rhi;
      w = rhi * rhi * B[0]; /* B[0] == -0.5.  */
      hi = r + w;
      lo = r - hi + w;
      lo += B[0] * rlo * (rhi + r);
      y += lo;
      y += hi;
      return y;
    }
  else if (__glibc_unlikely ((ix[0] - LO < HI - LO)))
    {
      /* Handle close to 1.0 inputs separately.  */
      /* Fix sign of zero with downward rounding when x==1.  */
      if (WANT_ROUNDING && __glibc_unlikely (ix[0] == asuint64 (1.0)))
        {
          y[0] = 0.0;
          y[1] = log (x[1]);
          return y;
        }
      y[0] = log (x[0]);
      y[1] = log (x[1]);
      return y;
    }
  else if (__glibc_unlikely ((ix[1] - LO < HI - LO)))
    {
      /* Handle close to 1.0 inputs separately.  */
      /* Fix sign of zero with downward rounding when x==1.  */
      if (WANT_ROUNDING && __glibc_unlikely (ix[1] == asuint64 (1.0)))
        {
          y[0] = log (x[0]);
          y[1] = 0.0;
          return y;
        }
      y[0] = log (x[0]);
      y[1] = log (x[1]);
      return y;
    }

  if (__glibc_unlikely ((top[0] - 0x0010 >= 0x7ff0 - 0x0010)
                        && (top[1] - 0x0010 >= 0x7ff0 - 0x0010)))
    {
      /* x < 0x1p-1022 or inf or nan.  */
      if ((ix[0] * 2 == 0) || (ix[1] * 2 == 0))
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* log(inf) == inf.  */
      if ((ix[0] == asuint64 (INFINITY)) && (ix[1] == asuint64 (INFINITY)))
        {
          y[0] = x[0];
          y[1] = x[1];
          return y;
        }
      else if (ix[0] == asuint64 (INFINITY))
        {
          y[0] = x[0];
          y[1] = log (x[1]);
          return y;
        }
      else if (ix[1] == asuint64 (INFINITY))
        {
          y[0] = log (x[0]);
          y[1] = x[1];
          return y;
        }

      if (((top[0] & 0x8000) || (top[0] & 0x7ff0) == 0x7ff0)
          || ((top[1] & 0x8000) || (top[1] & 0x7ff0) == 0x7ff0))
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* x is subnormal, normalize it.  */
      ix[0] = asuint64 (x[0] * 0x1p52);
      ix[0] -= 52ULL << 52;
      ix[1] = asuint64 (x[1] * 0x1p52);
      ix[1] -= 52ULL << 52;
    }
  else if (__glibc_unlikely (top[0] - 0x0010 >= 0x7ff0 - 0x0010))
    {
      /* x < 0x1p-1022 or inf or nan.  */
      if (ix[0] * 2 == 0)
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* log(inf) == inf.  */
      if (ix[0] == asuint64 (INFINITY))
        {
          y[0] = x[0];
          y[1] = log (x[1]);
          return y;
        }

      if (((top[0] & 0x8000) || (top[0] & 0x7ff0) == 0x7ff0))
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* x is subnormal, normalize it.  */
      ix[0] = asuint64 (x[0] * 0x1p52);
      ix[0] -= 52ULL << 52;
    }
  else if (__glibc_unlikely (top[1] - 0x0010 >= 0x7ff0 - 0x0010))
    {
      /* x < 0x1p-1022 or inf or nan.  */
      if (ix[1] * 2 == 0)
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* log(inf) == inf.  */
      if (ix[1] == asuint64 (INFINITY))
        {
          y[0] = log (x[0]);
          y[1] = x[1];
          return y;
        }

      if (((top[1] & 0x8000) || (top[1] & 0x7ff0) == 0x7ff0))
        {
          y[0] = log (x[0]);
          y[1] = log (x[1]);
          return y;
        }

      /* x is subnormal, normalize it.  */
      ix[1] = asuint64 (x[1] * 0x1p52);
      ix[1] -= 52ULL << 52;
    }

  /* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - (vector unsigned long long) {OFF, OFF};
  i = (vector signed long)
      (tmp >> vec_splats ((unsigned long) (52 - LOG_TABLE_BITS)))
      % vec_splats ((unsigned long) N);
  k = ((vector signed long long) tmp) >> vec_splats (52ULL);
  iz = ix - (tmp & vec_splats (0xfffULL) << vec_splats (52ULL));
  invc[0] = T[i[0]].invc;
  logc[0] = T[i[0]].logc;
  invc[1] = T[i[1]].invc;
  logc[1] = T[i[1]].logc;
  z[0] = asdouble (iz[0]);
  z[1] = asdouble (iz[1]);

  /* log(x) = log1p(z/c-1) + log(c) + k*Ln2.  */
  /* r ~= z/c - 1, |r| < 1/(2*N).  */
  /* rounding error: 0x1p-55/N + 0x1p-66.  */
  r = z * invc - (vector double) {1.0, 1.0};
  kd = vec_double (k);

  /* hi + lo = r + log(c) + k*Ln2.  */
  w = kd * Ln2hi + logc;
  hi = w + r;
  lo = w - hi + r + kd * Ln2lo;

  /* log(x) = lo + (log1p(r) - r) + hi.  */
  r2 = r * r; /* rounding error: 0x1p-54/N^2.  */
  /* Worst case error if |y| > 0x1p-4: 0.519 ULP (0.520 ULP without fma).
     0.5 + 2.06/N + abs-poly-error*2^56 ULP (+ 0.001 ULP without fma).  */
  y = lo + r2 * A[0]
      + r * r2 * (A[1] + r * A[2] + r2 * (A[3] + r * A[4])) + hi;
  return y;
}
