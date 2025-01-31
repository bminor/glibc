/* Correctly-rounded biased argument base-10 logarithm function for binary32 value.

Copyright (c) 2022-2023 Alexei Sibidanov.

This file is part of the CORE-MATH project
project (file src/binary32/log10p1/log10p1f.c revision bc385c2).

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
#include <fenv.h>
#include <libm-alias-float.h>
#include "math_config.h"

static __attribute__ ((noinline)) float
as_special (float x)
{
  uint32_t ux = asuint (x);
  if (ux == 0x7f800000u)
    return x; /* +inf */
  uint32_t ax = ux << 1;
  if (ax == 0x17fu << 24)
    /* x+1 = 0.0 */
    return __math_divzerof (1);
  if (ax > 0xff000000u)
    return x + x; /* nan */
  return  __math_invalidf (x);
}

float
__log10p1f (float x)
{
  static const double tr[] =
    {
      0x1p+0,         0x1.f81f82p-1,  0x1.f07c1fp-1,  0x1.e9131acp-1,
      0x1.e1e1e1ep-1, 0x1.dae6077p-1, 0x1.d41d41dp-1, 0x1.cd85689p-1,
      0x1.c71c71cp-1, 0x1.c0e0704p-1, 0x1.bacf915p-1, 0x1.b4e81b5p-1,
      0x1.af286bdp-1, 0x1.a98ef6p-1,  0x1.a41a41ap-1, 0x1.9ec8e95p-1,
      0x1.999999ap-1, 0x1.948b0fdp-1, 0x1.8f9c19p-1,  0x1.8acb90fp-1,
      0x1.8618618p-1, 0x1.8181818p-1, 0x1.7d05f41p-1, 0x1.78a4c81p-1,
      0x1.745d174p-1, 0x1.702e05cp-1, 0x1.6c16c17p-1, 0x1.6816817p-1,
      0x1.642c859p-1, 0x1.605816p-1,  0x1.5c9882cp-1, 0x1.58ed231p-1,
      0x1.5555555p-1, 0x1.51d07ebp-1, 0x1.4e5e0a7p-1, 0x1.4afd6ap-1,
      0x1.47ae148p-1, 0x1.446f865p-1, 0x1.4141414p-1, 0x1.3e22cbdp-1,
      0x1.3b13b14p-1, 0x1.3813814p-1, 0x1.3521cfbp-1, 0x1.323e34ap-1,
      0x1.2f684bep-1, 0x1.2c9fb4ep-1, 0x1.29e412ap-1, 0x1.27350b9p-1,
      0x1.2492492p-1, 0x1.21fb781p-1, 0x1.1f7047ep-1, 0x1.1cf06aep-1,
      0x1.1a7b961p-1, 0x1.1811812p-1, 0x1.15b1e5fp-1, 0x1.135c811p-1,
      0x1.1111111p-1, 0x1.0ecf56cp-1, 0x1.0c9715p-1,  0x1.0a6810ap-1,
      0x1.0842108p-1, 0x1.0624dd3p-1, 0x1.041041p-1,  0x1.0204081p-1,
      0.5
    };
  static const double tl[] =
    {
     -0x1.562ec497ef351p-43, 0x1.b9476892ea99cp-8, 0x1.b5e909c959eecp-7,
      0x1.45f4f59ec84fp-6,   0x1.af5f92cbcf2aap-6, 0x1.0ba01a6069052p-5,
      0x1.3ed119b99dd41p-5,  0x1.714834298a088p-5, 0x1.a30a9d98309c1p-5,
      0x1.d41d51266b9d9p-5,  0x1.02428c0f62dfcp-4, 0x1.1a23444eea521p-4,
      0x1.31b30543f2597p-4,  0x1.48f3ed39bd5e7p-4, 0x1.5fe8049a0bd06p-4,
      0x1.769140a6a78eap-4,  0x1.8cf1836c96595p-4, 0x1.a30a9d5551a84p-4,
      0x1.b8de4d1ee5b21p-4,  0x1.ce6e4202c7bc9p-4, 0x1.e3bc1accaa6eap-4,
      0x1.f8c9683b584b7p-4,  0x1.06cbd68ca86ep-3,  0x1.11142f19de3a2p-3,
      0x1.1b3e71fa795fp-3,   0x1.254b4d37a3354p-3, 0x1.2f3b6912cab79p-3,
      0x1.390f6831144f7p-3,  0x1.42c7e7fffb21ap-3, 0x1.4c65808c779aep-3,
      0x1.55e8c507508c7p-3,  0x1.5f52445deb049p-3, 0x1.68a288c3efe72p-3,
      0x1.71da17bdef98bp-3,  0x1.7af9736089c4bp-3, 0x1.84011952a11ebp-3,
      0x1.8cf1837a7d6d1p-3,  0x1.95cb2891e3048p-3, 0x1.9e8e7b0f85651p-3,
      0x1.a73beaa5d9dfep-3,  0x1.afd3e39454544p-3, 0x1.b856cf060c662p-3,
      0x1.c0c5134de0c6dp-3,  0x1.c91f1371bb611p-3, 0x1.d1652ffcd2bc5p-3,
      0x1.d997c6f634ae6p-3,  0x1.e1b733ab8fbadp-3, 0x1.e9c3ceadab4c8p-3,
      0x1.f1bdeec438f77p-3,  0x1.f9a5e7a5f906fp-3, 0x1.00be05ac02564p-2,
      0x1.04a054d81990cp-2,  0x1.087a083594e33p-2, 0x1.0c4b457098b4fp-2,
      0x1.101431aa1f48ap-2,  0x1.13d4f08b98411p-2, 0x1.178da53edaecbp-2,
      0x1.1b3e71e9f9391p-2,  0x1.1ee777defd526p-2, 0x1.2288d7b48d874p-2,
      0x1.2622b0f52dad8p-2,  0x1.29b522a4c594cp-2, 0x1.2d404b0e305b9p-2,
      0x1.30c4478f3f21dp-2,  0x1.34413509f6f4dp-2
    };
  static const union
  {
    float f;
    uint32_t u;
  } st[] =
  {
    { 0x0p+0 },        { 0x1.2p+3 },      { 0x1.8cp+6 },
    { 0x1.f38p+9 },    { 0x1.3878p+13 },  { 0x1.869fp+16 },
    { 0x1.e847ep+19 }, { 0x1.312cfep+23 }
  };
  double z = x;
  uint32_t ux = asuint (x);
  if (__glibc_unlikely (ux >= 0x17fu << 23)) /* x <= -1 */
    return as_special (x);
  uint32_t ax = ux & (~0u >> 1);
  if (__glibc_unlikely (ax == 0))
    return copysign (0, x);
  if (__glibc_unlikely (ax >= (0xff << 23))) /* +inf, nan */
    return as_special (x);
  int ie = ux;
  ie >>= 23;
  unsigned int je = ie - 126;
  je = (je * 0x9a209a8) >> 29;
  if (__glibc_unlikely (ux == st[je].u))
    return je;

  uint64_t tz = asuint64 (z + 1.0);
  uint64_t m = tz & (~(uint64_t) 0 >> 12);
  int32_t e = (tz >> 52) - 1023, j = ((m + ((int64_t) 1 << 45)) >> 46);
  tz = m | ((uint64_t) 0x3ff << 52);
  double ix = tr[j], l = tl[j];
  double off = e * 0x1.34413509f79ffp-2 + l;
  double v = asdouble (tz) * ix - 1;

  static const double h[] =
    {
      0x1.bcb7b150bf6d8p-2, -0x1.bcb7b1738c07ep-3,
      0x1.287de19e795c5p-3, -0x1.bca44edc44bc4p-4
    };
  double v2 = v * v;
  double f = (h[0] + v * h[1]) + v2 * (h[2] + v * h[3]);
  double r = off + v * f;
  float ub = r;
  float lb = r + 0x1.5cp-42;
  if (__glibc_unlikely (ub != lb))
    {
      if (__glibc_unlikely (ax < 0x3d32743eu))
	{ /* 0x1.64e87cp-5f */
	  if (__glibc_unlikely (ux == 0xa6aba8afu))
	    return -0x1.2a33bcp-51f + 0x1p-76f;
	  if (__glibc_unlikely (ux == 0xaf39b9a7u))
	    return -0x1.42a342p-34f + 0x1p-59f;
	  if (__glibc_unlikely (ux == 0x399a7c00u))
	    return 0x1.0c53cap-13f + 0x1p-38f;
	  z /= 2.0 + z;
	  double z2 = z * z, z4 = z2 * z2;
	  static const double c[] =
	    {
	      0x1.bcb7b1526e50fp-1, 0x1.287a76370129dp-2,
	      0x1.63c62378fa3dbp-3, 0x1.fca4139a42374p-4
	    };
	  float ret = z * ((c[0] + z2 * c[1]) + z4 * (c[2] + z2 * c[3]));
	  if (x != 0.0f && ret == 0.0)
	    __set_errno (ERANGE);
	  return ret;
	}
      if (__glibc_unlikely (ux == 0x7956ba5eu))
	return 0x1.16bebap+5f + 0x1p-20f;
      if (__glibc_unlikely (ux == 0xbd86ffb9u))
	return -0x1.e53536p-6f + 0x1p-31f;
      static const double c[] =
	{
	  0x1.bcb7b1526e50ep-2,  -0x1.bcb7b1526e53dp-3, 0x1.287a7636f3fa2p-3,
	  -0x1.bcb7b146a14b3p-4,  0x1.63c627d5219cbp-4, -0x1.2880736c8762dp-4,
	  0x1.fc1ecf913961ap-5
	};
      f = v
	  * ((c[0] + v * c[1])
	     + v2 * ((c[2] + v * c[3]) + v2 * (c[4] + v * c[5] + v2 * c[6])));
      f += l - tl[0];
      double el = e * 0x1.34413509f79ffp-2;
      r = el + f;
      ub = r;
    }
  return ub;
}
libm_alias_float (__log10p1, log10p1)
