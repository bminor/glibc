/* Correctly-rounded cosine of binary32 value for angles in half-revolutions

Copyright (c) 2022-2025 Alexei Sibidanov.

The original version of this file was copied from the CORE-MATH
project (src/binary32/cospi/cospif.c, revision f786e13).

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
#include <libm-alias-float.h>
#include "math_config.h"

float
__cospif (float x)
{
  static const double sn[] =
    {
      0x1.921fb54442d0fp-37, -0x1.4abbce6102b94p-112, 0x1.4669fa3c58463p-189
    };
  static const double cn[] =
    {
      -0x1.3bd3cc9be45cfp-74, 0x1.03c1f08088742p-150, -0x1.55d1e5eff55a5p-228
    };
  /* S[i] approximates sin(i*pi/2^6) */
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
  int32_t m = (ix & ~0u >> 9) | 1 << 23;
  int32_t s = 143 - e;
  int32_t p = e - 112;
  if (__glibc_unlikely (p < 0)) /* |x| < 2^-15  */
    {
      uint32_t ax = ix & (~0u>>1);
      /* Warning: -0x1.3bd3ccp+2f * x underflows for |x| < 0x1.9f03p-129 */
      if (ax >= 0x19f030u)
	return fmaf (-0x1.3bd3ccp+2f * x, x, 1.0f);
      else /* |x| < 0x1.9f03p-129 */
	return fmaf (-x, x, 1.0f);
    }
  if (__glibc_unlikely (p > 31))
    {
      if (__glibc_unlikely (p > 63))
	return 1.0f;
      int32_t iq = m << (p - 32);
      return S[(iq + 32) & 127];
    }
  int32_t k = m << p;
  if (__glibc_unlikely (k == 0))
    {
      int32_t iq = m >> (32 - p);
      return S[(iq + 32) & 127];
    }
  double z = k;
  double z2 = z * z;
  double fs = sn[0] + z2 * (sn[1] + z2 * sn[2]);
  double fc = cn[0] + z2 * (cn[1] + z2 * cn[2]);
  uint32_t iq = m >> s;
  iq = (iq + 1) >> 1;
  uint32_t is = iq & 127, ic = (iq + 32) & 127;
  double ts = S[ic], tc = S[is];
  double r = ts + (ts * z2) * fc - (tc * z) * fs;
  return r;
}
libm_alias_float (__cospi, cospi)
