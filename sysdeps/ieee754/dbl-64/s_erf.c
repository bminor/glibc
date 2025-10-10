/* Correctly-rounded error function for binary64 value.

Copyright (c) 2023-2025 Paul Zimmermann

The original version of this file was copied from the CORE-MATH
project (file src/binary64/erf/erf.c, revision 384ed01d).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <array_length.h>
#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <libm-alias-double.h>
#include "math_config.h"
#include "s_erf_data.h"

/* CH+CL is a double-double approximation of 2/sqrt(pi) to nearest */
static const double CH = 0x1.20dd750429b6dp+0;
static const double CL = 0x1.1ae3a914fed8p-56;

/* Add a + b, such that *hi + *lo approximates a + b.
   Assumes |a| >= |b|.
   For rounding to nearest we have hi + lo = a + b exactly.
   For directed rounding, we have
   (a) hi + lo = a + b exactly when the exponent difference between a and b
       is at most 53 (the binary64 precision)
   (b) otherwise |(a+b)-(hi+lo)| <= 2^-105 min(|a+b|,|hi|)
       (see https://hal.inria.fr/hal-03798376)
   We also have |lo| < ulp(hi). */
static inline void
fast_two_sum (double *hi, double *lo, double a, double b)
{
  double e;

  *hi = a + b;
  e = *hi - a; /* exact */
  *lo = b - e; /* exact */
}

/* Reference: https://hal.science/hal-01351529v3/document */
static inline void
two_sum (double *hi, double *lo, double a, double b)
{
  *hi = a + b;
  double aa = *hi - b;
  double bb = *hi - aa;
  double da = a - aa;
  double db = b - bb;
  *lo = da + db;
}

// Multiply exactly a and b, such that *hi + *lo = a * b.
static inline void
a_mul (double *hi, double *lo, double a, double b)
{
  *hi = a * b;
  *lo = fma (a, b, -*hi);
}

/* Assuming 0 <= z <= 0x1.7afb48dc96626p+2, put in h+l an approximation
   of erf(z). Return err the maximal relative error:
   |(h + l)/erf(z) - 1| < err*|h+l| */
static double
cr_erf_fast (double *h, double *l, double z)
{
  double th, tl;
  /* we split [0,0x1.7afb48dc96626p+2] into intervals i/16 <= z < (i+1)/16,
     and for each interval, we use a minimax polynomial:
     * for i=0 (0 <= z < 1/16) we use a polynomial evaluated at zero,
       since if we evaluate in the middle 1/32, we will get bad accuracy
       for tiny z, and moreover z-1/32 might not be exact
     * for 1 <= i <= 94, we use a polynomial evaluated in the middle of
       the interval, namely i/16+1/32
  */
  if (z < 0.0625) /* z < 1/16 */
    {
      double z2h, z2l, z4;
      a_mul (&z2h, &z2l, z, z);
      z4 = z2h * z2h;
      double c9 = fma (C0[7], z2h, C0[6]);
      double c5 = fma (C0[5], z2h, C0[4]);
      c5 = fma (c9, z4, c5);
      /* compute C0[2] + C0[3] + z2h*c5 */
      a_mul (&th, &tl, z2h, c5);
      fast_two_sum (h, l, C0[2], th);
      *l += tl + C0[3];
      /* compute C0[0] + C0[1] + (z2h + z2l)*(h + l) */
      double h_copy = *h;
      a_mul (&th, &tl, z2h, *h);
      tl += fma (z2h, *l, C0[1]);
      fast_two_sum (h, l, C0[0], th);
      *l += fma (z2l, h_copy, tl);
      /* multiply (h,l) by z */
      a_mul (h, &tl, *h, z);
      *l = fma (*l, z, tl);
      return 0x1.78p-69; /* err < 2.48658249618372e-21, cf Analyze0() */
    }
  double v = floor (16.0 * z);
  uint32_t i = 16.0 * z;
  /* i/16 <= z < (i+1)/16 */
  /* For 0.0625 0 <= z <= 0x1.7afb48dc96626p+2, z - 0.03125 is exact:
     (1) either z - 0.03125 is in the same binade as z, then 0.03125 is
	 an integer multiple of ulp(z), so is z - 0.03125
     (2) if z - 0.03125 is in a smaller binade, both z and 0.03125 are
	 integer multiple of the ulp() of that smaller binade.
     Also, subtracting 0.0625 * v is exact. */
  z = (z - 0.03125) - 0.0625 * v;
  /* now |z| <= 1/32 */
  const double *c = C[i - 1];
  double z2 = z * z, z4 = z2 * z2;
  /* the degree-10 coefficient is c[12] */
  double c9 = fma (c[12], z, c[11]);
  double c7 = fma (c[10], z, c[9]);
  double c5 = fma (c[8], z, c[7]);
  double c3h, c3l;
  /* c3h, c3l <- c[5] + z*c[6] */
  fast_two_sum (&c3h, &c3l, c[5], z * c[6]);
  c7 = fma (c9, z2, c7);
  /* c3h, c3l <- c3h, c3l + c5*z2 */
  fast_two_sum (&c3h, &tl, c3h, c5 * z2);
  c3l += tl;
  /* c3h, c3l <- c3h, c3l + c7*z4 */
  fast_two_sum (&c3h, &tl, c3h, c7 * z4);
  c3l += tl;
  /* c2h, c2l <- c[4] + z*(c3h + c3l) */
  double c2h, c2l;
  a_mul (&th, &tl, z, c3h);
  fast_two_sum (&c2h, &c2l, c[4], th);
  c2l += fma (z, c3l, tl);
  /* compute c[2] + c[3] + z*(c2h + c2l) */
  a_mul (&th, &tl, z, c2h);
  fast_two_sum (h, l, c[2], th);
  *l += tl + fma (z, c2l, c[3]);
  /* compute c[0] + c[1] + z*(h + l) */
  a_mul (&th, &tl, z, *h);
  tl = fma (z, *l, tl); /* tl += z*l */
  fast_two_sum (h, l, c[0], th);
  *l += tl + c[1];
  return 0x1.11p-69; /* err < 1.80414390200020e-21, cf analyze_p(1)
			(larger values of i yield smaller error bounds) */
}

/* for |z| < 1/8, assuming z >= 2^-61, thus no underflow can occur */
static void
cr_erf_accurate_tiny (double *h, double *l, double z)
{
  uint64_t i, j, k;
  /* use dichotomy */
  for (i = 0, j = EXCEPTIONS_LEN; i + 1 < j;)
    {
      k = (i + j) / 2;
      if (EXCEPTIONS_TINY[k][0] <= z)
	i = k;
      else
	j = k;
    }
  /* Either i=0 and z < exceptions[i+1][0],
     or i=n-1 and exceptions[i][0] <= z
     or 0 < i < n-1 and exceptions[i][0] <= z < exceptions[i+1][0].
     In all cases z can only equal exceptions[i][0]. */
  if (z == EXCEPTIONS_TINY[i][0])
    {
      *h = EXCEPTIONS_TINY[i][1];
      *l = EXCEPTIONS_TINY[i][2];
      return;
    }

  double z2 = z * z, th, tl;
  *h = P[21 / 2 + 4]; /* degree 21 */
  for (int a = 19; a > 11; a -= 2)
    *h = fma (*h, z2, P[a / 2 + 4]); /* degree j */
  *l = 0;
  for (int a = 11; a > 7; a -= 2)
    {
      /* multiply h+l by z^2 */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      a_mul (h, l, th, z);
      *l = fma (tl, z, *l);
      /* add p[j] to h + l */
      fast_two_sum (h, &tl, P[a / 2 + 4], *h);
      *l += tl;
    }
  for (int a = 7; a >= 1; a -= 2)
    {
      /* multiply h+l by z^2 */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      a_mul (h, l, th, z);
      *l = fma (tl, z, *l);
      fast_two_sum (h, &tl, P[a - 1], *h);
      *l += P[a] + tl;
    }
  /* multiply by z */
  a_mul (h, &tl, *h, z);
  *l = fma (*l, z, tl);
  return;
}

/* Assuming 0 <= z <= 0x1.7afb48dc96626p+2, put in h+l an accurate
   approximation of erf(z).
   Assumes z >= 2^-61, thus no underflow can occur. */
static void
cr_erf_accurate (double *h, double *l, double z)
{
  for (int i = 0; i < EXCEPTIONS_LEN; i++)
    if (z == EXCEPTIONS[i][0])
      {
	*h = EXCEPTIONS[i][1];
	*l = EXCEPTIONS[i][2];
	return;
      }
  double th, tl;
  /* we split [0,0x1.7afb48dc96626p+2] into intervals i/8 <= z < (i+1)/8,
     and for each interval, we use a minimax polynomial:
     * for i=0 (0 <= z < 1/8) we use a polynomial evaluated at zero,
       since if we evaluate in the middle 1/16, we will get bad accuracy
       for tiny z, and moreover z-1/16 might not be exact
     * for 1 <= i <= 47, we use a polynomial evaluated in the middle of
       the interval, namely i/8+1/16
  */
  if (z < 0.125) /* z < 1/8 */
    {
      cr_erf_accurate_tiny (h, l, z);
      return;
    }
  double v = floor (8.0 * z);
  uint32_t i = 8.0 * z;
  z = (z - 0.0625) - 0.125 * v;
  /* now |z| <= 1/16 */
  const double *p = C2[i - 1];
  *h = p[26]; /* degree-18 */
  for (int j = 17; j > 10; j--)
    *h = fma (*h, z, p[8 + j]); /* degree j */
  *l = 0;
  for (int j = 10; j > 7; j--)
    {
      /* multiply h+l by z */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      /* add p[8+j] to th + tl */
      two_sum (h, l, p[8 + j], th);
      *l += tl;
    }
  for (int j = 7; j >= 0; j--)
    {
      /* multiply h+l by z */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      /* add p[2*j] + p[2*j+1] to th + tl: we use two_sum() instead of
	 fast_two_sum because for example for i=3, the coefficient of
	 degree 7 is tiny (0x1.060b78c935b8ep-13) with respect to that
	 of degree 8 (0x1.678b51a9c4b0ap-7) */
      two_sum (h, l, p[2 * j], th);
      *l += p[2 * j + 1] + tl;
    }
}

double
__erf (double x)
{
  double z = fabs (x);
  uint64_t t = asuint64 (z);
  uint64_t ux = t;
  /* erf(x) rounds to +/-1 for RNDN for |x| > 0x1.7afb48dc96626p+2 */
  // |x| > 0x1.7afb48dc96626p+2
  if (__glibc_unlikely (ux > UINT64_C(0x4017afb48dc96626)))
    {
      double os = copysign (1.0, x);
#define MASK UINT64_C(0x7ff0000000000000) // encoding of +Inf
      if (ux > MASK)
	return x + x; /* NaN */
      if (ux == MASK)
	return os; /* +/-Inf */
      return os - 0x1p-54 * os;
    }
  double h, l, err;
  /* now |x| <= 0x1.7afb48dc96626p+2 */
  if (__glibc_unlikely (z < 0x1p-61))
    {
      /* for x=-0 the code below returns +0 which is wrong */
      if (x == 0)
	return x;
      /* tiny x: erf(x) ~ 2/sqrt(pi) * x + O(x^3), where the ratio of the
	 O(x^3) term to the main term is in x^2/3, thus less than 2^-123 */
      double y = CH * x; /* tentative result */
      /* scale x by 2^106 to get out the subnormal range */
      double sx = x * 0x1p106;
      a_mul (&h, &l, CH, sx);
      l = fma (CL, sx, l);
      /* now compute the residual h + l - y */
      l += h
	   - y * 0x1p106; /* h-y*2^106 is exact since h and y are very close */
      double res = fma (l, 0x1p-106, y);
      return res;
    }
  /* now z >= 2^-61 */
  err = cr_erf_fast (&h, &l, z);
  uint64_t u = asuint64 (h), v = asuint64 (l);
  t = asuint64 (x);
  u ^= t & SIGN_MASK;
  v ^= t & SIGN_MASK;
  double uf = asdouble (u);
  double vf = asdouble (v);
  double left = uf + fma (err, -uf, vf);
  double right = uf + fma (err, uf, vf);
  if (__glibc_likely (left == right))
    return left;

  cr_erf_accurate (&h, &l, z);

  return (x >= 0) ? h + l : (-h) + (-l);
}
libm_alias_double (__erf, erf)
