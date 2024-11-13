/* Correctly-rounded arc-sine function for binary32 value.

Copyright (c) 2023-2024 Alexei Sibidanov.

The original version of this file was copied from the CORE-MATH
project (file src/binary32/asin/asinf.c, revision bc385c2).

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

#include <stdint.h>
#include <errno.h>
#include <libm-alias-finite.h>
#include "math_config.h"

static __attribute__ ((noinline)) float
as_special (float x)
{
  uint32_t ax = asuint (x) << 1;
  if (ax > (0xffu << 24))
    return x + x; /* nan */
  return __math_invalidf (0.0);
}

static double
poly12 (double z, const double *c)
{
  double z2 = z * z;
  double z4 = z2 * z2;
  double c0 = c[0] + z * c[1];
  double c2 = c[2] + z * c[3];
  double c4 = c[4] + z * c[5];
  double c6 = c[6] + z * c[7];
  double c8 = c[8] + z * c[9];
  double c10 = c[10] + z * c[11];
  c0 += c2 * z2;
  c4 += c6 * z2;
  c8 += z2 * c10;
  c0 += z4 * (c4 + z4 * c8);
  return c0;
}

float
__ieee754_asinf (float x)
{
  const double pi2 = 0x1.921fb54442d18p+0;
  double xs = x;
  double r;
  uint32_t ax = asuint (x) << 1;
  if (__glibc_unlikely (ax > 0x7f << 24))
    return as_special(x);
  if (__glibc_likely (ax < 0x7ec29000u))
    {
      if (__glibc_unlikely (ax < 115 << 24))
	return fmaf (x, 0x1p-25, x);
      static const double b[] =
	{
	   0x1.0000000000005p+0,  0x1.55557aeca105dp-3,  0x1.3314ec3db7d12p-4,
	   0x1.775738a5a6f92p-5,  0x1.5d5f7ce1c8538p-8,  0x1.605c6d58740fp-2,
	  -0x1.5728b732d73c6p+1,  0x1.f152170f151ebp+3, -0x1.f962ea3ca992ep+5,
	   0x1.71971e17375ap+7,  -0x1.860512b4ba23p+8,   0x1.26a3b8d4bdb14p+9,
	  -0x1.36f2ea5698b51p+9,  0x1.b3d722aebfa2ep+8, -0x1.6cf89703b1289p+7,
	   0x1.1518af6a65e2dp+5
	};
      double z = xs;
      double z2 = z * z;
      double z4 = z2 * z2;
      double z8 = z4 * z4;
      double z16 = z8 * z8;
      r = z * ((((b[0] + z2 * b[1]) + z4 * (b[2] + z2 * b[3]))
		+ z8 * ((b[4] + z2 * b[5]) + z4 * (b[6] + z2 * b[7])))
	       + z16 * (((b[8] + z2 * b[9]) + z4 * (b[10] + z2 * b[11]))
			+ z8 * ((b[12] + z2 * b[13])
				+ z4 * (b[14] + z2 * b[15]))));
      float ub = r;
      float lb = r - z * 0x1.efa8ebp-31;
      if (ub == lb)
	return ub;
    }
  if (ax < (0x7eu << 24))
    {
      static const double c[] =
	{
	  0x1.555555555529cp-3,  0x1.333333337e0ddp-4, 0x1.6db6db3b4465ep-5,
	  0x1.f1c72e13ac306p-6,  0x1.6e89cebe06bc4p-6, 0x1.1c6dcf5289094p-6,
	  0x1.c6dbbcc7c6315p-7,  0x1.8f8dc2615e996p-7, 0x1.a5833b7bf15e8p-8,
	  0x1.43f44ace1665cp-6, -0x1.0fb17df881c73p-6, 0x1.07520c026b2d6p-5
	};
      double z = xs;
      double z2 = z * z;
      double c0 = poly12 (z2, c);
      r = z + (z * z2) * c0;
    }
  else
    {
      if (__glibc_unlikely (ax == 0x7e55688au))
	return copysignf (0x1.75b8a2p-1f, x) + copysignf (0x1p-26f, x);
      if (__glibc_unlikely (ax == 0x7e107434u))
	return copysignf (0x1.1f4b64p-1f, x) + copysignf (0x1p-26f, x);
      double bx = fabs (xs);
      double z = 1.0 - bx;
      double s = sqrt (z);
      static const double c[] =
	{
	  0x1.6a09e667f3bcbp+0,   0x1.e2b7dddff2db9p-4,  0x1.b27247ab42dbcp-6,
	  0x1.02995cc4e0744p-7,   0x1.5ffb0276ec8eap-9,  0x1.033885a928decp-10,
	  0x1.911f2be23f8c7p-12,  0x1.4c3c55d2437fdp-13, 0x1.af477e1d7b461p-15,
	  0x1.abd6bdff67dcbp-15, -0x1.1717e86d0fa28p-16, 0x1.6ff526de46023p-16
	};
      r = pi2 - s * poly12 (z, c);
      r = copysign (r, xs);
    }
  return r;
}
libm_alias_finite (__ieee754_asinf, __asinf)
