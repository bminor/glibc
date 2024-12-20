/* Correctly-rounded sine of binary32 value for angles in half-revolutions

Copyright (c) 2022-2025 Alexei Sibidanov.

The original version of this file was copied from the CORE-MATH
project (src/binary32/sinpi/sinpif.c, revision f786e13).

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
#include <errno.h>
#include <fenv.h>
#include <libm-alias-float.h>
#include "math_config.h"

float
__sinpif (float x)
{
  static const double sn[] =
    {
      0x1.921fb54442d0fp-37, -0x1.4abbce6102b94p-112, 0x1.4669fa3c58463p-189
    };
  static const double cn[] =
    {
      -0x1.3bd3cc9be45cfp-74, 0x1.03c1f08088742p-150, -0x1.55d1e5eff55a5p-228
    };
  static const double S[] =
    {
       0x0p+0,                 0x1.91f65f10dd814p-5,    0x1.917a6bc29b42cp-4,
       0x1.2c8106e8e613ap-3,   0x1.8f8b83c69a60bp-3,    0x1.f19f97b215f1bp-3,
       0x1.294062ed59f06p-2,   0x1.58f9a75ab1fddp-2,    0x1.87de2a6aea963p-2,
       0x1.b5d1009e15ccp-2,    0x1.e2b5d3806f63bp-2,    0x1.073879922ffeep-1,
       0x1.1c73b39ae68c8p-1,   0x1.30ff7fce17035p-1,    0x1.44cf325091dd6p-1,
       0x1.57d69348cecap-1,    0x1.6a09e667f3bcdp-1,    0x1.7b5df226aafafp-1,
       0x1.8bc806b151741p-1,   0x1.9b3e047f38741p-1,    0x1.a9b66290ea1a3p-1,
       0x1.b728345196e3ep-1,   0x1.c38b2f180bdb1p-1,    0x1.ced7af43cc773p-1,
       0x1.d906bcf328d46p-1,   0x1.e212104f686e5p-1,    0x1.e9f4156c62ddap-1,
       0x1.f0a7efb9230d7p-1,   0x1.f6297cff75cbp-1,     0x1.fa7557f08a517p-1,
       0x1.fd88da3d12526p-1,   0x1.ff621e3796d7ep-1,    0x1p+0,
       0x1.ff621e3796d7ep-1,   0x1.fd88da3d12526p-1,    0x1.fa7557f08a517p-1,
       0x1.f6297cff75cbp-1,    0x1.f0a7efb9230d7p-1,    0x1.e9f4156c62ddap-1,
       0x1.e212104f686e5p-1,   0x1.d906bcf328d46p-1,    0x1.ced7af43cc773p-1,
       0x1.c38b2f180bdb1p-1,   0x1.b728345196e3ep-1,    0x1.a9b66290ea1a3p-1,
       0x1.9b3e047f38741p-1,   0x1.8bc806b151741p-1,    0x1.7b5df226aafafp-1,
       0x1.6a09e667f3bcdp-1,   0x1.57d69348cecap-1,     0x1.44cf325091dd6p-1,
       0x1.30ff7fce17035p-1,   0x1.1c73b39ae68c8p-1,    0x1.073879922ffeep-1,
       0x1.e2b5d3806f63bp-2,   0x1.b5d1009e15ccp-2,     0x1.87de2a6aea963p-2,
       0x1.58f9a75ab1fddp-2,   0x1.294062ed59f06p-2,    0x1.f19f97b215f1bp-3,
       0x1.8f8b83c69a60bp-3,   0x1.2c8106e8e613ap-3,    0x1.917a6bc29b42cp-4,
       0x1.91f65f10dd814p-5,   0x0p+0,                 -0x1.91f65f10dd814p-5,
      -0x1.917a6bc29b42cp-4,  -0x1.2c8106e8e613ap-3,   -0x1.8f8b83c69a60bp-3,
      -0x1.f19f97b215f1bp-3,  -0x1.294062ed59f06p-2,   -0x1.58f9a75ab1fddp-2,
      -0x1.87de2a6aea963p-2,  -0x1.b5d1009e15ccp-2,    -0x1.e2b5d3806f63bp-2,
      -0x1.073879922ffeep-1,  -0x1.1c73b39ae68c8p-1,   -0x1.30ff7fce17035p-1,
      -0x1.44cf325091dd6p-1,  -0x1.57d69348cecap-1,    -0x1.6a09e667f3bcdp-1,
      -0x1.7b5df226aafafp-1,  -0x1.8bc806b151741p-1,   -0x1.9b3e047f38741p-1,
      -0x1.a9b66290ea1a3p-1,  -0x1.b728345196e3ep-1,   -0x1.c38b2f180bdb1p-1,
      -0x1.ced7af43cc773p-1,  -0x1.d906bcf328d46p-1,   -0x1.e212104f686e5p-1,
      -0x1.e9f4156c62ddap-1,  -0x1.f0a7efb9230d7p-1,   -0x1.f6297cff75cbp-1,
      -0x1.fa7557f08a517p-1,  -0x1.fd88da3d12526p-1,   -0x1.ff621e3796d7ep-1,
      -0x1p+0,                -0x1.ff621e3796d7ep-1,   -0x1.fd88da3d12526p-1,
      -0x1.fa7557f08a517p-1,  -0x1.f6297cff75cbp-1,    -0x1.f0a7efb9230d7p-1,
      -0x1.e9f4156c62ddap-1,  -0x1.e212104f686e5p-1,   -0x1.d906bcf328d46p-1,
      -0x1.ced7af43cc773p-1,  -0x1.c38b2f180bdb1p-1,   -0x1.b728345196e3ep-1,
      -0x1.a9b66290ea1a3p-1,  -0x1.9b3e047f38741p-1,   -0x1.8bc806b151741p-1,
      -0x1.7b5df226aafafp-1,  -0x1.6a09e667f3bcdp-1,   -0x1.57d69348cecap-1,
      -0x1.44cf325091dd6p-1,  -0x1.30ff7fce17035p-1,   -0x1.1c73b39ae68c8p-1,
      -0x1.073879922ffeep-1,  -0x1.e2b5d3806f63bp-2,   -0x1.b5d1009e15ccp-2,
      -0x1.87de2a6aea963p-2,  -0x1.58f9a75ab1fddp-2,   -0x1.294062ed59f06p-2,
      -0x1.f19f97b215f1bp-3,  -0x1.8f8b83c69a60bp-3,   -0x1.2c8106e8e613ap-3,
      -0x1.917a6bc29b42cp-4,  -0x1.91f65f10dd814p-5
    };

  uint32_t ix = asuint (x);
  int32_t e = (ix >> 23) & 0xff;
  if (__glibc_unlikely (e == 0xff))
    {
      if (!(ix << 9))
        return __math_invalidf (x);
      return x + x; /* nan */
    }
  int32_t m = (ix & ~0u >> 9) | 1 << 23, sgn = ix;
  sgn >>= 31;
  m = (m ^ sgn) - sgn;
  int32_t s = 143 - e;
  if (__glibc_unlikely (s < 0))
    {
      if (__glibc_unlikely (s < -6))
	return copysignf (0.0f, x);
      int32_t iq = m << (-s - 1);
      iq &= 127;
      if (iq == 0 || iq == 64)
	return copysignf (0.0f, x);
      return S[iq];
    }
  else if (__glibc_unlikely (s > 30))
    {
      double z = x, z2 = z * z;
      return z * (0x1.921fb54442d18p+1 + z2 * (-0x1.4abbce625be53p+2));
    }
  int32_t si = 25 - s;
  if (__glibc_unlikely (si >= 0 && (m << si) == 0))
    return copysignf (0.0f, x);

  int32_t k = m << (31 - s);
  double z = k, z2 = z * z;
  double fs = sn[0] + z2 * (sn[1] + z2 * sn[2]);
  double fc = cn[0] + z2 * (cn[1] + z2 * cn[2]);
  uint32_t iq = m >> s;
  iq = (iq + 1) >> 1;
  uint32_t is = iq & 127, ic = (iq + 32) & 127;
  double ts = S[is], tc = S[ic];
  double r = ts + (ts * z2) * fc + (tc * z) * fs;
  return r;
}
libm_alias_float (__sinpi, sinpi)
