/* Correctly-rounded hyperbolic sine function for binary32 value.

Copyright (c) 2022-2024 Alexei Sibidanov.

The original version of this file was copied from the CORE-MATH
project (file src/binary32/sinh/sinhf.c, revision 572ecec).

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

#include <math.h>
#include <stdint.h>
#include <libm-alias-finite.h>
#include <math-narrow-eval.h>
#include "math_config.h"

float
__ieee754_sinhf (float x)
{
  static const double c[] =
    {
      1, 0x1.62e42fef4c4e7p-6, 0x1.ebfd1b232f475p-13, 0x1.c6b19384ecd93p-20
    };
  static const double ch[] =
    {
      1,                     0x1.62e42fefa39efp-6,  0x1.ebfbdff82c58fp-13,
      0x1.c6b08d702e0edp-20, 0x1.3b2ab6fb92e5ep-27, 0x1.5d886e6d54203p-35,
      0x1.430976b8ce6efp-43
    };
  static const uint64_t tb[] =
    {
      0x3fe0000000000000, 0x3fe059b0d3158574, 0x3fe0b5586cf9890f,
      0x3fe11301d0125b51, 0x3fe172b83c7d517b, 0x3fe1d4873168b9aa,
      0x3fe2387a6e756238, 0x3fe29e9df51fdee1, 0x3fe306fe0a31b715,
      0x3fe371a7373aa9cb, 0x3fe3dea64c123422, 0x3fe44e086061892d,
      0x3fe4bfdad5362a27, 0x3fe5342b569d4f82, 0x3fe5ab07dd485429,
      0x3fe6247eb03a5585, 0x3fe6a09e667f3bcd, 0x3fe71f75e8ec5f74,
      0x3fe7a11473eb0187, 0x3fe82589994cce13, 0x3fe8ace5422aa0db,
      0x3fe93737b0cdc5e5, 0x3fe9c49182a3f090, 0x3fea5503b23e255d,
      0x3feae89f995ad3ad, 0x3feb7f76f2fb5e47, 0x3fec199bdd85529c,
      0x3fecb720dcef9069, 0x3fed5818dcfba487, 0x3fedfc97337b9b5f,
      0x3feea4afa2a490da, 0x3fef50765b6e4540
    };
  static const struct
  {
    uint32_t uarg;
    float rh, rl;
  } st = { 0x74250bfeu, 0x1.250bfep-11, 0x1p-36 };
  const double iln2 = 0x1.71547652b82fep+5;
  double z = x;
  uint32_t ux = asuint (x) << 1;
  if (__glibc_unlikely (ux > 0x8565a9f8u))
    { /* |x| >~ 89.4 */
      float sgn = copysignf (2.0f, x);
      if (ux >= 0xff000000u)
	{
	  if (ux << 8)
	    return x + x;			    /* nan */
	  return copysignf (INFINITY, x); /* +-inf */
	}
      float r = math_narrow_eval (sgn * 0x1.fffffep127f);
      return r;
    }
  if (__glibc_unlikely (ux < 0x7c000000u))
    { /* |x| < 0.125 */
      if (__glibc_unlikely (ux <= 0x74250bfeu))
	{					   /* |x| <= 0x1.250bfep-11 */
	  if (__glibc_unlikely (ux < 0x66000000u)) /* |x| < 0x1p-24 */
	    return fmaf (x, fabsf (x), x);
	  if (__glibc_unlikely (st.uarg == ux))
	    {
	      float sgn = copysignf (1.0f, x);
	      return sgn * st.rh + sgn * st.rl;
	    }
	  return (x * 0x1.555556p-3f) * (x * x) + x;
	}
      static const double cp[] =
	{
	  0x1.5555555555555p-3,  0x1.11111111146e1p-7,
	  0x1.a01a00930dda6p-13, 0x1.71f92198aa6e9p-19
	};
      double z2 = z * z;
      double z4 = z2 * z2;
      return z + (z2 * z)
	* ((cp[0] + z2 * cp[1]) + z4 * (cp[2] + z2 * (cp[3])));
    }
  double a = iln2 * z;
  double ia = roundeven_finite (a);
  double h = a - ia;
  double h2 = h * h;
  int64_t jp = asuint64 (ia + 0x1.8p52);
  int64_t jm = -jp;
  double sp = asdouble (tb[jp & 31] + ((jp >> 5) << 52));
  double sm = asdouble (tb[jm & 31] + ((jm >> 5) << 52));
  double te = c[0] + h2 * c[2];
  double to = (c[1] + h2 * c[3]);
  double rp = sp * (te + h * to);
  double rm = sm * (te - h * to);
  double r = rp - rm;
  float ub = r;
  double lb = r - 1.52e-10 * r;
  if (__glibc_unlikely (ub != lb))
    {
      const double iln2h = 0x1.7154765p+5;
      const double iln2l = 0x1.5c17f0bbbe88p-26;
      h = (iln2h * z - ia) + iln2l * z;
      h2 = h * h;
      te = ch[0] + h2 * ch[2] + (h2 * h2) * (ch[4] + h2 * ch[6]);
      to = ch[1] + h2 * (ch[3] + h2 * ch[5]);
      r = sp * (te + h * to) - sm * (te - h * to);
      ub = r;
    }
  return ub;
}
libm_alias_finite (__ieee754_sinhf, __sinhf)
