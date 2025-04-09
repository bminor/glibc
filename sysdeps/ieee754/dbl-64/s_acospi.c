/* Correctly-rounded acospi of binary64 value.

Copyright (c) 2021-2023 Paul Zimmermann and Tom Hubrecht

This file is part of the CORE-MATH project
(https://core-math.gitlabpages.inria.fr/).

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

/* This implementation is derived from the CORE-MATH acos.c code
   (revision b9d8cae), using the formula acospi (x) = acos(x) / pi.
   The parts specific to acospi are marked either with "acospi_specific",
   or enclosed between "acospi_begin" and "acospi_end", to make it easier
   to maintain this code if acos is fixed or improved.
*/

#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <libm-alias-double.h>

#define CORE_MATH_SUPPORT_ERRNO

// Warning: clang also defines __GNUC__
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#pragma STDC FENV_ACCESS ON

/****************** code copied from pow.[ch] ********************************/

// Multiply exactly a and b, such that *hi + *lo = a * b.
static inline void a_mul(double *hi, double *lo, double a, double b) {
  *hi = a * b;
  *lo = __builtin_fma (a, b, -*hi);
}

// Returns (ah + al) * (bh + bl) - (al * bl)
static inline void d_mul(double *hi, double *lo, double ah, double al,
                         double bh, double bl) {
  a_mul (hi, lo, ah, bh);
  *lo = __builtin_fma (ah, bl, *lo);
  *lo = __builtin_fma (al, bh, *lo);
}

/****************** end of code copied from pow.[ch] *************************/

// acospi_begin
/* ONE_OVER_PIH + ONE_OVER_PIL is a double-double approximation of 1/pi,
   with | ONE_OVER_PIH + ONE_OVER_PIL - 1/pi | < 2^-109.523 */
#define ONE_OVER_PIH 0x1.45f306dc9c883p-2
#define ONE_OVER_PIL -0x1.6b01ec5417056p-56
// acospi_end

/* return h=sqrt(x) and l such that h+l approximates sqrt(x) for 0 <= x <= 0.25
   with relative error bounded by 2^-102.677:
   |h + l - sqrt(x)| < 2^-102.677 |h| and |h + l - sqrt(x)| < 2^-102.677 |h+l|
*/
static double
sqrt_dbl_dbl (double x, double *l)
{
  double h = __builtin_sqrt (x);
  /* h = sqrt(x) * (1 + e1) with |e1| < 2^-52
     thus h^2 = x * (1 + e2) with |e2| < 2^-50.999 */
  double e = -__builtin_fma (h, h, -x); // exact
  /* e = x - h^2 */
  *l = e / (h + h);
  /* If there is no rounding error in l = e/(2h),
     and if we denote h' = h + l, then x - h'^2 = x - (h+e/(2h))^2
     = x - h^2 - e - e^2/(4h^2) = -e^2/(4h^2).
     We have h = sqrt(x) * (1 + e1) with |e1| < 2^-52,
     which can also be written sqrt(x) = h * (1 + e1') with |e1'| < 2^-52,
     thus x = h^2 * (1 + e1')^2, and e = x - h^2 = h^2 * (2e1' + e1'^2),
     thus with no rounding error in l = e/(2h), we would have:
     |x - h'^2| <= |e1' + e1'^2/2|^2 * h^2 < 2^-103.999 h^2.
     This yields |x + h'| * |x - h'| < 2^-103.999 h^2;
     since both x and h' differ relatively from h by less than 2^-50 or less,
     |x + h'| is very close to 2|h| and we get: |x - h'| < 2^-104.998 |h|.

     We also have to take into account the rounding error in e/(2h),
     where 2h is exact. It is bounded by ulp(e/(2h)), where
     e = h^2 * (2e1' + e1'^2), thus e/(2h) = h * (e1' + e1'^2/2).
     Since |e1'| < 2^-52, |e1' + e1'^2/2| < 2^-51,
     thus |e/(2h)| < |h|*2^-51, thus ulp(e/(2h)) <= 2^-51 ulp(h):
     |l - e/(2h)| < 2^-51 ulp(h) <= 2^-103 |h| since ulp(h) <= 2^-52 |h|.

     Finally we get:
     |h + l - sqrt(x)| < (2^-104.998 + 2^-103) |h| < 2^-102.677 |h|
     (since l is tiny with respect to h, the same bound holds for |h+l|
     instead of |l|, due to the difference between 2^-104.998 + 2^-103
     and 2^-102.677).
  */
  return h;
}

/* s + t <- a + b, assuming |a| >= |b| */
static void
fast_two_sum (double *s, double *t, double a, double b)
{
  *s = a + b;
  double e = *s - a;
  *t = b - e;
}

/* h_out + l_out = (h_in + l_in) * y */
static void
mul2_1 (double *h_out, double *l_out, double h_in, double l_in, double y)
{
  a_mul (h_out, l_out, h_in, y);
  *l_out += l_in * y;
}

/* The following table was generated with
   Gen_P_aux(6,256,threshold=192,verbose=true,extra0=12,extra1=3,k1=192)
   for the first part (0 <= i < 192), and for the second part (192 <= i < 256)
   with
   Gen_P_aux(5,256,threshold=192,verbose=true,extra0=12,extra1=3,k0=192)
   For 0 <= i < 192, each entry contains 8 coefficients corresponding to
   a degree-6 polynomial and its evaluation point:
   * T[i][0] is p[0] (the upper 12 bits of the low part are zero)
   * T[i][1] is p[1] (the upper 3 bits of the low part are zero)
   * T[i][2] is p[2]
   * T[i][3] is p[3]
   * T[i][4] is p[4]
   * T[i][5] is p[5]
   * T[i][6] is p[6]
   * T[i][7] is the evaluation point (xmid).
   For 192 <= i < 256, each entry contains 7 coefficients corresponding to
   a degree-5 polynomial and its evaluation point:
   * T[i][0] is p[0]
   * T[i][1] is p[1]
   * T[i][2] is p[2]
   * T[i][3] is p[3]
   * T[i][4] is p[4]
   * T[i][5] is p[5]
   * T[i][6] is the evaluation point (xmid).
*/
static const double T[256][8] = {
  {0x1.921fb54442d18p+0, /*0x4.6989e4b05fa3p-56,*/ -0x1p+0, -0x1.c6c0b6834ba0ep-49, -0x1.555555551318ap-3, -0x1.d02e84f046fbp-28, -0x1.3330337ffdfc1p-4, -0x1.2e509a02e335bp-11, 0x0p+0}, /* i=0 72.838 */
  {0x1.909fb4b44253fp+0, -0x1.00012001e6037p+0, -0x1.8005100e28eb9p-9, -0x1.5562d594993b6p-3, -0x1.200a8c3965d9cp-9, -0x1.3355447aa843dp-4, -0x1.e026a0a5f0aefp-10, 0x1.7fffffffebc6fp-8}, /* i=1 64.518 */
  {0x1.8f9fb2a990bfbp+0, -0x1.0003200ea64c8p+0, -0x1.400bb85b9a23fp-8, -0x1.557ad73dabcdfp-3, -0x1.e030d6eae8d23p-9, -0x1.33914cbc551f1p-4, -0x1.9052bcaec1773p-9, 0x1.400000000a072p-7}, /* i=2 64.379 */
  {0x1.8e9fae1ec5e86p+0, -0x1.00062038483f3p+0, -0x1.c02029ec97416p-8, -0x1.559edca9aafc8p-3, -0x1.504305dafd9f7p-8, -0x1.33eb677d14e3dp-4, -0x1.186edf2c1538bp-8, 0x1.c0000000173dcp-7}, /* i=3 65.686 */
  {0x1.8d9fa613b89eap+0, -0x1.000a2099d022ap+0, -0x1.20222f613b541p-7, -0x1.55cee95daeca9p-3, -0x1.b08e7d9b5ed11p-8, -0x1.3463a5fffc8c4p-4, -0x1.68eb18bc28196p-8, 0x1.1ffffffff1befp-6}, /* i=4 66.481 */
  {0x1.8c9f99881e5bcp+0, -0x1.000f215747cefp+0, -0x1.603e6d387928ap-7, -0x1.560b020c3969bp-3, -0x1.088220a9b46c8p-7, -0x1.34fa1f513523fp-4, -0x1.b9aa6a457192fp-8, 0x1.600000000e7b8p-6}, /* i=5 65.068 */
  {0x1.8b9f877b8725bp+0, -0x1.0015229dc21c5p+0, -0x1.a0671142e6ff1p-7, -0x1.56532c961ca7bp-3, -0x1.38d6e3dc0130ep-7, -0x1.35aef04dfe9d2p-4, -0x1.05605e2f608a4p-7, 0x1.9ffffffff24c1p-6}, /* i=6 64.966 */
  {0x1.8a9f6eed4e15fp+0, -0x1.001c24a35f6c6p+0, -0x1.e09e5f7ed0f45p-7, -0x1.56a7700b9c222p-3, -0x1.694a48b8af916p-7, -0x1.36823bad3f08bp-4, -0x1.2e1d1964a6284p-7, 0x1.e0000000026a6p-6}, /* i=7 64.484 */
  {0x1.899f4edc9635ap+0, -0x1.002427a752d4cp+0, -0x1.10734eacbb336p-6, -0x1.5707d4adc6ep-3, -0x1.99e1157450dc9p-7, -0x1.37742a0b7a1e4p-4, -0x1.5714ee0467369p-7, 0x1.0ffffffffef23p-5}, /* i=8 64.459 */
  {0x1.889f26483e1ecp+0, -0x1.002d2bf1e8623p+0, -0x1.30a108f493ffap-6, -0x1.577463f00e4f7p-3, -0x1.caa01713d90bep-7, -0x1.3884e9f63c6e9p-4, -0x1.804e8b4c133f6p-7, 0x1.2fffffffe7e71p-5}, /* i=9 64.306 */
  {0x1.879ef42ed5a5bp+0, -0x1.003731d48c3fap+0, -0x1.50d983139a09ep-6, -0x1.57ed287a18a0ap-3, -0x1.fb8c225b0575ap-7, -0x1.39b4affcb63bp-4, -0x1.a9cfaddd34c0cp-7, 0x1.4fffffffeed7dp-5}, /* i=10 64.279 */
  {0x1.869eb78e991c6p+0, -0x1.004239a9d28aep+0, -0x1.711de29462b92p-6, -0x1.58722e29cc767p-3, -0x1.16550a5c8528dp-6, -0x1.3b03b6be3ce92p-4, -0x1.d3a6c22377945p-7, 0x1.6ffffffff6f53p-5}, /* i=11 64.782 */
  {0x1.859e6f6564ab3p+0, -0x1.004e43d5805a6p+0, -0x1.916f4e2d12b6fp-6, -0x1.590382159cab6p-3, -0x1.2eff6aa2b1545p-6, -0x1.3c723f003ff56p-4, -0x1.fdd714b7bc66cp-7, 0x1.9000000003224p-5}, /* i=12 64.33 */
  {0x1.849e1ab0aca76p+0, -0x1.005b50c4957e2p+0, -0x1.b1ceeddc970fep-6, -0x1.59a1328f0f4cap-3, -0x1.47c7aad7fcc87p-6, -0x1.3e008fbf7c925p-4, -0x1.1434d9c3786f8p-6, 0x1.b000000004788p-5}, /* i=13 64.655 */
  {0x1.839db86d7338ep+0, -0x1.006960ed574a5p+0, -0x1.d23deb0873651p-6, -0x1.5a4b4f25842f6p-3, -0x1.60b0499f2ca91p-6, -0x1.3faef648f515ap-4, -0x1.29b3d747278a6p-6, 0x1.cffffffffd82bp-5}, /* i=14 65.934 */
  {0x1.829d47983f084p+0, -0x1.007874cf5c462p+0, -0x1.f2bd709ab30f9p-6, -0x1.5b01e8a93a2d4p-3, -0x1.79bbcbe4fc61ep-6, -0x1.417dc652943adp-4, -0x1.3f6d27b2cbba7p-6, 0x1.f00000000311ep-5}, /* i=15 64.868 */
  {0x1.819cc72d142dbp+0, -0x1.00888cf398a56p+0, -0x1.09a7558fed732p-5, -0x1.5bc5112e92ac2p-3, -0x1.92ecbd6228734p-6, -0x1.436d5a16421b5p-4, -0x1.5562f384ef754p-6, 0x1.07fffffff9c8fp-4}, /* i=16 64.429 */
  {0x1.809c3627667cep+0, -0x1.0099a9ec6c1b3p+0, -0x1.19f9647301316p-5, -0x1.5c94dc119b20cp-3, -0x1.ac45b11c2bb6dp-6, -0x1.457e126dfa8e5p-4, -0x1.6b9c978907f68p-6, 0x1.18000000005e6p-4}, /* i=17 65.118 */
  {0x1.7f9b938215cf2p+0, -0x1.00abcc55b001dp+0, -0x1.2a557d0d7ef2dp-5, -0x1.5d715df9d13fp-3, -0x1.c5c941edcc181p-6, -0x1.47b056f437d6ep-4, -0x1.821e5ff7c96b4p-6, 0x1.27fffffffc329p-4}, /* i=18 65.39 */
  {0x1.7e9ade375e48ep+0, -0x1.00bef4d4c742ap+0, -0x1.3abc387636164p-5, -0x1.5e5aacde3426cp-3, -0x1.df7a130f072f6p-6, -0x1.4a049625c398ep-4, -0x1.98ec111d7ee37p-6, 0x1.3800000005926p-4}, /* i=19 64.94 */
  {0x1.7d9a1540d593ap+0, -0x1.00d32418ae534p+0, -0x1.4b2e30d33fc86p-5, -0x1.5f50e0098f737p-3, -0x1.f95ad09eb22e6p-6, -0x1.4c7b4585fc60fp-4, -0x1.b00ac8c965ef2p-6, 0x1.47fffffff9be3p-4}, /* i=20 64.892 */
  {0x1.7c99379758aa9p+0, -0x1.00e85ada0d548p+0, -0x1.5bac016a943fdp-5, -0x1.6054101f1a64ep-3, -0x1.09b71818f2959p-5, -0x1.4f14e1c4ac981p-4, -0x1.c77f8af2a9e24p-6, 0x1.580000000393cp-4}, /* i=21 67.748 */
  {0x1.7b9844330bddep+0, -0x1.00fe99db49c9ap+0, -0x1.6c3646b1ad2eep-5, -0x1.6164571f4d893p-3, -0x1.16db78b26044ep-5, -0x1.51d1eee6a4e68p-4, -0x1.df4d9ac4c196ap-6, 0x1.68000000008bep-4}, /* i=22 64.168 */
  {0x1.7a973a0b48f84p+0, -0x1.0115e1e89abf8p+0, -0x1.7ccd9e5e88401p-5, -0x1.6281d06d1305dp-3, -0x1.241bef36b774fp-5, -0x1.54b2f873a0e64p-4, -0x1.f77bf2f15ccebp-6, 0x1.7800000001812p-4}, /* i=23 65.506 */
  {0x1.79961816995f6p+0, -0x1.012e33d81d053p+0, -0x1.8d72a77823475p-5, -0x1.63ac98d33616bp-3, -0x1.3179e65b8e077p-5, -0x1.57b891a2a8474p-4, -0x1.080657ba24afap-5, 0x1.880000000b248p-4}, /* i=24 64.813 */
  {0x1.7894dd4aad755p+0, -0x1.01479089e89e7p+0, -0x1.9e26026766e75p-5, -0x1.64e4ce8a21824p-3, -0x1.3ef6cebbf9a88p-5, -0x1.5ae3558a8e26bp-4, -0x1.14855a81806f4p-5, 0x1.9800000000ff4p-4}, /* i=25 65.081 */
  {0x1.7793889c4dcbdp+0, -0x1.0161f8e827e39p+0, -0x1.aee85108bd609p-5, -0x1.662a913df44c8p-3, -0x1.4c941f2c51075p-5, -0x1.5e33e75926448p-4, -0x1.213b9f6f2e7e8p-5, 0x1.a7fffffff17b3p-4}, /* i=26 64.919 */
  {0x1.769218ff52aeap+0, -0x1.017d6de72f22bp+0, -0x1.bfba36bd89bdp-5, -0x1.677e0214e0178p-3, -0x1.5a535509624aep-5, -0x1.61aaf285d96ecp-4, -0x1.2e2b593a9222fp-5, 0x1.b7fffffffb717p-4}, /* i=27 64.544 */
  {0x1.75908d669e9ecp+0, -0x1.0199f08594facp+0, -0x1.d09c587db9e58p-5, -0x1.68df43b5d5b8dp-3, -0x1.6835f48bd262bp-5, -0x1.65492b0f68076p-4, -0x1.3b5973ea597abp-5, 0x1.c800000003818p-4}, /* i=28 64.249 */
  {0x1.748ee4c40f636p+0, -0x1.01b781cc4cd07p+0, -0x1.e18f5cea469d6p-5, -0x1.6a4e7a4f9311ep-3, -0x1.763d8920d31cdp-5, -0x1.690f4db6cd4p-4, -0x1.48c8796c0bac3p-5, 0x1.d7fffffffdaabp-4}, /* i=29 64.273 */
  {0x1.738d1e087244bp+0, -0x1.01d622cec2209p+0, -0x1.f293ec5fd42ddp-5, -0x1.6bcbcba005163p-3, -0x1.846ba5c215174p-5, -0x1.6cfe2040ea4a9p-4, -0x1.567b0b884be47p-5, 0x1.e800000002cdp-4}, /* i=30 64.306 */
  {0x1.728b38237fc97p+0, -0x1.01f5d4aaf412dp+0, -0x1.01d55884965c6p-4, -0x1.6d575efbf886p-3, -0x1.92c1e550e9df8p-5, -0x1.711671b94ba69p-4, -0x1.6473d3e183917p-5, 0x1.f7ffffffed1edp-4}, /* i=31 66.891 */
  {0x1.71893203c6f8fp+0, -0x1.0216988994331p+0, -0x1.0a6a2b7997277p-4, -0x1.6ef15d573fdc2p-3, -0x1.a141eaf52c28fp-5, -0x1.75591abb938bbp-4, -0x1.72b5160a4507bp-5, 0x1.03fffffffc5c1p-3}, /* i=32 64.333 */
  {0x1.70870a96adc0dp+0, -0x1.02386f9e23cdp+0, -0x1.1308c60fda193p-4, -0x1.7099f14d18a22p-3, -0x1.afed627bea996p-5, -0x1.79c6fdbcfdd48p-4, -0x1.81450b731a9e9p-5, 0x1.0c00000009394p-3}, /* i=33 64.578 */
  {0x1.6f84c0c86028fp+0, -0x1.025b5b271497p+0, -0x1.1bb1804d0488ep-4, -0x1.725147290d179p-3, -0x1.bec600bf2023cp-5, -0x1.7e61075b239fp-4, -0x1.902599f661ce5p-5, 0x1.140000001afe2p-3}, /* i=34 64.402 */
  {0x1.6e825383cd1afp+0, -0x1.027f5c6de8d69p+0, -0x1.2464b3456dc13p-4, -0x1.74178cf024fa7p-3, -0x1.cdcd840ab60ebp-5, -0x1.83282eb1ebe64p-4, -0x1.9f58b59a7499p-5, 0x1.1bfffffff93efp-3}, /* i=35 65.679 */
  {0x1.6d7fc1b280c44p+0, -0x1.02a474c7599bp+0, -0x1.2d22b9277965ep-4, -0x1.75ecf26abbb0ap-3, -0x1.dd05b48621b58p-5, -0x1.881d75af61cedp-4, -0x1.aee68b1af19f9p-5, 0x1.240000000724p-3}, /* i=36 64.691 */
  {0x1.6c7d0a3cc04edp+0, -0x1.02caa59375263p+0, -0x1.35ebed44f8951p-4, -0x1.77d1a92e4f5e1p-3, -0x1.ec7064a7101a2p-5, -0x1.8d41e972e5acp-4, -0x1.becc69d166a21p-5, 0x1.2c0000000e626p-3}, /* i=37 64.402 */
  {0x1.6b7a2c0965d62p+0, -0x1.02f1f03dc7808p+0, -0x1.3ec0ac1eecacp-4, -0x1.79c5e4a82f08bp-3, -0x1.fc0f719b5a977p-5, -0x1.9296a2aab38d5p-4, -0x1.cf139a963c96ep-5, 0x1.340000000273p-3}, /* i=38 65.241 */
  {0x1.6a7725fdd5bc9p+0, -0x1.031a563d81032p+0, -0x1.47a15370afc09p-4, -0x1.7bc9da28716bfp-3, -0x1.05f261e2d7b9fp-4, -0x1.981cc5fa65a18p-4, -0x1.dfbfbba77fdf6p-5, 0x1.3bfffffff95dfp-3}, /* i=39 64.887 */
  {0x1.6973f6fdf680fp+0, -0x1.0343d9159dd8dp+0, -0x1.508e423b48b34p-4, -0x1.7dddc0ed5d97ap-3, -0x1.0df9279aafcc4p-4, -0x1.9dd58465923cp-4, -0x1.f0d2ce03a9d87p-5, 0x1.440000000f922p-3}, /* i=40 64.431 */
  {0x1.68709dec32aa8p+0, -0x1.036e7a550d25cp+0, -0x1.5987d8d0a96fep-4, -0x1.8001d22f38134p-3, -0x1.161d0a111844cp-4, -0x1.a3c21bbece4cdp-4, -0x1.012818df20cc4p-4, 0x1.4bfffffffaf31p-3}, /* i=41 64.408 */
  {0x1.676d19a94e1bcp+0, -0x1.039a3b96e0e74p+0, -0x1.628e78e0bf137p-4, -0x1.8236492cecffap-3, -0x1.1e5f0fb7dd5ddp-4, -0x1.a9e3d71bc4af6p-4, -0x1.0a1f6dd2593c5p-4, 0x1.53fffffffcdd9p-3}, /* i=42 64.977 */
  {0x1.666969147661cp+0, -0x1.03c71e8275c5fp+0, -0x1.6ba28584c398ep-4, -0x1.847b6338c4131p-3, -0x1.26c0459a58df2p-4, -0x1.b03c0f5207f82p-4, -0x1.135226a841bfcp-4, 0x1.5c00000000d8cp-3}, /* i=43 64.394 */
  {0x1.65658b0b297aep+0, -0x1.03f524cba3263p+0, -0x1.74c4634c4bf68p-4, -0x1.86d15fc5f3d4p-3, -0x1.2f41bfa442c8ep-4, -0x1.b6cc2b758d196p-4, -0x1.1cc12f7dd6dbcp-4, 0x1.6400000001feap-3}, /* i=44 66.033 */
  {0x1.64617e692a3p+0, -0x1.04245032ea9fp+0, -0x1.7df4784a2f89bp-4, -0x1.89388076a72fap-3, -0x1.37e498e85441ep-4, -0x1.bd95a160f6a9dp-4, -0x1.26704ce77d56bp-4, 0x1.6c00000003b2ap-3}, /* i=45 64.887 */
  {0x1.635d420875812p+0, -0x1.0454a285a8d14p+0, -0x1.87332c21b99f1p-4, -0x1.8bb1092a9361dp-3, -0x1.40a9f3eceb242p-4, -0x1.c499f6433ff6p-4, -0x1.30627a5c5d56cp-4, 0x1.74000000006bap-3}, /* i=46 64.227 */
  {0x1.6258d4c132bacp+0, -0x1.04861d9e48dbdp+0, -0x1.9080e81462ee2p-4, -0x1.8e3b400e33532p-3, -0x1.4992fafb1c3e8p-4, -0x1.cbdabf336ef02p-4, -0x1.3a97b45c4c021p-4, 0x1.7c00000001038p-3}, /* i=47 64.794 */
  {0x1.61543569aaa93p+0, -0x1.04b8c364782cep+0, -0x1.99de170fa5909p-4, -0x1.90d76daa903edp-3, -0x1.52a0e06c1e2d6p-4, -0x1.d359a1cdc2e3ap-4, -0x1.4515a21e5983cp-4, 0x1.83fffffffa6dfp-3}, /* i=48 64.796 */
  {0x1.604f62d634e06p+0, -0x1.04ec95cd5deeep+0, -0x1.a34b25bb8a91fp-4, -0x1.9385dcf5c7ab9p-3, -0x1.5bd4df01d79e5p-4, -0x1.db1854d5fd617p-4, -0x1.4fdd284dce08ap-4, 0x1.8bfffffff7cfbp-3}, /* i=49 64.788 */
  {0x1.5f4a5bd92e0a3p+0, -0x1.052196dbd285ap+0, -0x1.acc8828947cc5p-4, -0x1.9646db6425e44p-3, -0x1.65303a3a304dbp-4, -0x1.e318a0e3e743ep-4, -0x1.5af453ba13ccdp-4, 0x1.93fffffffbe95p-3}, /* i=50 65.133 */
  {0x1.5e451f42e9f7cp+0, -0x1.0557c8a099a78p+0, -0x1.b6569dc26b1efp-4, -0x1.991ab8f9fc63ep-3, -0x1.6eb43eae7c501p-4, -0x1.eb5c6115dd46ap-4, -0x1.665b1206461b2p-4, 0x1.9c000000021d2p-3}, /* i=51 65.479 */
  {0x1.5d3fabe1a56dep+0, -0x1.058f2d3a9e659p+0, -0x1.bff5e99873377p-4, -0x1.9c01c85e31b78p-3, -0x1.7862426dfbd5p-4, -0x1.f3e583cf00be9p-4, -0x1.721512657c562p-4, 0x1.a3ffffffffc15p-3}, /* i=52 64.93 */
  {0x1.5c3a0081755adp+0, -0x1.05c7c6d731b24p+0, -0x1.c9a6da34f05bep-4, -0x1.9efc5eed99102p-3, -0x1.823ba560faea5p-4, -0x1.fcb60b7cd79eep-4, -0x1.7e27d5c87e887p-4, 0x1.abfffffff7d41p-3}, /* i=53 64.455 */
  {0x1.5b341bec33a5bp+0, -0x1.060197b24b7cap+0, -0x1.d369e5ca30f48p-4, -0x1.a20ad4cf19dedp-3, -0x1.8c41d1b19e43p-4, -0x1.02e807b367bf3p-3, -0x1.8a94b92c5f705p-4, 0x1.b40000001f6b6p-3}, /* i=54 66.67 */
  {0x1.5a2dfce98c38p+0, -0x1.063ca216c6ap+0, -0x1.dd3f84a331e7ep-4, -0x1.a52d8508456f8p-3, -0x1.96763c3284854p-4, -0x1.079ade441fa73p-3, -0x1.975f97a8dedd6p-4, 0x1.bc0000000447cp-3}, /* i=55 64.437 */
  {0x1.5927a23ebd7cdp+0, -0x1.0678e85eafecdp+0, -0x1.e7283136e849ep-4, -0x1.a864cd93ab3dcp-3, -0x1.a0da64cf7b789p-4, -0x1.0c74ab3abeb5dp-3, -0x1.a48e869e2cc49p-4, 0x1.c400000007bbcp-3}, /* i=56 65.395 */
  {0x1.58210aaebb6d3p+0, -0x1.06b66cf38158dp+0, -0x1.f12468385dc26p-4, -0x1.abb10f76e3e79p-3, -0x1.ab6fd7015f33ap-4, -0x1.11769c265227dp-3, -0x1.b2239b0d502c5p-4, 0x1.cbffffffd5295p-3}, /* i=57 64.89 */
  {0x1.571a34f9eb46ap+0, -0x1.06f5324e76f25p+0, -0x1.fb34a8ab3949bp-4, -0x1.af12aedac7ac9p-3, -0x1.b6382a459425bp-4, -0x1.16a1ead8db744p-3, -0x1.c023bcb6ff9fbp-4, 0x1.d3fffffffd461p-3}, /* i=58 65.186 */
  {0x1.56131fde742b5p+0, -0x1.07353af8c2f65p+0, -0x1.02acb9f993d2dp-3, -0x1.b28a1322b5d6dp-3, -0x1.c1350299056c3p-4, -0x1.1bf7ddea6a254p-3, -0x1.ce923cdd7680ap-4, 0x1.dbffffff06263p-3}, /* i=59 64.132 */
  {0x1.550bca175781dp+0, -0x1.0776898c0fbffp+0, -0x1.07c9a6f7ece28p-3, -0x1.b617a708ef53p-3, -0x1.cc681101b077fp-4, -0x1.2179c94852244p-3, -0x1.dd759c6cb4961p-4, 0x1.e3fffffff8859p-3}, /* i=60 65.073 */
  {0x1.5404325db6048p+0, -0x1.07b920b27c2f3p+0, -0x1.0cf15e820f1fap-3, -0x1.b9bbd8b49cc87p-3, -0x1.d7d3140bbb35p-4, -0x1.27290ec06b53bp-3, -0x1.eccf208ae3d17p-4, 0x1.ebfffffffdd3bp-3}, /* i=61 65.106 */
  {0x1.52fc576782753p+0, -0x1.07fd03273bf8ap+0, -0x1.1224251b8743bp-3, -0x1.bd7719d9aaaecp-3, -0x1.e377d85f7b4dp-4, -0x1.2d071ea0bc28p-3, -0x1.fca7b6a10ee9p-4, 0x1.f3fffffffef73p-3}, /* i=62 64.193 */
  {0x1.51f437e808d47p+0, -0x1.084233b6c7882p+0, -0x1.176240a1e16c7p-3, -0x1.c149dfd388dcep-3, -0x1.ef583954ae3fep-4, -0x1.33157855db36cp-3, -0x1.06810859b673ep-3, 0x1.fc00000002ddep-3}, /* i=63 64.885 */
  {0x1.50ebd28fc514dp+0, -0x1.0888b53f39c35p+0, -0x1.1cabf85842846p-3, -0x1.c534a3c36f248p-3, -0x1.fb76218aae08cp-4, -0x1.3955ab14fce1bp-3, -0x1.0ef1633e7f9a8p-3, 0x1.01fffffff4107p-2}, /* i=64 65.314 */
  {0x1.4fe3260c3d6b5p+0, -0x1.08d08ab0b0357p+0, -0x1.220194f34e30bp-3, -0x1.c937e2afda2dfp-3, -0x1.03e9c5c51c4aap-3, -0x1.3fc9568e0b9edp-3, -0x1.17a8afa5d2111p-3, 0x1.05ffffffff725p-2}, /* i=65 64.333 */
  {0x1.4eda3108130a3p+0, -0x1.0919b70d9f765p+0, -0x1.276360a45315cp-3, -0x1.cd541da480a52p-3, -0x1.0a3941367a796p-3, -0x1.46722ba5f211ep-3, -0x1.20ab45e9f1575p-3, 0x1.09fffffffd495p-2}, /* i=66 64.315 */
  {0x1.4dd0f22acc672p+0, -0x1.09643d6b3d714p+0, -0x1.2cd1a72642cb2p-3, -0x1.d189d9d4ada36p-3, -0x1.10aa9149f0c3fp-3, -0x1.4d51ed3deee2cp-3, -0x1.29f804434d1dbp-3, 0x1.0e0000000112cp-2}, /* i=67 64.77 */
  {0x1.4cc76818d8ee5p+0, -0x1.09b020f1df239p+0, -0x1.324cb5c9e7706p-3, -0x1.d5d9a0be2320cp-3, -0x1.173ecd2963eb1p-3, -0x1.546a70ffa9dbep-3, -0x1.339653d6fa362p-3, 0x1.120000000088ep-2}, /* i=68 64.803 */
  {0x1.4bbd917370a87p+0, -0x1.09fd64dd635e9p+0, -0x1.37d4db833e1cp-3, -0x1.da44004e00df3p-3, -0x1.1df7155e72fa4p-3, -0x1.5bbda0396b585p-3, -0x1.3d8759c330f31p-3, 0x1.1600000005ee6p-2}, /* i=69 64.301 */
  {0x1.4ab36cd88d8ebp+0, -0x1.0a4c0c7d99782p+0, -0x1.3d6a68f6b8571p-3, -0x1.dec98b06c8e66p-3, -0x1.24d49432dc563p-3, -0x1.634d78c1dca4cp-3, -0x1.47cf249275da4p-3, 0x1.19fffffffdb0fp-2}, /* i=70 67.408 */
  {0x1.49a8f8e2c6ba4p+0, -0x1.0a9c1b36b409cp+0, -0x1.430db08790d53p-3, -0x1.e36ad8287cc54p-3, -0x1.2bd87e1afd6b1p-3, -0x1.6b1c0dea2d531p-3, -0x1.527207e23aa89p-3, 0x1.1dfffffff68b3p-2}, /* i=71 66.785 */
  {0x1.489e342943062p+0, -0x1.0aed9481b8618p+0, -0x1.48bf06661bb6bp-3, -0x1.e82883da048a9p-3, -0x1.330412245786fp-3, -0x1.732b897c6a352p-3, -0x1.5d72844e36d23p-3, 0x1.220000000594ep-2}, /* i=72 64.49 */
  {0x1.47931d3fb83abp+0, -0x1.0b407beced595p+0, -0x1.4e7ec09e4c212p-3, -0x1.ed032f5415dc1p-3, -0x1.3a589a66554c8p-3, -0x1.7b7e2cc500cc4p-3, -0x1.68d5487421308p-3, 0x1.25fffffff8c03p-2}, /* i=73 65.058 */
  {0x1.4687b2b62a177p+0, -0x1.0b94d51c619ddp+0, -0x1.544d37281bf2fp-3, -0x1.f1fb810f16e64p-3, -0x1.41d76c7d01e02p-3, -0x1.841651af5c1bfp-3, -0x1.749e1e554d3dfp-3, 0x1.29fffffffd8e9p-2}, /* i=74 64.219 */
  {0x1.457bf318fe923p+0, -0x1.0beaa3ca5b872p+0, -0x1.5a2ac3f6a7b05p-3, -0x1.f71224f163cf1p-3, -0x1.4981ea04fd46p-3, -0x1.8cf66bebca0dep-3, -0x1.80d2053353325p-3, 0x1.2dffffffff087p-2}, /* i=75 66.309 */
  {0x1.446fdcf0ce1e2p+0, -0x1.0c41ebc7e1e01p+0, -0x1.6017c3094d859p-3, -0x1.fc47cc80cfe8ep-3, -0x1.51598120e84fep-3, -0x1.96210a2aa4326p-3, -0x1.8d7523d6b86b1p-3, 0x1.3200000006796p-2}, /* i=76 65.603 */
  {0x1.43636ec2c3571p+0, -0x1.0c9ab0fd19b47p+0, -0x1.6614927a30afcp-3, -0x1.00ce9789b7a8bp-2, -0x1.595facff1a806p-3, -0x1.9f98d76081dccp-3, -0x1.9a8d055319d59p-3, 0x1.35fffffe76d7bp-2}, /* i=77 64.358 */
  {0x1.4256a70e6ccc2p+0, -0x1.0cf4f76a81efdp+0, -0x1.6c21929bfa918p-3, -0x1.03898507d8162p-2, -0x1.6195f67b606d5p-3, -0x1.a9609c3586c74p-3, -0x1.a81e94a373c9ep-3, 0x1.3a0000000337cp-2}, /* i=78 65.294 */
  {0x1.41498451c1bcep+0, -0x1.0d50c3282299fp+0, -0x1.723f25f4ae6d5p-3, -0x1.0655108771d78p-2, -0x1.69fdf496eb48dp-3, -0x1.b37b404f12143p-3, -0x1.b62e271d35ca2p-3, 0x1.3e0000000115ep-2}, /* i=79 64.264 */
  {0x1.403c0504d76d7p+0, -0x1.0dae186785edcp+0, -0x1.786db168d3e7bp-3, -0x1.09319f167b9fcp-2, -0x1.72994d36fdbc2p-3, -0x1.bdebcbf69794ap-3, -0x1.c4c1d806d56dp-3, 0x1.420000006679cp-2}, /* i=80 64.14 */
  {0x1.3f2e279c592fep+0, -0x1.0e0cfb7372c78p+0, -0x1.7ead9c406de41p-3, -0x1.0c1f991bdc1d7p-2, -0x1.7b69b5b857503p-3, -0x1.c8b5699cc0d98p-3, -0x1.d3dfd4835731ap-3, 0x1.4600000002574p-2}, /* i=81 64.142 */
  {0x1.3e1fea87ffd4ap+0, -0x1.0e6d70b107882p+0, -0x1.84ff5043dd9a4p-3, -0x1.0f1f6a7af058ap-2, -0x1.8470f3aa40cf1p-3, -0x1.d3db6794922d7p-3, -0x1.e38d72b37534cp-3, 0x1.49ffffffeec05p-2}, /* i=82 64.938 */
  {0x1.3d114c3369c94p+0, -0x1.0ecf7ca00785cp+0, -0x1.8b6339cb91509p-3, -0x1.123182b2043ffp-2, -0x1.8db0dd7d1a891p-3, -0x1.df6139d137e46p-3, -0x1.f3d28fb118fe2p-3, 0x1.4dfffffff79cbp-2}, /* i=83 64.509 */
  {0x1.3c024b05a180cp+0, -0x1.0f3323dba2f4p+0, -0x1.91d9c7d83c7bep-3, -0x1.155654fddb76ep-2, -0x1.972b5b4893674p-3, -0x1.eb4a7bc91f9a9p-3, -0x1.0259d808701cap-2, 0x1.5200000001d3cp-2}, /* i=84 65.555 */
  {0x1.3af2e56101fd3p+0, -0x1.0f986b1b23d44p+0, -0x1.98636c29b9ad9p-3, -0x1.188e587dc68e6p-2, -0x1.a0e26792aa224p-3, -0x1.f79af2738ceecp-3, -0x1.0b1d5777050d6p-2, 0x1.560000000a08cp-2}, /* i=85 65.086 */
  {0x1.39e319a321df4p+0, -0x1.0fff57329df85p+0, -0x1.9f009b569cab6p-3, -0x1.1bda085940b12p-2, -0x1.aad810287bb5ep-3, -0x1.022b472f7b545p-2, -0x1.1435ef96e434ep-2, 0x1.5a00000008332p-2}, /* i=86 65.123 */
  {0x1.38d2e624afd24p+0, -0x1.1067ed13aa36ap+0, -0x1.a5b1cce501203p-3, -0x1.1f39e3e76b01bp-2, -0x1.b50e76f8cddb4p-3, -0x1.08c0b6f47dc69p-2, -0x1.1da96aed12d03p-2, 0x1.5e00000007d28p-2}, /* i=87 65.343 */
  {0x1.37c249396449fp+0, -0x1.10d231ce1f749p+0, -0x1.ac777b63c0966p-3, -0x1.22ae6ed80c896p-2, -0x1.bf87d30411346p-3, -0x1.0f8fefcb1bd4fp-2, -0x1.277a126d5f9d7p-2, 0x1.61ffffffed243p-2}, /* i=88 65.571 */
  {0x1.36b1412fbd2e5p+0, -0x1.113e2a90e6ebp+0, -0x1.b3522485f7625p-3, -0x1.2638315f1fd13p-2, -0x1.ca46714f80104p-3, -0x1.169b3245dce6fp-2, -0x1.31ad29d34356ep-2, 0x1.6600000002ac4p-2}, /* i=89 65.438 */
  {0x1.359fcc5120892p+0, -0x1.11abdcaaae8afp+0, -0x1.ba42493cfb90dp-3, -0x1.29d7b86107a8ap-2, -0x1.d54cb5e95867p-3, -0x1.1de4d9975a5e5p-2, -0x1.3c470512e036ap-2, 0x1.6a00000001124p-2}, /* i=90 65.237 */
  {0x1.348de8e17d2d3p+0, -0x1.121b4d8ad51a5p+0, -0x1.c1486dd6a1c34p-3, -0x1.2d8d95a27fce4p-2, -0x1.e09d1cfb45733p-3, -0x1.256f5cf59ba6fp-2, -0x1.474c7a7431a45p-2, 0x1.6dfffffffc06dp-2}, /* i=91 67.224 */
  {0x1.337b951f42c49p+0, -0x1.128c82c23b213p+0, -0x1.c8651a1a7104bp-3, -0x1.315a5ff99e66fp-2, -0x1.ec3a3be8cb2dbp-3, -0x1.2d3d51121fb54p-2, -0x1.52c2ecf9123bep-2, 0x1.7200000003ce6p-2}, /* i=92 64.554 */
  {0x1.3268cf4349b4ep+0, -0x1.12ff82042105cp+0, -0x1.cf98d96862018p-3, -0x1.353eb3814fca8p-2, -0x1.f826c27e70369p-3, -0x1.355169a81a5dfp-2, -0x1.5eaef9fcfef9dp-2, 0x1.7600000000a3ep-2}, /* i=93 66.13 */
  {0x1.3155958099f49p+0, -0x1.137451271a74fp+0, -0x1.d6e43ad9b226fp-3, -0x1.393b31cfb2e4ep-2, -0x1.0232be17d2803p-2, -0x1.3dae7b23a4bf5p-2, -0x1.6b16765251bc7p-2, 0x1.7a00000006068p-2}, /* i=94 65.48 */
  {0x1.3041e6045d564p+0, -0x1.13eaf625f791dp+0, -0x1.de47d161da6fep-3, -0x1.3d50822e6454ep-2, -0x1.087ca8b2e1f5fp-2, -0x1.46577c5f73208p-2, -0x1.77ffe9c0692e4p-2, 0x1.7e00000000746p-2}, /* i=95 64.757 */
  {0x1.2f2dbef5a889dp+0, -0x1.14637720c7a0cp+0, -0x1.e5c433f1f11a3p-3, -0x1.417f51d60d62dp-2, -0x1.0ef2a471e11a1p-2, -0x1.4f4f8880efc73p-2, -0x1.8570e39bf2ea9p-2, 0x1.81fffffff9609p-2}, /* i=96 65.939 */
  {0x1.2e191e755a64ep+0, -0x1.14ddda5ddae36p+0, -0x1.ed59fd9cdbd11p-3, -0x1.45c8542c7501bp-2, -0x1.1596449c8b189p-2, -0x1.5899e0ef6fc5p-2, -0x1.936fe2cf5d3d9p-2, 0x1.860000000461ep-2}, /* i=97 65.092 */
  {0x1.2d04029e0651p+0, -0x1.155a264ac80a8p+0, -0x1.f509cdbca63d1p-3, -0x1.4a2c430559a47p-2, -0x1.1c692d251d462p-2, -0x1.6239ef690335dp-2, -0x1.a203c2875c72cp-2, 0x1.89ffffffff98fp-2}, /* i=98 64.625 */
  {0x1.2bee6983af089p+0, -0x1.15d8617d9002dp+0, -0x1.fcd4481ab0fd1p-3, -0x1.4eabdee72c22bp-2, -0x1.236d1377c6341p-2, -0x1.6c33483a54b31p-2, -0x1.b134e21ccf818p-2, 0x1.8e00000000968p-2}, /* i=99 65.449 */
  {0x1.2ad85133b22p+0, -0x1.165892b5b5bc3p+0, -0x1.025d0a8c12fffp-2, -0x1.5347ef5258369p-2, -0x1.2aa3bf562ee5fp-2, -0x1.7689ac98de76fp-2, -0x1.c1099d7f5c6e9p-2, 0x1.920000000880ap-2}, /* i=100 64.53 */
  {0x1.29c1b7b49be5cp+0, -0x1.16dac0dd6a3c1p+0, -0x1.065df0ec608d6p-2, -0x1.5801430c669cep-2, -0x1.320f0bbc7c675p-2, -0x1.81410d219a2bap-2, -0x1.d18aa81433bfep-2, 0x1.960000000bb24p-2}, /* i=101 65.149 */
  {0x1.28aa9b05f6c7ap+0, -0x1.175ef30ac53c1p+0, -0x1.0a6d3037bfef1p-2, -0x1.5cd8b06ee35a2p-2, -0x1.39b0e7d663625p-2, -0x1.8c5d8c812c1efp-2, -0x1.e2bfb8598882ap-2, 0x1.9a0000000555p-2}, /* i=102 65.592 */
  {0x1.2792f920207e8p+0, -0x1.17e5308105f7ep+0, -0x1.0e8b240681cd5p-2, -0x1.61cf15ba106acp-2, -0x1.418b57febf13dp-2, -0x1.97e38245ee89bp-2, -0x1.f4b3a47454181p-2, 0x1.9dfffffff1201p-2}, /* i=103 64.584 */
  {0x1.267acff40ffabp+0, -0x1.186d80b1e69ccp+0, -0x1.12b82a982d31ep-2, -0x1.66e5596bfb06ap-2, -0x1.49a076d2711fep-2, -0x1.a3d77de13bbcfp-2, -0x1.03b6df60552f7p-1, 0x1.a1fffffff82a1p-2}, /* i=104 64.788 */
  {0x1.25621d6b45916p+0, -0x1.18f7eb3ee6efep+0, -0x1.16f4a4ec493a2p-2, -0x1.6c1c6a9b253cep-2, -0x1.51f2764b28cap-2, -0x1.b03e49d723d2bp-2, -0x1.0d7d73ef3cb3p-1, 0x1.a5fffffffe61bp-2}, /* i=105 64.232 */
  {0x1.2448df677b8b6p+0, -0x1.198477fac1edcp+0, -0x1.1b40f6dd2b4f5p-2, -0x1.71754156eb4dap-2, -0x1.5a83a0f42536fp-2, -0x1.bd1cef264c4ebp-2, -0x1.17b1b61e45dd5p-1, 0x1.aa00000013c0ep-2}, /* i=106 64.982 */
  {0x1.232f13c29537bp+0, -0x1.1a132eead180dp+0, -0x1.1f9d873af63c3p-2, -0x1.76f0df0b9d3d7p-2, -0x1.63565b27288b3p-2, -0x1.ca78b8e485663p-2, -0x1.2259b6c3db2c2p-1, 0x1.adfffffffc103p-2}, /* i=107 65.42 */
  {0x1.2214b84e34163p+0, -0x1.1aa41848ac9dcp+0, -0x1.240abfe92a40fp-2, -0x1.7c904eed73c69p-2, -0x1.6c6d24639e7b4p-2, -0x1.d857381ccedeep-2, -0x1.2d7bae4d28ae6p-1, 0x1.b1fffffff8735p-2}, /* i=108 65.706 */
  {0x1.20f9cad3b7ef8p+0, -0x1.1b373c839d872p+0, -0x1.28890dfbd04abp-2, -0x1.8254a666e78c8p-2, -0x1.75ca98b55c2e1p-2, -0x1.e6be47e7e06afp-2, -0x1.391d6b0b11108p-1, 0x1.b60000000660ap-2}, /* i=109 64.689 */
  {0x1.1fde4914008dfp+0, -0x1.1bcca4424573ep+0, -0x1.2d18e1d6e73b1p-2, -0x1.883f058df4344p-2, -0x1.7f717230a8529p-2, -0x1.f5b411ceb8b76p-2, -0x1.4545e915705dp-1, 0x1.b9fffffffc357p-2}, /* i=110 64.626 */
  {0x1.1ec230c71572p+0, -0x1.1c6458645d9e9p+0, -0x1.31baaf4fa24dfp-2, -0x1.8e5097a00885ep-2, -0x1.89648a871558p-2, -0x1.029f893b9a313p-1, -0x1.51fd4e6462ea2p-1, 0x1.bdfffffffd2dbp-2}, /* i=111 64.534 */
  {0x1.1da57f9c0822bp+0, -0x1.1cfe6204698ffp+0, -0x1.366eedce2cc0ap-2, -0x1.948a938338544p-2, -0x1.93a6dcb50442p-2, -0x1.0ab30f505570ap-1, -0x1.5f49c0f657e1ep-1, 0x1.c20000001324p-2}, /* i=112 64.231 */
  {0x1.1c883338c5f3dp+0, -0x1.1d9aca7981d9fp+0, -0x1.3b3618713344p-2, -0x1.9aee3c4e90854p-2, -0x1.9e3b86c3483a7p-2, -0x1.13183439d0bc3p-1, -0x1.6d33b550806ccp-1, 0x1.c5fffffffe7c1p-2}, /* i=113 65.107 */
  {0x1.1b6a4939a326cp+0, -0x1.1e399b5956faap+0, -0x1.4010ae343a4b5p-2, -0x1.a17ce1db4502bp-2, -0x1.a925cbac2667cp-2, -0x1.1bd2c29ab81bcp-1, -0x1.7bc3ae1a70122p-1, 0x1.c9fffffffcca1p-2}, /* i=114 64.366 */
  {0x1.1a4bbf3156ea1p+0, -0x1.1edade7a08c53p+0, -0x1.44ff3215c6209p-2, -0x1.a837e15b113ecp-2, -0x1.b46915572ada6p-2, -0x1.24e6b9abcbcbdp-1, -0x1.8b02e447251c1p-1, 0x1.cdfffffff315dp-2}, /* i=115 64.557 */
  {0x1.192c92a8929fep+0, -0x1.1f7e9df447dbep+0, -0x1.4a022b40fce76p-2, -0x1.af20a5f9057c8p-2, -0x1.c008f6b93552bp-2, -0x1.2e58507c00449p-1, -0x1.9afa7e2db939bp-1, 0x1.d1fffffffa845p-2}, /* i=116 64.297 */
  {0x1.180cc11ddad6ep+0, -0x1.2024e42566747p+0, -0x1.4f1a25380e848p-2, -0x1.b638a9817f9fcp-2, -0x1.cc092e11ac31ap-2, -0x1.382bf968ce08ep-1, -0x1.abb50f3b4eb5p-1, 0x1.d5fffffffa417p-2}, /* i=117 64.662 */
  {0x1.16ec48052a33bp+0, -0x1.20cdbbb19c57fp+0, -0x1.5447b001897e7p-2, -0x1.bd817514a2968p-2, -0x1.d86da74fe18ecp-2, -0x1.426665d5ca597p-1, -0x1.bd3d569f9c01ep-1, 0x1.d9fffffffac51p-2}, /* i=118 65.365 */
  {0x1.15cb24c7b45dep+0, -0x1.21792f864e7c8p+0, -0x1.598b60573c5bep-2, -0x1.c4fca1e1d7566p-2, -0x1.e53a7e9c2eb2cp-2, -0x1.4d0c8a26d84bdp-1, -0x1.cf9eb2e49e323p-1, 0x1.ddfffffffeb9fp-2}, /* i=119 65.316 */
  {0x1.14a954c3a1c3bp+0, -0x1.22274adc711fcp+0, -0x1.5ee5cfd76c7bfp-2, -0x1.ccabd9eddd7dbp-2, -0x1.f274030a3f916p-2, -0x1.5823a2029f70cp-1, -0x1.e2e5bd3fac846p-1, 0x1.e1ffffffed671p-2}, /* i=120 64.427 */
  {0x1.1386d54ba021ap+0, -0x1.22d8193b1bd46p+0, -0x1.64579d3932aecp-2, -0x1.d490d8e518293p-2, -0x1.000f5cbd53864p-1, -0x1.63b134e48c64fp-1, -0x1.f71f4ea30620dp-1, 0x1.e60000000d03ep-2}, /* i=121 64.194 */
  {0x1.1263a3a6de6f8p+0, -0x1.238ba679f577bp+0, -0x1.69e16c8150fa5p-2, -0x1.dcad6cf6bf31bp-2, -0x1.071fafd274794p-1, -0x1.6fbb1afed0ff4p-1, -0x1.062cd246e6acp+0, 0x1.e9fffffff9237p-2}, /* i=122 65.378 */
  {0x1.113fbd1058181p+0, -0x1.2441fec425bd9p+0, -0x1.6f83e73cf4c6dp-2, -0x1.e50377c16692ap-2, -0x1.0e6d77af50b36p-1, -0x1.7c47827f199fap-1, -0x1.11515efa000e9p+0, 0x1.edfffffffeccdp-2}, /* i=123 64.334 */
  {0x1.101b1eb6d081p+0, -0x1.24fb2e9af69e9p+0, -0x1.753fbcbbecd64p-2, -0x1.ed94ef4811f51p-2, -0x1.15fb51068e769p-1, -0x1.895cf52da6841p-1, -0x1.1d0587127d8ep+0, 0x1.f2000000019d8p-2}, /* i=124 64.443 */
  {0x1.0ef5c5bc514efp+0, -0x1.25b742d8df698p+0, -0x1.7b15a2502a722p-2, -0x1.f663def8f95d7p-2, -0x1.1dcbfa2dfb23bp-1, -0x1.97025e7c61abp-1, -0x1.295124f7c97ccp+0, 0x1.f60000000fbd8p-2}, /* i=125 64.871 */
  {0x1.0dcfaf35ef7afp+0, -0x1.267648b481c97p+0, -0x1.8106538efc8afp-2, -0x1.ff7268c174013p-2, -0x1.25e255143683cp-1, -0x1.a53f1205d88dcp-1, -0x1.363e049e0de42p+0, 0x1.f9fffffff2eabp-2}, /* i=126 64.459 */
  {0x1.0ca8d82b329d9p+0, -0x1.27384dc402fc9p+0, -0x1.871292977235bp-2, -0x1.0461631a75b32p-1, -0x1.2e41695e9c902p-1, -0x1.b41ad28de1093p-1, -0x1.43d523412b4f6p+0, 0x1.fdfffffffda81p-2}, /* i=127 64.991 */
  {0x1.0b813d95feb08p+0, -0x1.27fd600031d0cp+0, -0x1.8d3b285994611p-2, -0x1.092ba4e089d6ap-1, -0x1.36ec66a3deb39p-1, -0x1.c39dd98242b7cp-1, -0x1.5220c8aed571fp+0, 0x1.01000000034e4p-1}, /* i=128 64.932 */
  {0x1.0a58dc620307cp+0, -0x1.28c58dc81f30cp+0, -0x1.9380e4e3c5563p-2, -0x1.0e192ffc2b85dp-1, -0x1.3fe6a6d2db80dp-1, -0x1.d3d0df143b66dp-1, -0x1.612c29c4f4832p+0, 0x1.0300000001efap-1}, /* i=129 64.567 */
  {0x1.092fb16c4224cp+0, -0x1.2990e5e4ccab4p+0, -0x1.99e49fb391779p-2, -0x1.132b4877a78dp-1, -0x1.4933b0c34cf38p-1, -0x1.e4bd22f023ae7p-1, -0x1.71026c5d36295p+0, 0x1.05000000210b2p-1}, /* i=130 64.738 */
  {0x1.0805b9830e811p+0, -0x1.2a5f778cb0eccp+0, -0x1.a067380816973p-2, -0x1.186340d59d297p-1, -0x1.52d73aed40b89p-1, -0x1.f66c759a2a931p-1, -0x1.81b0dea82a6fbp+0, 0x1.06fffffffe3e3p-1}, /* i=131 65.907 */
  {0x1.06daf164d8746p+0, -0x1.2b31526835582p+0, -0x1.a709953f5b8e5p-2, -0x1.1dc27ad8b30b3p-1, -0x1.5cd52e5ee0211p-1, -0x1.0474a14c9c252p+0, -0x1.93441d35ee076p+0, 0x1.08fffffffd12bp-1}, /* i=132 65.617 */
  {0x1.05af55c07f3edp+0, -0x1.2c06869553dc3p+0, -0x1.adcca72feaa5bp-2, -0x1.234a6850a5147p-1, -0x1.6731a9d604eddp-1, -0x1.0e1f4daaad957p+0, -0x1.a5ca92f3e11c3p+0, 0x1.0afffffff48e5p-1}, /* i=133 64.537 */
  {0x1.0482e3345ce53p+0, -0x1.2cdf24ac42f69p+0, -0x1.b4b1668e7379cp-2, -0x1.28fc8bf9e2256p-1, -0x1.71f1051f5e46bp-1, -0x1.183c2173ed5aap+0, -0x1.b953da2657c35p+0, 0x1.0d000000047d6p-1}, /* i=134 64.161 */
  {0x1.0355964e22ff3p+0, -0x1.2dbb3dc3be91fp+0, -0x1.bbb8d55408e0bp-2, -0x1.2eda7a6735779p-1, -0x1.7d17d4ab8c8e3p-1, -0x1.22d15d681aac4p+0, -0x1.cdf0460a07ee5p+0, 0x1.0efffffffd1cbp-1}, /* i=135 64.043 */
  {0x1.02276b89f6e02p+0, -0x1.2e9ae3760d62ap+0, -0x1.c2e3ff2e59306p-2, -0x1.34e5dafdda504p-1, -0x1.88aaed6c6efa7p-1, -0x1.2de5a85dbb599p+0, -0x1.e3b13364fa1bap+0, 0x1.110000000775ep-1}, /* i=136 64.903 */
  {0x1.00f85f524826dp+0, -0x1.2f7e27e5b41cp+0, -0x1.ca33f9f187fc9p-2, -0x1.3b2068fdda27cp-1, -0x1.94af68f2bf43dp-1, -0x1.398016ae983e1p+0, -0x1.faa9b4611c082p+0, 0x1.13000000082ecp-1}, /* i=137 64.618 */
  {0x1.ff90dbfe1b273p-1, -0x1.30651dc2d07fbp+0, -0x1.d1a9e613ebe54p-2, -0x1.418bf49e69d5ep-1, -0x1.a12aa9df21479p-1, -0x1.45a83245eaf41p+0, -0x1.09773cb9a0206p+1, 0x1.14ffffffff1bfp-1}, /* i=138 65.699 */
  {0x1.fd2f27aa7a344p-1, -0x1.314fd85084a52p+0, -0x1.d946ef2f29ee4p-2, -0x1.482a643b6c4bdp-1, -0x1.ae2260a554cf6p-1, -0x1.5266035748ecap+0, -0x1.164a6e9adbc78p+1, 0x1.16fffffff8e73p-1}, /* i=139 64.983 */
  {0x1.facb9a0c7f1f9p-1, -0x1.323e6b6aa34b6p+0, -0x1.e10c4c88907e6p-2, -0x1.4efdb596a51b8p-1, -0x1.bb9c90a7f68bp-1, -0x1.5fc219cf7457ep+0, -0x1.23da3ebad7332p+1, 0x1.18fffffffefa1p-1}, /* i=140 64.822 */
  {0x1.f8662b5ec66acp-1, -0x1.3330eb8b9e115p+0, -0x1.e8fb41a113e7dp-2, -0x1.5607ff2dfd8b2p-1, -0x1.c99f95b853e27p-1, -0x1.6dc597904e504p+0, -0x1.323327695a533p+1, 0x1.1affffffffdfdp-1}, /* i=141 64.605 */
  {0x1.f5fed3b239292p-1, -0x1.34276dd2e0208p+0, -0x1.f1151eceb40d7p-2, -0x1.5d4b71a996f44p-1, -0x1.d8322a010da43p-1, -0x1.7c7a3b8ea772cp+0, -0x1.4162f93f2d7fep+1, 0x1.1d0000000076ep-1}, /* i=142 64.87 */
  {0x1.f3958aecdee06p-1, -0x1.3522080b5339ep+0, -0x1.f95b41dd8e655p-2, -0x1.64ca5961633c6p-1, -0x1.e75b6c64d7ba6p-1, -0x1.8bea6de0f203p+0, -0x1.51781037f109cp+1, 0x1.1effffffff385p-1}, /* i=143 65.216 */
  {0x1.f12a48c87c719p-1, -0x1.3620d0b246484p+0, -0x1.00e78b5d6cfcep-1, -0x1.6c871ffd95c1ap-1, -0x1.f722e7589c3a3p-1, -0x1.9c214cdb2ebd2p+0, -0x1.628281f5ebe8cp+1, 0x1.20fffffff6fd7p-1}, /* i=144 64.963 */
  {0x1.eebd04d10716bp-1, -0x1.3723defeb8854p+0, -0x1.05390c15332cep-1, -0x1.74844e33f62e5p-1, -0x1.03c84c256fd27p+0, -0x1.ad2abb5072b7bp+0, -0x1.7492fc525c9d4p+1, 0x1.22fffffffa3afp-1}, /* i=145 64.67 */
  {0x1.ec4db6635e82ap-1, -0x1.382b4ae8de57fp+0, -0x1.09a2e842dac87p-1, -0x1.7cc48da2d65fap-1, -0x1.0c567bccd8972p+0, -0x1.bf13701381eb3p+0, -0x1.87bc1a64a3032p+1, 0x1.250000000731ap-1}, /* i=146 64.379 */
  {0x1.e9dc54abd005p-1, -0x1.39372d321827bp+0, -0x1.0e25ed438dd44p-1, -0x1.854aaacd98fa9p-1, -0x1.154080976fdaep+0, -0x1.d1e906db3965bp+0, -0x1.9c1185c2b850bp+1, 0x1.26fffffffd2ddp-1}, /* i=147 64.919 */
  {0x1.e768d6a3f6aa7p-1, -0x1.3a479f6d917e1p+0, -0x1.12c2f111adae7p-1, -0x1.8e19973f0cf06p-1, -0x1.1e8b1ee701bcap+0, -0x1.e5ba12ae6d533p+0, -0x1.b1a8df3f6e3c9p+1, 0x1.2900000009734p-1}, /* i=148 65.602 */
  {0x1.e4f33311af921p-1, -0x1.3b5cbc08d1f65p+0, -0x1.177ad2b620b41p-1, -0x1.97346bce79372p-1, -0x1.283b668ee128p+0, -0x1.fa9631f3b636p+0, -0x1.c898a9ed2e92dp+1, 0x1.2b00000023be6p-1}, /* i=149 64.303 */
  {0x1.e27b6085689c7p-1, -0x1.3c769e5500aefp+0, -0x1.1c4e7ac1b3cc3p-1, -0x1.a09e6b104801ap-1, -0x1.3256b8403fecep+0, -0x1.0847122da6093p+1, -0x1.e0fb01c9aaec2p+1, 0x1.2d00000004ca8p-1}, /* i=150 64.45 */
  {0x1.e001555734d6bp-1, -0x1.3d9562912adadp+0, -0x1.213edbd060431p-1, -0x1.aa5b03f878ac9p-1, -0x1.3ce2cb72f8163p+0, -0x1.13d9f16dc4c1p+1, -0x1.faea329e25c5ap+1, 0x1.2f0000000b0bp-1}, /* i=151 64.97 */
  {0x1.dd8507a66ae67p-1, -0x1.3eb925f3e4a1cp+0, -0x1.264cf30f9734bp-1, -0x1.b46dd4a410de4p-1, -0x1.47e5b4bb39c98p+0, -0x1.200d5ce8c3c3bp+1, -0x1.0b4226cc2dbc7p+2, 0x1.3100000000544p-1}, /* i=152 64.552 */
  {0x1.db066d56827a9p-1, -0x1.3fe206b6a9169p+0, -0x1.2b79c8d285425p-1, -0x1.bedaad61d719p-1, -0x1.5365ecb80ae04p+0, -0x1.2cebb2d96224cp+1, -0x1.19f45d94fc17ep+2, 0x1.330000000977p-1}, /* i=153 64.377 */
  {0x1.d8857c0de3146p-1, -0x1.41102420e572bp+0, -0x1.30c6712ba9a45p-1, -0x1.c9a593ec95d5fp-1, -0x1.5f6a578b54c8bp+0, -0x1.3a8019331375bp+1, -0x1.299d36982ccd5p+2, 0x1.34fffffff260dp-1}, /* i=154 64.385 */
  {0x1.d6022932d59a5p-1, -0x1.42439e94764e5p+0, -0x1.36340c942832dp-1, -0x1.d4d2c6eb21436p-1, -0x1.6bfa4d00a99fcp+0, -0x1.48d68eda08746p+1, -0x1.3a4f93d334639p+2, 0x1.36ffffffe695dp-1}, /* i=155 64.399 */
  {0x1.d37c69e9de86fp-1, -0x1.437c979a1cb4ap+0, -0x1.3bc3c89ad679p-1, -0x1.e066c1adeb714p-1, -0x1.791da160c712bp+0, -0x1.57fbfe7fb537bp+1, -0x1.4c1fc0685f65fp+2, 0x1.38fffffff4905p-1}, /* i=156 64.453 */
  {0x1.d0f433136c977p-1, -0x1.44bb31eeeb01ep+0, -0x1.4176e0a017767p-1, -0x1.ec6640388cb3bp-1, -0x1.86dcaf0bdc21ap+0, -0x1.67fe535c4181dp+1, -0x1.5f245c59dbd0fp+2, 0x1.3b00000006762p-1}, /* i=157 64.434 */
  {0x1.ce6979492226ep-1, -0x1.45ff91929871bp+0, -0x1.474e9e9ef2f32p-1, -0x1.f8d6439ce453ap-1, -0x1.954060f1b1f3ep+0, -0x1.78ec8ff8dce62p+1, -0x1.7375783647c67p+2, 0x1.3d00000014dc2p-1}, /* i=158 64.663 */
  {0x1.cbdc30db7524cp-1, -0x1.4749dbd66b9a8p+0, -0x1.4d4c5c02bb463p-1, -0x1.02de0b55bb9c4p+0, -0x1.a4523df3bb35dp+0, -0x1.8ad6e73fe9a5ep+1, -0x1.892d6ae0c3ec3p+2, 0x1.3efffffffdc7fp-1}, /* i=159 64.674 */
  {0x1.c94c4dce4df36p-1, -0x1.489a376d74d4ap+0, -0x1.5371828d689f5p-1, -0x1.098ea983bc81ap+0, -0x1.b41c7553e44d5p+0, -0x1.9dced81db0ceep+1, -0x1.a069740e99d3ep+2, 0x1.410000000ce3ap-1}, /* i=160 64.598 */
  {0x1.c6b9c3d759d43p-1, -0x1.49f0cc7cc78dep+0, -0x1.59bf8d496e0a5p-1, -0x1.107ff34ca96bfp+0, -0x1.c4a9ec3cf085dp+0, -0x1.b1e74be94ad62p+1, -0x1.b948d6618c791p+2, 0x1.43000000151bcp-1}, /* i=161 63.983 */
  {0x1.c424865a5bd5dp-1, -0x1.4b4dc4ada2061p+0, -0x1.60380990fe788p-1, -0x1.17b50cbdf1819p+0, -0x1.d6064c91d451ep+0, -0x1.c734b800748d4p+1, -0x1.d3ee52516cd9p+2, 0x1.4500000001db6p-1}, /* i=162 63.759 */
  {0x1.c18c8865d29f2p-1, -0x1.4cb14b406a7c6p+0, -0x1.66dc9826c62f8p-1, -0x1.1f314a31c802cp+0, -0x1.e83e151fd238ep+0, -0x1.ddcd42e5abc65p+1, -0x1.f07f495df89f3p+2, 0x1.47000000074aap-1}, /* i=163 64.622 */
  {0x1.bef1bcb0844dcp-1, -0x1.4e1b8d203e315p+0, -0x1.6daeee5febdfep-1, -0x1.26f833c35e679p+0, -0x1.fb5eab4b68097p+0, -0x1.f5c8ed433da92p+1, -0x1.0792bdb9c7c98p+3, 0x1.4900000003438p-1}, /* i=164 64.232 */
  {0x1.bc541595689bfp-1, -0x1.4f8cb8f87e796p+0, -0x1.74b0d76767a38p-1, -0x1.2f0d891164b6bp+0, -0x1.07bb373dccfe2p+1, -0x1.07a0dfafbc8b7p+2, -0x1.180701b77937cp+3, 0x1.4b0000000192ep-1}, /* i=165 64.058 */
  {0x1.b9b3851047e91p-1, -0x1.5104ff4b34aa7p+0, -0x1.7be435969c538p-1, -0x1.3775454f57ccbp+0, -0x1.124a66ad0c529p+1, -0x1.1529fdba386dep+2, -0x1.29b552e764ac3p+3, 0x1.4d0000001249ap-1}, /* i=166 64.134 */
  {0x1.b70ffcba718dp-1, -0x1.52849288b1478p+0, -0x1.834b03e68a7c3p-1, -0x1.4033a3aee1a75p+0, -0x1.1d652e929d5p+1, -0x1.238f2ac46f226p+2, -0x1.3cb8ef8ce2264p+3, 0x1.4effffffec6a9p-1}, /* i=167 63.577 */
  {0x1.b4696dc562ff1p-1, -0x1.540ba729ba7f7p+0, -0x1.8ae75781e0de9p-1, -0x1.494d2431409b6p+0, -0x1.291479ae64e2dp+1, -0x1.32e118c645cb5p+2, -0x1.51300de69c5bdp+3, 0x1.50fffffffae2dp-1}, /* i=168 63.778 */
  {0x1.b1bfc8f8617fep-1, -0x1.559a73c98e71cp+0, -0x1.92bb616c4781bp-1, -0x1.52c690d9d9187p+0, -0x1.3561e0f2bb8eap+1, -0x1.4331f424c7693p+2, -0x1.673bbfd781335p+3, 0x1.530000000592p-1}, /* i=169 63.7 */
  {0x1.af12feab06f05p-1, -0x1.573131433bfe7p+0, -0x1.9ac970524071dp-1, -0x1.5ca503606ec3bp+0, -0x1.4257bb0d74299p+1, -0x1.54958a239b463p+2, -0x1.7f004d1546b9dp+3, 0x1.55000000007aep-1}, /* i=170 63.59 */
  {0x1.ac62fec0ab9e2p-1, -0x1.58d01ad03e283p+0, -0x1.a313f279a93c1p-1, -0x1.66edeb624a635p+0, -0x1.50012d81779ddp+1, -0x1.672173ba3a00dp+2, -0x1.98a626c721398p+3, 0x1.57000000053aap-1}, /* i=171 63.653 */
  {0x1.a9afb8a3e2ee7p-1, -0x1.5a776e28e8334p+0, -0x1.ab9d77d80360fp-1, -0x1.71a71521d762ep+0, -0x1.5e6a3f857f62p+1, -0x1.7aed456197ee8p+2, -0x1.b45987806e20ep+3, 0x1.590000000ff9ep-1}, /* i=172 63.694 */
  {0x1.a6f91b41789cp-1, -0x1.5c276ba71e8f6p+0, -0x1.b468b453b210dp-1, -0x1.7cd6b0e5a90ecp+0, -0x1.6d9feeda91a38p+1, -0x1.9012c482b3248p+2, -0x1.d24bc6968d188p+3, 0x1.5affffffea651p-1}, /* i=173 63.595 */
  {0x1.a43f1501e65f5p-1, -0x1.5de0566c31dap+0, -0x1.bd7882379e5c8p-1, -0x1.88835b0c4255ep+0, -0x1.7db046e1d4137p+1, -0x1.a6ae234cfb04cp+2, -0x1.f2b322298994ap+3, 0x1.5d00000001344p-1}, /* i=174 63.268 */
  {0x1.a18193c583136p-1, -0x1.5fa274875f554p+0, -0x1.c6cfe4cf9546ep-1, -0x1.94b424d56db05p+0, -0x1.8eaa7a180e8fbp+1, -0x1.bede43abf5cf8p+2, -0x1.0ae60d2a21c22p+4, 0x1.5effffffffab9p-1}, /* i=175 63.343 */
  {0x1.9ec084dcdb422p-1, -0x1.616e0f21419d9p+0, -0x1.d0720b478218p-1, -0x1.a1709e11597a6p+0, -0x1.a09efe6d8371ep+1, -0x1.d8c502895732bp+2, -0x1.1decc2bace9c4p+4, 0x1.6100000001ffap-1}, /* i=176 63.384 */
  {0x1.9bfbd5031708p-1, -0x1.634372a8b9516p+0, -0x1.da6253bf817d1p-1, -0x1.aec0dfb44f60cp+0, -0x1.b39facabde5ddp+1, -0x1.f4878c4aa495dp+2, -0x1.3292e501f59c9p+4, 0x1.6300000004bcep-1}, /* i=177 62.89 */
  {0x1.9933705713f5cp-1, -0x1.6522ef03a2634p+0, -0x1.e4a44ea599864p-1, -0x1.bcad977769f87p+0, -0x1.c7bfe362d33c3p+1, -0x1.09275df82f11dp+3, -0x1.4901f5629cae3p+4, 0x1.650000000330ap-1}, /* i=178 62.991 */
  {0x1.966742542cdc3p-1, -0x1.670cd7c2fc64fp+0, -0x1.ef3bc2586630cp-1, -0x1.cb40149fec3dbp+0, -0x1.dd14adc7aca04p+1, -0x1.1923c325c4eaap+3, -0x1.616805ebdf193p+4, 0x1.6700000007a8ep-1}, /* i=179 62.826 */
  {0x1.939735cae7ff7p-1, -0x1.6901845aa491p+0, -0x1.fa2caf19003b6p-1, -0x1.da82560d50653p+0, -0x1.f3b4ef03e4691p+1, -0x1.2a51b97f16d0ap+3, -0x1.7bf8a595e7c32p+4, 0x1.6900000000caep-1}, /* i=180 62.998 */
  {0x1.90c334d898904p-1, -0x1.6b01505d970f5p+0, -0x1.02bda9abdde68p+0, -0x1.ea7f19b8b70adp+0, -0x1.05dcc9488a31dp+2, -0x1.3ccc92a87a329p+3, -0x1.98ed63f054b21p+4, 0x1.6b000000041fp-1}, /* i=181 62.77 */
  {0x1.8deb28df47c47p-1, -0x1.6d0c9bbe240eap+0, -0x1.0896182fcc849p+0, -0x1.fb41edc94a51p+0, -0x1.129ee11f0fe07p+2, -0x1.50b27b8dfbdfp+3, -0x1.b886c8c4d5608p+4, 0x1.6d00000004f16p-1}, /* i=182 62.457 */
  {0x1.8b0efa7c611fap-1, -0x1.6f23cb13866d8p+0, -0x1.0ea207b824cf7p+0, -0x1.066ba1bb078c3p+1, -0x1.202f91571e00ap+2, -0x1.6624d4434b0c4p+3, -0x1.db0cf573fdcfep+4, 0x1.6f0000000d3eap-1}, /* i=183 62.484 */
  {0x1.882e917f7d5f5p-1, -0x1.714747e450ae1p+0, -0x1.14e403a64884bp+0, -0x1.0fa641f21ded8p+1, -0x1.2e9f0b90c751dp+2, -0x1.7d4893d5f97c9p+3, -0x1.00688b3928679p+5, 0x1.70fffffffba87p-1}, /* i=184 62.33 */
  {0x1.8549d4df7ed47p-1, -0x1.737780f775926p+0, -0x1.1b5ec1289036ap+0, -0x1.195813a5a6f16p+1, -0x1.3dff06ce8bed1p+2, -0x1.9646b9f33d3b1p+3, -0x1.151732ee4c78ep+5, 0x1.7300000001896p-1}, /* i=185 62.334 */
  {0x1.8260aab0a4d53p-1, -0x1.75b4eaaa78e78p+0, -0x1.2215228b4bd63p+0, -0x1.2388e74a721fep+1, -0x1.4e62ea3eb6f2cp+2, -0x1.b14cd06f5975fp+3, -0x1.2bc5c53ce5dep+5, 0x1.7500000000ba8p-1}, /* i=186 62.142 */
  {0x1.7f72f819da81ap-1, -0x1.77ffff4e90137p+0, -0x1.290a3adaa9d69p+0, -0x1.2e412fe94c73p+1, -0x1.5fdffd6c0343bp+2, -0x1.ce8d7f39676b3p+3, -0x1.44ae43cd8dadbp+5, 0x1.76fffffefa109p-1}, /* i=187 61.953 */
  {0x1.7c80a1413e209p-1, -0x1.7a593f93c2a1bp+0, -0x1.304151f251a6ep+0, -0x1.398a13124abaep+1, -0x1.728d9ed14eac2p+2, -0x1.ee41360544b04p+3, -0x1.60122bb27ff5dp+5, 0x1.78ffffffee08bp-1}, /* i=188 61.82 */
  {0x1.7989894d80aa7p-1, -0x1.7cc132eb4a884p+0, -0x1.37bde8bd19f81p+0, -0x1.456d7a4dd7912p+1, -0x1.868580fd6de2ep+2, -0x1.085376f69fa61p+4, -0x1.7e3bb49161a68p+5, 0x1.7afffffffce25p-1}, /* i=189 61.755 */
  {0x1.768d9249c5ce6p-1, -0x1.7f386809bc4fbp+0, -0x1.3f83be29922a7p+0, -0x1.51f62704542dap+1, -0x1.9be3f08a35933p+2, -0x1.1b0284955276ap+4, -0x1.9f7eed5208dcap+5, 0x1.7d00000001c2cp-1}, /* i=190 61.66 */
  {0x1.738c9d1d679cep-1, -0x1.81bf7562d8fefp+0, -0x1.4796d46955bap+0, -0x1.5f2fc86964e14p+1, -0x1.b2c822edad027p+2, -0x1.2f5529ded2291p+4, -0x1.c43b83d4762a9p+5, 0x1.7efffffff90dbp-1}, /* i=191 61.485 */
  {0x1.71f93c6d63c51p+0, -0x1.0fa989278b9e8p-3, 0x1.12e5718944089p-5, -0x1.7279ee5dcae5bp-7, 0x1.1e4f5193c9038p-8, -0x1.df9bd40debb84p-10, 0x1.80fffffffc78bp-1}, /* i=192 66.95 */
  {0x1.71d74fd079481p+0, -0x1.0f205bc1b6ea9p-3, 0x1.11d06c3c0254p-5, -0x1.703fa55440ee2p-7, 0x1.1bfae59d98d05p-8, -0x1.daad1b6208ad8p-10, 0x1.83000000c4c04p-1}, /* i=193 66.82 */
  {0x1.71b57450b185bp+0, -0x1.0e97b8744d4e9p-3, 0x1.10bd10e8918b1p-5, -0x1.6e09fefffe701p-7, 0x1.19ac9a3a4d121p-8, -0x1.d5ce14b42d997p-10, 0x1.85000000081aep-1}, /* i=194 67.683 */
  {0x1.7193a9dcb1183p+0, -0x1.0e0f9e6a97d75p-3, 0x1.0fab5c184b761p-5, -0x1.6bd8ef30f0a88p-7, 0x1.17645bed0ee29p-8, -0x1.d0fe85712628dp-10, 0x1.86ffffffff97dp-1}, /* i=195 66.726 */
  {0x1.7171f06367b2p+0, -0x1.0d880cd25cbc6p-3, 0x1.0e9b4a5f2a742p-5, -0x1.69ac69e0dff6ep-7, 0x1.1522178791ebcp-8, -0x1.cc3e3404d5cbdp-10, 0x1.8900000004e0ep-1}, /* i=196 66.731 */
  {0x1.715047d3d6663p+0, -0x1.0d0102daf2776p-3, 0x1.0d8cd859d7a3fp-5, -0x1.6784632f1ad8cp-7, 0x1.12e5ba1c65e02p-8, -0x1.c78ce7cc40d3bp-10, 0x1.8afffffff68c1p-1}, /* i=197 66.352 */
  {0x1.712eb01d12eabp+0, -0x1.0c7a7fb5498acp-3, 0x1.0c8002adabadp-5, -0x1.6560cf60253d3p-7, 0x1.10af310c6e4f7p-8, -0x1.c2ea691c94f3dp-10, 0x1.8d00000004792p-1}, /* i=198 66.218 */
  {0x1.710d292e52438p+0, -0x1.0bf4829413bf8p-3, 0x1.0b74c608ea2edp-5, -0x1.6341a2dddfad5p-7, 0x1.0e7e69f7eb7cfp-8, -0x1.be568130d1fbbp-10, 0x1.8f0000000a53ep-1}, /* i=199 67.838 */
  {0x1.70ebb2f6e1063p+0, -0x1.0b6f0aaba1958p-3, 0x1.0a6b1f2269a99p-5, -0x1.6126d23682ca7p-7, 0x1.0c5352c06ae07p-8, -0x1.b9d0fa3003b57p-10, 0x1.910000000210ep-1}, /* i=200 67.086 */
  {0x1.70ca4d66233aep+0, -0x1.0aea1731de627p-3, 0x1.09630ab979518p-5, -0x1.5f10521c1b03p-7, 0x1.0a2dd99306652p-8, -0x1.b5599f23eee8cp-10, 0x1.9300000002ee8p-1}, /* i=201 67.344 */
  {0x1.70a8f86b98b8p+0, -0x1.0a65a75e5e42fp-3, 0x1.085c8595ea73cp-5, -0x1.5cfe17644d71p-7, 0x1.080decd700a23p-8, -0x1.b0f03bf5ff7d7p-10, 0x1.94fffffffb9a7p-1}, /* i=202 66.677 */
  {0x1.7087b3f6d7d5p+0, -0x1.09e1ba6a459b4p-3, 0x1.07578c87cdba6p-5, -0x1.5af0170783f1fp-7, 0x1.05f37b3589c31p-8, -0x1.ac949d6dc2bc7p-10, 0x1.970000000060ep-1}, /* i=203 67.396 */
  {0x1.70667ff7922dcp+0, -0x1.095e4f9058abfp-3, 0x1.06541c678040bp-5, -0x1.58e64620bc027p-7, 0x1.03de739bfc68dp-8, -0x1.a8469126fb4afp-10, 0x1.98fffffffef0bp-1}, /* i=204 66.631 */
  {0x1.70455c5d9026ep+0, -0x1.08db660ce6793p-3, 0x1.055232157043dp-5, -0x1.56e099ecc4d3fp-7, 0x1.01cec52ee141ep-8, -0x1.a405e5924f077p-10, 0x1.9b0000000043ep-1}, /* i=205 67.752 */
  {0x1.70244918b34fcp+0, -0x1.0858fd1dcefa9p-3, 0x1.0451ca7a1816cp-5, -0x1.54df07c9ec259p-7, 0x1.ff88bea8b557fp-9, -0x1.9fd269eea7691p-10, 0x1.9d00000000f12p-1}, /* i=206 66.994 */
  {0x1.70034618f5379p+0, -0x1.07d714027b41cp-3, 0x1.0352e285dd99p-5, -0x1.52e185377503bp-7, 0x1.fb7e6360fe01ep-9, -0x1.9babee4b4029dp-10, 0x1.9f000000002ep-1}, /* i=207 67.239 */
  {0x1.6fe2534e65ee6p+0, -0x1.0755a9fbd46d7p-3, 0x1.02557730efacbp-5, -0x1.50e807d50c06ap-7, 0x1.f77e5841a33d9p-9, -0x1.9792437a8083ap-10, 0x1.a1000000142p-1}, /* i=208 66.531 */
  {0x1.6fc170a932b07p+0, -0x1.06d4be4c5ab01p-3, 0x1.0159857b62acfp-5, -0x1.4ef28562ba57ap-7, 0x1.f3887d73c6525p-9, -0x1.93853b12e12c9p-10, 0x1.a2ffffffff323p-1}, /* i=209 66.755 */
  {0x1.6fa09e1997e16p+0, -0x1.06545037eb32dp-3, 0x1.005f0a6cae6adp-5, -0x1.4d00f3bf9e2edp-7, 0x1.ef9cb3a43717p-9, -0x1.8f84a76f8707fp-10, 0x1.a5000000073dep-1}, /* i=210 66.42 */
  {0x1.6f7fdb8ff0e27p+0, -0x1.05d45f03fb1ddp-3, 0x1.fecc062822f4p-6, -0x1.4b1348ea6d3e2p-7, 0x1.ebbadbe1c07f7p-9, -0x1.8b905ba6835adp-10, 0x1.a6fffffffc25bp-1}, /* i=211 67.332 */
  {0x1.6f5f28fcab6a7p+0, -0x1.0554e9f762fdp-3, 0x1.fcdcd910353a2p-6, -0x1.49297b0046b7cp-7, 0x1.e7e2d7b06bab5p-9, -0x1.87a82b8778d77p-10, 0x1.a8fffffffa327p-1}, /* i=212 67.17 */
  {0x1.6f3e86504d97dp+0, -0x1.04d5f05a73871p-3, 0x1.faf087cd83668p-6, -0x1.4743803ca3dap-7, 0x1.e4148902cd102p-9, -0x1.83cbeb986ff65p-10, 0x1.ab000000089e6p-1}, /* i=213 67.462 */
  {0x1.6f1df37b7785fp+0, -0x1.04577176f8be7p-3, 0x1.f9070caabc668p-6, -0x1.45614ef904aacp-7, 0x1.e04fd2364b74dp-9, -0x1.7ffb7114af63dp-10, 0x1.acfffffffda2fp-1}, /* i=214 66.907 */
  {0x1.6efd706edaac7p+0, -0x1.03d96c98157d8p-3, 0x1.f7206200a2bf4p-6, -0x1.4382ddac07b1dp-7, 0x1.dc94960f33bdp-9, -0x1.7c3691e339aecp-10, 0x1.af00000003316p-1}, /* i=215 66.689 */
  {0x1.6edcfd1b440abp+0, -0x1.035be10a6812ap-3, 0x1.f53c82367c3e9p-6, -0x1.41a822e999fbbp-7, 0x1.d8e2b7c04364bp-9, -0x1.787d249add6d1p-10, 0x1.b0fffffff4577p-1}, /* i=216 66.811 */
  {0x1.6ebc9971913b1p+0, -0x1.02dece1bdd1p-3, 0x1.f35b67c145f23p-6, -0x1.3fd11561f1845p-7, 0x1.d53a1adf0498fp-9, -0x1.74cf007c9f96bp-10, 0x1.b300000006976p-1}, /* i=217 67.247 */
  {0x1.6e9c4562bce7ap+0, -0x1.0262331bdca46p-3, 0x1.f17d0d2445aeap-6, -0x1.3dfdabe1dfd74p-7, 0x1.d19aa36805fbap-9, -0x1.712bfd6c485d2p-10, 0x1.b5000000028f8p-1}, /* i=218 67.03 */
  {0x1.6e7c00dfd2646p+0, -0x1.01e60f5b17ea7p-3, 0x1.efa16cf02a351p-6, -0x1.3c2ddd51bba8ap-7, 0x1.ce0435b62894ep-9, -0x1.6d93f3f1f0543p-10, 0x1.b7000000052p-1}, /* i=219 67.149 */
  {0x1.6e5bcbd9f4eep+0, -0x1.016a622ba2236p-3, 0x1.edc881c34feeep-6, -0x1.3a61a0b56a123p-7, 0x1.ca76b68d0ce06p-9, -0x1.6a06bd3b3a14p-10, 0x1.b90000000a39cp-1}, /* i=220 67.338 */
  {0x1.6e3ba6425edefp+0, -0x1.00ef2ae0eae1p-3, 0x1.ebf246498e5a4p-6, -0x1.3898ed2bf3c2ap-7, 0x1.c6f20b0de6a64p-9, -0x1.6684330da1d9bp-10, 0x1.bafffffff683bp-1}, /* i=221 68.184 */
  {0x1.6e1b900a5a905p+0, -0x1.007468cfa00edp-3, 0x1.ea1eb53baa2cep-6, -0x1.36d3b9eec2e4fp-7, 0x1.c37618b84c80ap-9, -0x1.630c2fc9fc55p-10, 0x1.bd00000006378p-1}, /* i=222 69.345 */
  {0x1.6dfb89234e8ddp+0, -0x1.fff4369bb4117p-4, 0x1.e84dc95fe24adp-6, -0x1.3511fe51f47fcp-7, 0x1.c002c56995215p-9, -0x1.5f9e8e6e511ep-10, 0x1.bf000000080d4p-1}, /* i=223 67.658 */
  {0x1.6ddb917eb3508p+0, -0x1.ff008365e33e6p-4, 0x1.e67f7d8934c9fp-6, -0x1.3353b1c36c67bp-7, 0x1.bc97f75825b5cp-9, -0x1.5c3b2a877b81ep-10, 0x1.c0fffffffe3c1p-1}, /* i=224 68.43 */
  {0x1.6dbba90e13891p+0, -0x1.fe0db6aefd6a9p-4, 0x1.e4b3cc973faa2p-6, -0x1.3198cbca81b7fp-7, 0x1.b9359519013d6p-9, -0x1.58e1e0376e6cp-10, 0x1.c300000017b4p-1}, /* i=225 66.926 */
  {0x1.6d9bcfc31739cp+0, -0x1.fd1bcf2affcdap-4, 0x1.e2eab176bcc6cp-6, -0x1.2fe14408413aap-7, 0x1.b5db859b6eef1p-9, -0x1.55928c33413adp-10, 0x1.c4fffffffb533p-1}, /* i=226 67.063 */
  {0x1.6d7c058f6fabfp+0, -0x1.fc2acb901799p-4, 0x1.e12427203e2ebp-6, -0x1.2e2d123601c35p-7, 0x1.b289b01dad58p-9, -0x1.524d0bb7aa9b4p-10, 0x1.c70000000725cp-1}, /* i=227 69.83 */
  {0x1.6d5c4a64eb89bp+0, -0x1.fb3aaa9735b61p-4, 0x1.df6028992c1e8p-6, -0x1.2c7c2e26255f4p-7, 0x1.af3ffc3d936e3p-9, -0x1.4f113c93cd1f2p-10, 0x1.c90000000afp-1}, /* i=228 66.688 */
  {0x1.6d3c9e356adc1p+0, -0x1.fa4b6afbae969p-4, 0x1.dd9eb0f2f62aap-6, -0x1.2ace8fc3200bdp-7, 0x1.abfe51e903dep-9, -0x1.4bdefd185a769p-10, 0x1.cb000000069d4p-1}, /* i=229 66.781 */
  {0x1.6d1d00f2e3d2ap+0, -0x1.f95d0b7b5983ap-4, 0x1.dbdfbb4b3590ep-6, -0x1.29242f0f670ccp-7, 0x1.a8c49961d706dp-9, -0x1.48b62c1fbb358p-10, 0x1.ccffffffdab99p-1}, /* i=230 66.632 */
  {0x1.6cfd728f57671p+0, -0x1.f86f8ad63635fp-4, 0x1.da2342caeb86dp-6, -0x1.277d0424876f2p-7, 0x1.a592bb3bbc436p-9, -0x1.4596a9079e557p-10, 0x1.cefffffffe2d1p-1}, /* i=231 67.801 */
  {0x1.6cddf2fce8cddp+0, -0x1.f782e7cf189dap-4, 0x1.d86942a7aaf4bp-6, -0x1.25d9073411a53p-7, 0x1.a268a06075d58p-9, -0x1.428053afdd73p-10, 0x1.d0ffffffffcb7p-1}, /* i=232 66.661 */
  {0x1.6cbe822dc67a3p+0, -0x1.f697212af6f8ep-4, 0x1.d6b1b622336bdp-6, -0x1.2438308615e39p-7, 0x1.9f4632000f065p-9, -0x1.3f730c759eb2cp-10, 0x1.d2fffffff8613p-1}, /* i=233 68.561 */
  {0x1.6c9f201432a22p+0, -0x1.f5ac35b125683p-4, 0x1.d4fc9886c99f3p-6, -0x1.229a78794a279p-7, 0x1.9c2b59a1fbc59p-9, -0x1.3c6eb43676924p-10, 0x1.d4fffffffe829p-1}, /* i=234 67.111 */
  {0x1.6c7fcca286aafp+0, -0x1.f4c2242b6adccp-4, 0x1.d349e52d4717ap-6, -0x1.20ffd782eafb8p-7, 0x1.9918011615bc1p-9, -0x1.39732c46b55d7p-10, 0x1.d6ffffffec17bp-1}, /* i=235 67.524 */
  {0x1.6c6087cb29418p+0, -0x1.f3d8eb65b2754p-4, 0x1.d19997787148ap-6, -0x1.1f68462dee925p-7, 0x1.960c12776bf74p-9, -0x1.368056757a8e7p-10, 0x1.d8fffffffdc37p-1}, /* i=236 68.433 */
  {0x1.6c4151809b324p+0, -0x1.f2f08a2e66ea4p-4, 0x1.cfebaad68cd8ap-6, -0x1.1dd3bd1b6361ep-7, 0x1.9307783279516p-9, -0x1.3396150c35cep-10, 0x1.dafffffffe8afp-1}, /* i=237 67.074 */
  {0x1.6c2229b56bc1ep+0, -0x1.f208ff561717cp-4, 0x1.ce401ac09dcb7p-6, -0x1.1c4235018f579p-7, 0x1.900a1cf371299p-9, -0x1.30b44abfbe7b8p-10, 0x1.dd00000004228p-1}, /* i=238 67.507 */
  {0x1.6c03105c3fb8bp+0, -0x1.f12249afa616cp-4, 0x1.cc96e2baaac6ap-6, -0x1.1ab3a6ac04284p-7, 0x1.8d13ebb84516p-9, -0x1.2ddadac03227dp-10, 0x1.defffffff9323p-1}, /* i=239 67.417 */
  {0x1.6be40567cbc8bp+0, -0x1.f03c68101d418p-4, 0x1.caeffe53529ep-6, -0x1.19280afb10e73p-7, 0x1.8a24cfc32e37fp-9, -0x1.2b09a8a98a8d4p-10, 0x1.e0fffffff7195p-1}, /* i=240 69.163 */
  {0x1.6bc508cad8c84p+0, -0x1.ef57594ec74e8p-4, 0x1.c94b6923e919ep-6, -0x1.179f5ae3b3467p-7, 0x1.873cb49a058afp-9, -0x1.2840988971172p-10, 0x1.e2fffffffe1d7p-1}, /* i=241 67.258 */
  {0x1.6ba61a784300fp+0, -0x1.ee731c4526d7ap-4, 0x1.c7a91ed050647p-6, -0x1.16198f6f4a30ap-7, 0x1.845b860cef905p-9, -0x1.257f8ed783e0cp-10, 0x1.e4fffffffe605p-1}, /* i=242 67.616 */
  {0x1.6b873a62f7b52p+0, -0x1.ed8fafcedfd4fp-4, 0x1.c6091b06bb024p-6, -0x1.1496a1bb335b5p-7, 0x1.8181302adce42p-9, -0x1.22c670778b71cp-10, 0x1.e6fffffffe98fp-1}, /* i=243 68.171 */
  {0x1.6b68687df638fp+0, -0x1.ecad12c9bb975p-4, 0x1.c46b597f9ec6ep-6, -0x1.13168af8974b9p-7, 0x1.7ead9f49835ebp-9, -0x1.201522b5672afp-10, 0x1.e900000007c9ep-1}, /* i=244 72.216 */
  {0x1.6b49a4bc4fb6p+0, -0x1.ebcb4415a2f0cp-4, 0x1.c2cfd5fd96014p-6, -0x1.1199446c255f4p-7, 0x1.7be0c0028a30fp-9, -0x1.1d6b8b470eb93p-10, 0x1.eb00000026f7cp-1}, /* i=245 68.179 */
  {0x1.6b2aef112d00ep+0, -0x1.eaea4294c4d8ap-4, 0x1.c1368c4d91cc1p-6, -0x1.101ec76e1b77p-7, 0x1.791a7f2f3835ep-9, -0x1.1ac990460631fp-10, 0x1.ed00000009a3p-1}, /* i=246 69.91 */
  {0x1.6b0c476fbe603p+0, -0x1.ea0a0d2b1b796p-4, 0x1.bf9f7845e6a74p-6, -0x1.0ea70d693e964p-7, 0x1.765ac9e3dffd1p-9, -0x1.182f182c2427p-10, 0x1.ef00000009eb2p-1}, /* i=247 69.004 */
  {0x1.6aedadcb4d608p+0, -0x1.e92aa2beeaea9p-4, 0x1.be0a95c71fc7ap-6, -0x1.0d320fdb78e7dp-7, 0x1.73a18d7c2fff8p-9, -0x1.159c09dd51cedp-10, 0x1.f1000000035e2p-1}, /* i=248 68.261 */
  {0x1.6acf221732c71p+0, -0x1.e84c023873904p-4, 0x1.bc77e0bb5eb3dp-6, -0x1.0bbfc85520676p-7, 0x1.70eeb78ead8adp-9, -0x1.13104c96b09ccp-10, 0x1.f2fffffff84fbp-1}, /* i=249 68.02 */
  {0x1.6ab0a446d8578p+0, -0x1.e76e2a81fb3cfp-4, 0x1.bae75516591d6p-6, -0x1.0a503078d0ap-7, 0x1.6e4235ef1b0f1p-9, -0x1.108bc7f6b7ce9p-10, 0x1.f4fffffff243dp-1}, /* i=250 68.024 */
  {0x1.6a92344db8424p+0, -0x1.e6911a87c53d3p-4, 0x1.b958eed537f21p-6, -0x1.08e341fb28929p-7, 0x1.6b9bf6aaea58p-9, -0x1.0e0e63fb4c524p-10, 0x1.f70000000970ap-1}, /* i=251 68.899 */
  {0x1.6a73d21f61d8p+0, -0x1.e5b4d13830a31p-4, 0x1.b7cca9febb7c7p-6, -0x1.0778f6a2c8577p-7, 0x1.68fbe8132a293p-9, -0x1.0b9808f9b60c4p-10, 0x1.f90000001379p-1}, /* i=252 68.528 */
  {0x1.6a557daf74927p+0, -0x1.e4d94d8390583p-4, 0x1.b64282a2e11b8p-6, -0x1.06114847db6b8p-7, 0x1.6661f8a9de40fp-9, -0x1.09289fa315b09p-10, 0x1.fafffffff4a35p-1}, /* i=253 71.829 */
  {0x1.6a3736f19a9c5p+0, -0x1.e3fe8e5bfff7fp-4, 0x1.b4ba74da84296p-6, -0x1.04ac30d39fd77p-7, 0x1.63ce173306a02p-9, -0x1.06c01103127f8p-10, 0x1.fcfffffffa751p-1}, /* i=254 70.154 */
  {0x1.6a18fde8aede9p+0, -0x1.e32493228aa7ep-4, 0x1.b3347d8a64a17p-6, -0x1.0349aaf167477p-7, 0x1.614033ed086e9p-9, -0x1.045e47a89e4cp-10, 0x1.fefffeffffb0fp-1}, /* i=255 69.757 */
};

/* The following is a table of error bounds for the fast path, constructed
   by get_max_err() in acospi.sage */
static const double Err[256] = {0x1.47p-62, 0x1.64p-63, 0x1.67p-63, 0x1.55p-63, 0x1.51p-63, 0x1.5dp-63, 0x1.5ep-63, 0x1.66p-63, 0x1.69p-63, 0x1.6dp-63, 0x1.6dp-63, 0x1.64p-63, 0x1.6cp-63, 0x1.66p-63, 0x1.57p-63, 0x1.62p-63, 0x1.6fp-63, 0x1.64p-63, 0x1.61p-63, 0x1.66p-63, 0x1.67p-63, 0x1.54p-63, 0x1.75p-63, 0x1.5fp-63, 0x1.68p-63, 0x1.64p-63, 0x1.67p-63, 0x1.6dp-63, 0x1.73p-63, 0x1.73p-63, 0x1.72p-63, 0x1.61p-63, 0x1.7cp-63, 0x1.76p-63, 0x1.7ap-63, 0x1.68p-63, 0x1.74p-63, 0x1.7ap-63, 0x1.6cp-63, 0x1.71p-63, 0x1.79p-63, 0x1.7ap-63, 0x1.7p-63, 0x1.7ap-63, 0x1.65p-63, 0x1.71p-63, 0x1.7ep-63, 0x1.73p-63, 0x1.73p-63, 0x1.73p-63, 0x1.6ep-63, 0x1.6ap-63, 0x1.71p-63, 0x1.79p-63, 0x1.62p-63, 0x1.79p-63, 0x1.6bp-63, 0x1.71p-63, 0x1.6dp-63, 0x1.95p-63, 0x1.83p-63, 0x1.83p-63, 0x1.93p-63, 0x1.86p-63, 0x1.8p-63, 0x1.9p-63, 0x1.9p-63, 0x1.88p-63, 0x1.87p-63, 0x1.91p-63, 0x1.73p-63, 0x1.76p-63, 0x1.8dp-63, 0x1.83p-63, 0x1.93p-63, 0x1.78p-63, 0x1.7dp-63, 0x1.9p-63, 0x1.8p-63, 0x1.92p-63, 0x1.95p-63, 0x1.95p-63, 0x1.85p-63, 0x1.8cp-63, 0x1.7ep-63, 0x1.83p-63, 0x1.83p-63, 0x1.8p-63, 0x1.7ep-63, 0x1.7fp-63, 0x1.81p-63, 0x1.74p-63, 0x1.8cp-63, 0x1.79p-63, 0x1.7ep-63, 0x1.88p-63, 0x1.7ap-63, 0x1.83p-63, 0x1.8ap-63, 0x1.7fp-63, 0x1.b5p-63, 0x1.abp-63, 0x1.a6p-63, 0x1.b4p-63, 0x1.bp-63, 0x1.bbp-63, 0x1.adp-63, 0x1.a8p-63, 0x1.a5p-63, 0x1.b2p-63, 0x1.b3p-63, 0x1.b5p-63, 0x1.bbp-63, 0x1.acp-63, 0x1.b8p-63, 0x1.b4p-63, 0x1.bap-63, 0x1.b2p-63, 0x1.a9p-63, 0x1.a9p-63, 0x1.b7p-63, 0x1.bcp-63, 0x1.a8p-63, 0x1.b9p-63, 0x1.b7p-63, 0x1.afp-63, 0x1.b7p-63, 0x1.aep-63, 0x1.aep-63, 0x1.b5p-63, 0x1.b1p-63, 0x1.a4p-63, 0x1.a6p-63, 0x1.b5p-63, 0x1.bdp-63, 0x1.c1p-63, 0x1.afp-63, 0x1.b4p-63, 0x1.a6p-63, 0x1.aep-63, 0x1.bp-63, 0x1.b4p-63, 0x1.afp-63, 0x1.abp-63, 0x1p-62, 0x1.02p-62, 0x1.05p-62, 0x1p-62, 0x1.f8p-63, 0x1.06p-62, 0x1.05p-62, 0x1p-62, 0x1.04p-62, 0x1.05p-62, 0x1.05p-62, 0x1.05p-62, 0x1.05p-62, 0x1.05p-62, 0x1.03p-62, 0x1.03p-62, 0x1.03p-62, 0x1.0bp-62, 0x1.0ep-62, 0x1.03p-62, 0x1.07p-62, 0x1.0ap-62, 0x1.09p-62, 0x1.11p-62, 0x1.0ep-62, 0x1.0fp-62, 0x1.11p-62, 0x1.1p-62, 0x1.0fp-62, 0x1.11p-62, 0x1.18p-62, 0x1.16p-62, 0x1.15p-62, 0x1.22p-62, 0x1.1fp-62, 0x1.24p-62, 0x1.1fp-62, 0x1.78p-62, 0x1.83p-62, 0x1.83p-62, 0x1.89p-62, 0x1.89p-62, 0x1.92p-62, 0x1.9dp-62, 0x1.a5p-62, 0x1.a9p-62, 0x1.bp-62, 0x1.bdp-62, 0x1.71p-66, 0x1.72p-66, 0x1.5bp-66, 0x1.7p-66, 0x1.6dp-66, 0x1.7ap-66, 0x1.7dp-66, 0x1.4bp-66, 0x1.58p-66, 0x1.4fp-66, 0x1.5fp-66, 0x1.48p-66, 0x1.5bp-66, 0x1.3cp-66, 0x1.4ap-66, 0x1.4p-66, 0x1.53p-66, 0x1.48p-66, 0x1.52p-66, 0x1.32p-66, 0x1.33p-66, 0x1.29p-66, 0x1.34p-66, 0x1.38p-66, 0x1.31p-66, 0x1.21p-66, 0x1.24p-66, 0x1.1dp-66, 0x1.15p-66, 0x1.04p-66, 0x1.ecp-67, 0x1.05p-66, 0x1.05p-67, 0x1.39p-67, 0x1.2ep-67, 0x1.cfp-68, 0x1.3dp-67, 0x1.33p-67, 0x1.3ap-67, 0x1.f6p-68, 0x1.2fp-67, 0x1.bfp-68, 0x1.0bp-67, 0x1.e8p-68, 0x1.aap-68, 0x1.ffp-68, 0x1.cdp-68, 0x1.cbp-68, 0x1.6bp-68, 0x1.c5p-68, 0x1.9cp-68, 0x1.6fp-68, 0x1.1fp-68, 0x1.57p-68, 0x1.18p-68, 0x1.1ep-68, 0x1.2bp-68, 0x1.29p-68, 0x1.18p-68, 0x1.c5p-69, 0x1.b8p-69, 0x1.26p-69, 0x1.0cp-69, 0x1.a7p-70};

// pi_hi + pi_lo approximates pi with error < 2^-108.04
static const double pi_hi = 0x1.921fb54442d18p1;
static const double pi_lo = 0x1.1a62633145c07p-53;

/* For the accurate path, use polynomials of degree DEGREE (thus with DEGREE+1
   coefficients), with coefficients of degree < LARGE represented as
   double-double, and coefficients of degree >= LARGE as double only.
   Thus each polynomial needs DEGREE+LARGE+1 'doubles'. */
#define DEGREE 11
#define LARGE 8

/* Each entry contains a degree 11-polynomial and the evaluation point xmid,
   so that:
   * for |x| < 0.5, i.e., 0 <= i < 64, acos(x)-pi/2 ~ p(x-xmid);
   * for 0.5 <= |x| < 1, acos(1-x) ~ sqrt(x)*p(x-xmid).
   The coefficients of degree < LARGE are stored as two double numbers (most
   significant first).
   Generated with:
   Gen_P_aux(11,128,threshold=64,large=8,verbose=true,slow=1,prec=128). */
static const double T2[128][DEGREE+LARGE+2] = {
  { 0x7.3f7b5c6f3abcp-120, 0x6.aa86b9baf9cab614bp-176, -0x1p+0, -0x4.045deep-104, 0x5.e6990ab0c2dd8p-92, -0x7.40b83a2b1b71454c98p-148, -0x2.aaaaaaaaaaaaap-4, -0xa.aaaae0ef10118e456p-60, 0x1.0160b49131a62p-68, -0x7.963a9045d1a6daedf2p-124, -0x1.3333333333333p-4, -0x6.015892f570086cb23ep-60, 0x4.f0883d2b50248p-52, -0xc.e10e4b3894d2b935c8p-108, -0xb.6db6db6e11cd8p-8, 0x2.bcd79079f2c4abcc19p-60, 0x4.4f35e4ec4bd2p-36, -0x7.c71c9380dc4fcp-8, 0x9.eba442f49964p-24, -0x5.bb98b11f1845p-8, 0x0p+0 }, /* i=0 117.1 */
  { -0x3.0004801239fb4p-8, 0x1.c7305887fe50696224p-64, -0x1.0004801e60e3dp+0, -0x7.023ec56b5ce7e77d04p-56, -0x1.801440e3d9586p-8, -0x6.9ef9beeedeb98a93f8p-64, -0x2.ab16b2941ef82p-4, 0xb.58b641169bbd0d67e8p-60, -0x1.202a33a27b15fp-8, -0x2.6b29211b91f914266p-68, -0x1.33ba46964a456p-4, 0x4.828c4ec205b6156d5ap-60, -0xf.044f1d0a0794p-12, 0x3.3528852e9ea44128f5p-64, -0xb.779131c48b988p-8, 0x2.06e1efb066f0b5a5fdp-60, -0xd.263c033a0cbbp-12, -0x7.d2335f752895p-8, -0xb.d88311b4bd9ep-12, -0x5.c672cfed3749p-8, 0x3p-8 }, /* i=1 115.6 */
  { -0x5.0014d63fc2f6p-8, 0x9.a9afbebc108d40843p-64, -0x1.000c80ea73147p+0, -0x1.603503f2bf69be97eep-56, -0x2.805dcb72fdedep-8, -0x7.ac245975dcd852336p-64, -0x2.abd6e7bd684fcp-4, 0x7.88b3d8f7ca980ee054p-60, -0x1.e0c37ec3b2803p-8, -0x5.22092fe7b71318476cp-64, -0x1.34aac8e3e936fp-4, -0x1.77055e8d9b0c603406p-60, -0x1.913f80fa00244p-8, -0x6.0cc691b2f88cf6cdcap-64, -0xb.8920e8cb460dp-8, 0x2.12732af294d8d7d6fp-60, -0x1.5fce7e060e262p-8, -0x7.e5fdcce61d648p-8, -0x1.3d71c9158346fp-8, -0x5.dc43c52ba8a18p-8, 0x5p-8 }, /* i=2 114.8 */
  { -0x7.00392f97c0c1p-8, -0x1.8d9822a9352bd84d68p-60, -0x1.00188384efb56p+0, -0x1.9e38be72f1d290f8fep-56, -0x3.81017d9a51848p-8, 0xf.fcc8c7cd6f1c44381cp-64, -0x2.acf7956d0c9c2p-4, 0x7.ddc8ba05bfed073b6cp-60, -0x2.a218ebb0302p-8, -0xe.00782f64664d278648p-64, -0x1.361472d1cef2cp-4, 0x5.0c23d206cb3877fa52p-60, -0x2.336e056048622p-8, -0x1.048359d7b3775c4604p-64, -0xb.a3944f235132p-8, -0x1.12ae0f735fdf980c9p-60, -0x1.eef7e59d50c2bp-8, -0x8.03dc4d4a96668p-8, -0x1.bfb5e8527c3b4p-8, -0x5.fd45609a9ba9p-8, 0x7p-8 }, /* i=3 114.3 */
  { -0x9.0079914fef96p-8, 0x7.9919e3720f5e815e2p-64, -0x1.0028899ee96f2p+0, 0x1.f02d504dc33579b604p-56, -0x4.8223988e5f804p-8, 0x1.24e896e2c72391bde8p-64, -0x2.ae792caf62116p-4, -0x5.23e2c1d0e9d717da98p-60, -0x3.6476851ae3a1p-8, -0xa.bf28a954b0a89afd8p-68, -0x1.37f85a3916f82p-4, -0x7.37fa8a155055dd7a5ep-60, -0x2.d74dd09a504ap-8, -0xf.065d667385196c9ee8p-64, -0xb.c70d046f05548p-8, -0x1.e9e639fb20fcf44c84p-60, -0x2.809742069052p-8, -0x8.2c07a71b58808p-8, -0x2.454e4a438e228p-8, -0x6.29cf5a1f56cp-8, 0x9p-8 }, /* i=4 113.9 */
  { -0xb.00de0491731ap-8, -0x2.33c42de1b15e0e5137p-60, -0x1.003c957ad60cbp+0, 0x2.e8ebcafcc589a16c38p-56, -0x5.83e88f0f3777p-8, 0x1.0ca00567fa6daae4dp-60, -0x2.b05c44c66c612p-4, -0x9.43d6bdadd054bcc944p-60, -0x4.282920e5796c4p-8, 0xc.a9d711debd8dcd7f08p-64, -0x1.3a57f3b4be11ap-4, -0x1.0b22af1783f22e176ap-60, -0x3.7d5e5bcc9102p-8, -0xe.b26e9187cbbc2f8c64p-64, -0xb.f3b840a1318ep-8, -0x1.9da6e2fa8461507dcap-60, -0x3.15688dc464eeap-8, -0x8.5ecc775a3bfb8p-8, -0x2.cf3d87d2daedcp-8, -0x6.6258878f46474p-8, 0xbp-8 }, /* i=5 113.6 */
  { -0xd.016e979c75fbp-8, 0x2.42eeb4bd53f53ac77bp-60, -0x1.0054a9ed716d4p+0, 0x6.a4f0e93b528c03909ep-56, -0x6.867513d1768d4p-8, 0x7.f61f66a00ca15692c8p-64, -0x2.b2a19b9e59876p-4, 0x4.9efcb191227740e90cp-60, -0x4.ed7e9be790fb4p-8, -0x1.5956c1dc457be11ep-64, -0x1.3d35147cb93b5p-4, 0xe.cbf0a41a4c5245662p-64, -0x4.2621ec3ede32cp-8, -0x1.f8d91354acd800b6068p-60, -0xc.29cf2a40e5b38p-8, -0x3.0c42f9cf57cd1f32fdp-60, -0x3.ae2ddec277414p-8, -0x8.9c8bfe0a25d4p-8, -0x3.5e91d54f523b2p-8, -0x6.a77883a9acc4cp-8, 0xdp-8 }, /* i=6 113.3 */
  { -0xf.02335eedc94ep-8, 0x2.1eba39d3cb0b0724d5p-60, -0x1.0070ca5eda78bp+0, 0x4.4698a04828924874e8p-56, -0x7.89ee2818a4314p-8, -0x1.74c41536e8ac2c8f83p-60, -0x2.b54a166032e88p-4, 0x9.7f98a8a1503dd5dab4p-60, -0x5.b4c61705d932p-8, -0x1.7a7d3b17e346b366d88p-60, -0x1.4091f4be69f0dp-4, 0x5.e5e6853ed886151de8p-60, -0x4.d21e3d4b4b37cp-8, -0x1.d321a1f6c23dd76398p-60, -0xc.699743e9e5b78p-8, 0x1.6d23f3e8c1b2b8e291p-60, -0x4.4bb0e2a02b81p-8, -0x8.e5bd23096b05p-8, -0x3.f467e9c697c0cp-8, -0x6.f9e9ccbf379cp-8, 0xfp-8 }, /* i=7 113.1 */
  { -0x1.103347666f892p-4, 0x5.ac543b551ba7a06cep-64, -0x1.0090facbeaf37p+0, 0x3.e9d3c9cdc30da5abccp-56, -0x8.8e792a8e57c6p-8, -0x2.d75c9c599f2963dac1p-60, -0x2.b856c2237e82ep-4, 0xc.bedd5ddd6414dcfd4p-64, -0x6.7e5035e8c7bcp-8, -0x5.838a49ddecc6b7b438p-64, -0x1.44713276eea77p-4, 0x5.ae8683b5279e546554p-60, -0x5.81dd30f34aa1p-8, -0x1.99d60dd06b9805dcd28p-60, -0xc.b362f22f93a8p-8, 0x2.858dff82832e431499p-60, -0x4.eec46f68d846p-8, -0x9.3aedb6f4a101p-8, -0x4.91ee1a7de9498p-8, -0x7.5a8c64dcf1858p-8, 0x1.1p-4 }, /* i=8 112.9 */
  { -0x1.3047a02794911p-4, 0x7.5ac7a969f75addd40ep-60, -0x1.00b53fc7cb3bdp+0, -0x7.5d094975c91de18538p-56, -0x9.943be661eacc8p-8, -0x1.876d7d06081e9e5ac5p-60, -0x2.bbc8d4bfb4916p-4, 0x3.16d54e90819c9634e4p-60, -0x7.4a6f5fa5bf064p-8, -0x1.8ad6654a718ae19b4a8p-60, -0x1.48d5d4d4ce7fdp-4, 0x6.3d5871f3fa3128f8e6p-60, -0x6.35ed8879fad14p-8, -0x1.31014e960e85e3b368p-60, -0xd.07921b4bbbd58p-8, 0x2.c730a8b0ffd5c4d042p-60, -0x5.98462d19700bp-8, -0x9.9cc3f496c07b8p-8, -0x5.3867b62c3bf9cp-8, -0x7.ca68ffa1a3448p-8, 0x1.3p-4 }, /* i=9 112.7 */
  { -0x1.5060c31541da5p-4, -0x6.764f6ab0927a4a3a52p-60, -0x1.00dd9e7dc32bcp+0, 0x3.059549c81809fe094p-56, -0xa.9b5ca2bcd89c8p-8, 0x1.a4f72628dbed900aedp-60, -0x2.bfa1adbe985p-4, 0x3.aa66eb723afdfa9d4cp-60, -0x8.197801b139b9p-8, -0x2.11f0447a80867a97a5p-60, -0x1.4dc350278e557p-4, -0x3.ba7f55aaac29deb9d2p-60, -0x6.eee3a6784cd04p-8, 0x1.af0cc75f9c608cc0ccp-60, -0xd.6692e24b6478p-8, 0x2.7700bfae94b563b806p-60, -0x6.49205cad4c6dp-8, -0xa.0c00485f61528p-8, -0x5.e930acc157e9p-8, -0x8.4ab4ccac41998p-8, 0x1.5p-4 }, /* i=10 112.5 */
  { -0x1.707f33c173a99p-4, 0x4.219874125e4d6fbc06p-60, -0x1.010a1cb349899p+0, -0x5.248e08ea83600a2dep-60, -0xb.a40232985a81p-8, -0x4.3d23c1089ff87ee2p-68, -0x2.c3e2d770aef52p-4, -0x2.d8d086aabe3c43f278p-60, -0x8.ebc0d57504af8p-8, 0x1.545af62ae17c7823ap-64, -0x1.533d8a64de43dp-4, -0x4.a0c9b8699b39a42e7ap-60, -0x7.ad5a5bf99080cp-8, -0x1.820d8993ddddab4bc68p-60, -0xd.d0e27fbaa24b8p-8, -0x3.683aa4518d38a3351bp-60, -0x7.024bc1d797d34p-8, -0xa.897f64655fe28p-8, -0x6.a5c191ee2fb8p-8, -0x8.dcd5f08f398f8p-8, 0x1.7p-4 }, /* i=11 112.3 */
  { -0x1.90a376809684p-4, -0x5.2435c5b68d2c3a5204p-60, -0x1.013ac0ca53a83p+0, 0x2.1f08fb5d1d627cc792p-56, -0xc.ae5405014de58p-8, 0x1.56ab9d57cd0e1038f6p-60, -0x2.c88e08253eb86p-4, 0xf.344d66e572456c1bacp-60, -0x9.c1a328eabd9p-8, 0x1.24ab251dba978ccee6p-60, -0x1.5948e04c44b89p-4, -0x5.d8f7f5babd05ee859p-60, -0x8.71f3c24d51c88p-8, 0x2.6188a395951b5c9dcfp-60, -0xe.470e3a26adb5p-8, -0x5.231dd02d10e247984p-64, -0x7.c4d1b50ea10e4p-8, -0xb.163ca8a8aaaap-8, -0x7.6fb40a52fedfcp-8, -0x9.8268c1b2fba68p-8, 0x1.9p-4 }, /* i=12 112.1 */
  { -0x1.b0ce107d3f69p-4, -0x7.c8e4c30e758d9ec1dap-60, -0x1.016f91c3e7096p+0, -0x5.f82ca03392f122b39p-56, -0xd.ba7a35d8032p-8, 0x3.9ee9e0d1178da88f4ep-60, -0x2.cda52387715p-4, -0x2.6022c3923504cec20cp-60, -0xa.9b7b2aa0c284p-8, 0xf.4238579cf985fd2b2p-64, -0x1.5fea2b337ac83p-4, -0xa.a66c4594b7aabdaap-68, -0x9.3d5a235ec4ca8p-8, -0x1.3d91ea217005d02f42p-60, -0xe.c9b4812c5a9ep-8, 0x4.f45716eb5784996d2p-64, -0x8.91ce642495688p-8, -0xb.b354e8974117p-8, -0x8.48c7b53893098p-8, -0xa.3d45e2882bd5p-8, 0x1.bp-4 }, /* i=13 112.0 */
  { -0x1.d0ff87cc3a7a5p-4, 0x2.ddab47f13b2d58bef2p-60, -0x1.01a89742ef0f4p+0, 0x5.2b6ac9787ebb47ac52p-56, -0xe.c89d9f1a377dp-8, -0x1.f159171ce24c99081ep-60, -0x2.d32a3c226a11p-4, 0x9.463d6bfe442e0ea61cp-60, -0xb.79a839968fc1p-8, -0xf.e837e6f2394f3a8f9p-64, -0x1.6726c78718976p-4, -0x5.7ae9796351e274d83ep-60, -0xa.1040f28ac7e8p-8, 0x8.0cee182f857e4799p-68, -0xf.59862e3224898p-8, -0xc.e5e7d1e4563487d4bp-64, -0x9.6a73485883dcp-8, -0xc.620998566504p-8, -0x9.32e7a71a1b99p-8, -0xb.0f8956f3b24cp-8, 0x1.dp-4 }, /* i=14 111.8 */
  { -0x1.f1386380f8b9ap-4, -0x4.9c5b12bdd5536e8c08p-60, -0x1.01e5d98f59023p+0, -0x4.45eff70ad666bfde72p-56, -0xf.d8e7eac635b18p-8, -0x3.a68cad6f53b93ead07p-60, -0x2.d91f950e583c8p-4, 0x6.932e39fb5cc7aa1a94p-60, -0xc.5c8d396544bf8p-8, 0xe.79e7b00e522176f7fp-64, -0x1.6f049c0cbc9c2p-4, 0x2.6d9229c6d4b46e8228p-60, -0xa.eb65d81c5a4p-8, 0x3.fad11ca5521eeaa6cdp-60, -0xf.f747ee61e58e8p-8, 0x2.df6aa762e23fe3e99ap-60, -0xa.5009d9967f258p-8, -0xd.23c46e0185468p-8, -0xa.30308c0f8f3e8p-8, -0xb.fb9ab830b4d58p-8, 0x1.fp-4 }, /* i=15 111.6 */
  { -0x2.11792bc265cdep-4, -0x6.328f56db8108a55208p-60, -0x1.0227619978f0bp+0, 0x3.0656c72a57a6533c08p-56, -0x1.0eb83a566e63dp-4, 0x7.251e63a30fa9cfc33ap-60, -0x2.df87a3c8ca8dp-4, 0x9.883cde197831344cd8p-60, -0xd.4490eb31cb188p-8, -0xe.168557f111584a942p-64, -0x1.778a21f58a06dp-4, -0x3.3e6e5784ce083ad74p-60, -0xb.cf91d1ccffee8p-8, -0x3.9f7367505278b48c68p-60, -0x1.0a3d3d9f8f18ep-4, 0x7.6ebf6d08dabbd16c54p-60, -0xb.43f6878a23f98p-8, -0xd.fa1b84fd953c8p-8, -0xb.42f78c71bdc8p-8, -0xd.0436b08dd4288p-8, 0x2.1p-4 }, /* i=16 111.5 */
  { -0x2.31c269e02f15ep-4, -0xf.41b37d98e3a044cdp-60, -0x1.026d38fdba2bep+0, -0x7.794dc2a8f64a0c491p-56, -0x1.2009c51597f42p-4, -0x4.6f04486e9add4e2774p-60, -0x2.e665123aca37cp-4, -0x4.6ef3df6bbc32896facp-60, -0xe.321e4beff543p-8, -0x3.350188a881b79f85efp-60, -0x1.80be6dd1b731p-4, -0x2.eb9e3c4f6c9c44d032p-60, -0xc.bd9a6ae3b949p-8, 0xb.5ed30daa57b2eb371p-64, -0x1.1601b3d97dadp-4, -0x5.8714b39c36f458825p-60, -0xc.47bc023855868p-8, -0xe.e6d613b3f3a78p-8, -0xc.6dd211ec9b468p-8, -0xe.2c79ef890a248p-8, 0x2.3p-4 }, /* i=17 111.3 */
  { -0x2.5214a86885ad4p-4, 0x1.d1c5881b1ed7659904p-60, -0x1.02b76a089e6c7p+0, 0x3.16a7a2c3fc121f5c98p-56, -0x1.3185e7ade82bfp-4, -0xe.91cc6b81d3e03829cp-64, -0x2.edbac0ef89fdcp-4, 0x7.56c7a4942c839bc0cp-64, -0xf.25a4f889006fp-8, -0x2.a50982f79cd284c945p-60, -0x1.8aa93977f279bp-4, 0x5.479cb46e445b88c8c4p-60, -0xd.b6630ed2dfa1p-8, 0x3.fd7d4834c1b6bc186cp-60, -0x1.22d289ae01667p-4, 0x7.7eb10a5064cc1cbf28p-60, -0xd.5cfedd111bab8p-8, -0xf.ebf1c6900fc18p-8, -0xd.b39e8fba3c12p-8, -0xf.77edcf4c490f8p-8, 0x2.5p-4 }, /* i=18 111.1 */
  { -0x2.7270733e656b8p-4, 0xb.af991a19247bcb8204p-60, -0x1.0305ffbb0ef02p+0, 0xc.dca961eab7a38a93cp-60, -0x1.432f7cd08d4c2p-4, -0x7.7bc78583db43102ed6p-60, -0x2.f58bc97ec2268p-4, 0xc.56636f9e4c8336df38p-60, -0x1.01f9998845356p-4, -0x7.4f8b620d04d1bd813ep-60, -0x1.9552eef5a94d6p-4, -0x3.10dccd6c8321f3cf9cp-60, -0xe.bade79847de38p-8, 0x3.102e8058f824009ee1p-60, -0x1.30c21e667e797p-4, -0x3.a7900ba2879eab74b4p-60, -0xe.858998d793b78p-8, -0x1.10ba8d60520b3p-4, -0xf.178e76104ce98p-8, -0x1.0ea96ee54d91cp-4, 0x2.7p-4 }, /* i=19 111.0 */
  { -0x2.92d657b06b572p-4, 0x1.5f053e118fc42bc28cp-60, -0x1.035905cf0331fp+0, -0x5.f6db1637cbc28a235ap-56, -0x1.55097279eec0cp-4, -0x2.e403f26c56fcd84e3ep-60, -0x2.fddb812e2519p-4, -0x8.7eeb9bac4d70bed068p-60, -0x1.120764fdf4128p-4, -0x6.e6c6a98a5af9c13ee2p-60, -0x1.a0c4b49fc1d9cp-4, 0x7.b242896efd644807acp-60, -0xf.cc1048ded555p-8, -0x2.bc0e71168febf440f9p-60, -0x1.3fe4b09dfb36fp-4, 0x6.32a4339d39f90a0fa4p-60, -0xf.c351226350db8p-8, -0x1.24878f0be7f28p-4, -0x1.09d317f0270d7p-4, -0x1.289060bb8114fp-4, 0x2.9p-4 }, /* i=20 110.8 */
  { -0x2.b346e490467d4p-4, -0x1.30e178922672fd2c68p-60, -0x1.03b088bc812e1p+0, 0x2.119e8ed372a11f815ap-56, -0x1.6716cb603786fp-4, -0x5.6317b8423125af174p-64, -0x3.06ad7bcdb27cap-4, -0x5.16a305d39a37fef34p-64, -0x1.228bb38d02e07p-4, 0x3.a14c45243915b3026p-60, -0x1.ad087a5e24c33p-4, 0x2.450edc78a8178d70dep-60, -0x1.0eb0eb371206cp-4, -0x4.fdf30a636a9e21f00ap-60, -0x1.50508b232eab5p-4, -0x6.5c264af8217838722ep-60, -0x1.11879d6252324p-4, -0x1.3a52b1701817cp-4, -0x1.248828a6eef18p-4, -0x1.4586b822402b8p-4, 0x2.bp-4 }, /* i=21 110.6 */
  { -0x2.d3c2aa4acea2ep-4, -0xc.315ad6689e72168f98p-60, -0x1.040c95befb7d1p+0, 0x3.b1a461ad7a28d28abap-56, -0x1.795aa073c56c8p-4, -0x9.33d886ca0fdd40dbap-64, -0x3.10058ed3113fap-4, -0x3.d1aba8e7be20147b64p-60, -0x1.338eee642e1f4p-4, 0x4.5714dad9d3e98eaa44p-60, -0x1.ba29084f92f49p-4, -0x5.5370ea96251be21f8p-64, -0x1.2190468acf9bp-4, -0x5.5127ded530c6bd86a6p-60, -0x1.621e3727e7c78p-4, -0x2.a2f5d1dc888a5e5bfcp-60, -0x1.2875d1a8b50d1p-4, -0x1.524dc88cab4e2p-4, -0x1.41df6471ee683p-4, -0x1.65eadcde5c842p-4, 0x2.dp-4 }, /* i=22 110.5 */
  { -0x2.f44a3b00ccd56p-4, 0x6.702830319a344089e4p-60, -0x1.046d3adb11f07p+0, 0x3.dc6ff72fe1cb76278p-60, -0x1.8bd8227304d5ep-4, 0x2.6dd1e7608ace4a54e8p-60, -0x3.19e7d4b884948p-4, 0xd.5afa3bae7a3c1de7f4p-60, -0x1.4519eefef5627p-4, 0x3.e458efeec61765868cp-60, -0x1.c8320ee6bf6efp-4, -0x5.d64f0cc41088973abap-60, -0x1.357329f94ff5fp-4, -0x4.d738dc6af07c93d69cp-60, -0x1.7568b45ec53aap-4, 0x2.ac4be3f96228aee4e8p-60, -0x1.4128fa5eaf9aep-4, -0x1.6cb08ed6dcd48p-4, -0x1.6228bf162570bp-4, -0x1.8a2839d638ff6p-4, 0x2.fp-4 }, /* i=23 110.3 */
  { -0x3.14de2aa082788p-4, -0x9.9347563cf5335bf33cp-60, -0x1.04d286e4b9beep+0, 0x6.5ff7c4d67d8fe41c8p-60, -0x1.9e929b935d8cbp-4, 0x5.4f4bed6482d1c36ed8p-60, -0x3.2458b0a485f12p-4, -0x6.54509f2931def72818p-60, -0x1.573608902158fp-4, 0x1.9b9b9cc35fe91832fcp-60, -0x1.d73038a58a9b9p-4, 0x6.ecafec3c958a32ca6p-60, -0x1.4a6f3595f97d7p-4, -0x1.614178c211dd9d09dap-60, -0x1.8a4db7be4a7a4p-4, 0x5.6abb5e2f9ce0d3bd7cp-60, -0x1.5bce88783dcbcp-4, -0x1.89b95e198a6efp-4, -0x1.85be07b51b4b3p-4, -0x1.b2b920234581dp-4, 0x3.1p-4 }, /* i=24 110.2 */
  { -0x3.357f0efffc1dp-4, 0xd.ad54442f02da6edb6p-60, -0x1.053c8985d2bc9p+0, -0x1.8e1719e46120dc2566p-56, -0x1.b18d7140f4d4p-4, -0x6.2d362cc209fbea3cf8p-60, -0x3.2f5cd25f8fa7ap-4, -0x5.01653de3685c3b8d88p-60, -0x1.69ed12265aa66p-4, 0x5.137c99e920d196265cp-60, -0x1.e7313d9f9a867p-4, 0x1.7fa6787634d28cb1cp-64, -0x1.609bbde37b139p-4, 0x6.82e7720d713b345f08p-60, -0x1.a0edf1b99c119p-4, -0x2.087ba1015e96dd9e4p-60, -0x1.789890e99d2eap-4, -0x1.a9ae01fb55853p-4, -0x1.ad04480ebc302p-4, -0x1.e028f73f1b549p-4, 0x3.3p-4 }, /* i=25 110.0 */
  { -0x3.562d7ff83e1bap-4, 0x8.b23961f0e00283c984p-60, -0x1.05ab53452f914p+0, 0x6.da82e0193d6eae4efcp-56, -0x1.c4cc25f72be96p-4, 0x5.dbd1c3c38119a7eebap-60, -0x3.3af93a9e3219p-4, 0xe.f01d760fffc06bbb88p-60, -0x1.7d49719d7d85cp-4, 0x7.fd080542b5ea5c2b3ap-60, -0x1.f843f8f272342p-4, 0x7.2725662d988253ef1p-60, -0x1.7811f7b07f32cp-4, -0x2.9dc3fbf176ccab11f8p-60, -0x1.b96d5cdc146f9p-4, 0x1.c0d69123a6057eab9p-60, -0x1.97be5ae8fcbf8p-4, -0x1.ccdca7ef41792p-4, -0x1.d86d57ad21b5fp-4, -0x2.1316c7608e0f8p-4, 0x3.5p-4 }, /* i=26 109.8 */
  { -0x3.76ea178159bdep-4, 0x4.9138ee4b1d0db1583cp-60, -0x1.061ef58e175b7p+0, -0x7.4d9691355bf43c913ap-56, -0x1.d8525b33e883bp-4, -0x1.2dfb1df665b1796fa8p-60, -0x3.47333fa62a632p-4, 0xf.8abeaaa0d6895c0d7cp-60, -0x1.915627713e873p-4, -0x1.3657b3a8bb5a07ad22p-60, -0x2.0a788066acfp-4, 0xf.9549cd6ffaf04921dp-60, -0x1.90ed2881a903ap-4, -0x4.2ade80042745f060c8p-60, -0x1.d3f395d608c31p-4, -0x6.d43866e9b9fea9f814p-60, -0x1.b97d003824c88p-4, -0x1.f39cf0efaf657p-4, -0x2.0879ab5fc629ap-4, -0x2.4c382d81876a4p-4, 0x3.7p-4 }, /* i=27 109.7 */
  { -0x3.97b571cf7a8ccp-4, -0xb.dc6cf43cf0f67cc048p-60, -0x1.069782b847ba6p+0, 0x7.7811e9e7adfc10abcap-56, -0x1.ec23d387df2b4p-4, -0x1.aebaa64c1536545cd2p-60, -0x3.54109255de6d8p-4, 0x6.ed5d5e4683a839e18p-60, -0x1.a61edb87d792ap-4, -0x4.9c0917f45d483e4ea8p-60, -0x2.1de03e7436cfap-4, 0x5.da6682f40fc716203p-60, -0x1.ab4adc154b6cp-4, 0xb.a6b8c5ed7613b6fd6p-64, -0x1.f0ac3e2f2a27dp-4, -0x3.4b1755eb1f8ef63f28p-60, -0x1.de182202eef3ep-4, -0x2.1e512a199359ep-4, -0x2.3dba6adeadc92p-4, -0x2.8c5ccb63213cap-4, 0x3.9p-4 }, /* i=28 109.5 */
  { -0x3.b8902d70fc4bcp-4, -0xf.c95f78e45444028cf4p-60, -0x1.07150e107ec92p+0, -0x6.2f7adccb6183b5ep-68, -0x2.004474c645cep-4, -0x1.2ecbc9cead273601dcp-60, -0x3.619743966155ap-4, -0xa.d1321efd05edf21884p-60, -0x1.bbafeb0dd91c5p-4, 0x5.47dd35303e8f864a62p-60, -0x2.328e0eea5ab92p-4, 0x6.840939050bbcaee1bcp-60, -0x1.c74b1f9f53817p-4, 0x1.03c112c3b1db775332p-60, -0x2.0fc76afebfa5p-4, 0x9.ad2bee3dcc7aff22f8p-60, -0x2.05dab5423ec4p-4, -0x2.4d67b21350e38p-4, -0x2.78d3d825cc89cp-4, -0x2.d47249519f738p-4, 0x3.bp-4 }, /* i=29 109.3 */
  { -0x3.d97aeb6d9b1ep-4, -0xd.7d17d85154db0b236p-60, -0x1.0797abe195398p+0, -0x4.d76f40edee54d158fcp-56, -0x2.14b84a568837ap-4, 0xa.deb2e2360b9aa7eec8p-60, -0x3.6fcdca40f6a5p-4, 0x5.ec556d3f78065cfa8cp-60, -0x1.d216777eea9ecp-4, 0x2.d93ffb2e307de20a8cp-60, -0x2.48965e8464d2ap-4, -0xb.c15c1c05fcbaf1982cp-60, -0x1.e510c3707c1b8p-4, -0x7.456d295d5ea2087ffep-60, -0x2.317a213c721ccp-4, 0x6.18082801d94df180a8p-60, -0x2.3117ea064eae4p-4, -0x2.815c9225baa02p-4, -0x2.ba8016235f49ep-4, -0x3.25890384d8b9cp-4, 0x3.dp-4 }, /* i=30 109.2 */
  { -0x3.fa764f66d066p-4, -0x9.8ecb00191242f74af4p-60, -0x1.081f717e3165ep+0, -0x2.5ac18e65bde23e013ep-56, -0x2.298387aad08bep-4, 0x1.de6c5d9cef5ee883dcp-60, -0x3.7ebb0981f4e28p-4, 0xd.ee8d56acb6c5a425dcp-60, -0x1.e96076fa48a37p-4, -0x6.42508937471f6098b2p-60, -0x2.600f4dbc8791ap-4, -0xa.dfc1e746dc170eb274p-60, -0x2.04c1a3d374744p-4, 0x9.ec97b77fd46723a24p-64, -0x2.55fee176a75f4p-4, -0x3.abb47e8629cc5a9a04p-60, -0x2.602c319176876p-4, -0x2.babb53154de38p-4, -0x3.03925ec1906e8p-4, -0x3.80d981e823b4p-4, 0x3.fp-4 }, /* i=31 109.0 */
  { -0x4.1b82ffb97015cp-4, -0xe.dadddd6e6658663c9p-60, -0x1.08ac754b1cf11p+0, 0x4.9d7d625213f2e205e6p-56, -0x2.3eaa8ade74246p-4, -0xc.3e8db0e4005fe743d8p-60, -0x3.8e6657c3ebcbep-4, -0xe.834dfe9fcb8027a8fcp-60, -0x2.019cc60530faap-4, -0x8.415bbab249e7663ccp-64, -0x2.7910d738cff18p-4, -0xa.1e0b8236fc3a3a157p-60, -0x2.2686fa05a59c8p-4, -0x4.1c318dea901094fedp-60, -0x2.7d9644fec27b4p-4, 0x7.1116f548ca6259b82p-60, -0x2.937e67c979056p-4, -0x2.fa2117266fc22p-4, -0x3.54fabb3851ca6p-4, -0x3.e7cadf03c2002p-4, 0x4.1p-4 }, /* i=32 108.8 */
  { -0x4.3ca1a5a09b654p-4, 0x1.6b73b247c58dd21408p-60, -0x1.093ececa4756p+0, 0x5.7324aeffa310273ddep-56, -0x2.5431df7f99ddp-4, -0x9.82afce910b0870511p-60, -0x3.9ed7862f12f2cp-4, 0x8.490c08239577879cap-60, -0x2.1adb3ae13beaep-4, -0xa.60e5cc0babd81f5abcp-60, -0x2.93b4fa396efcap-4, -0xb.f7dd93ba3f12dc301cp-60, -0x2.4a8db6503b9f8p-4, 0x1.0f00a444a01e35249cp-60, -0x2.a887aee996584p-4, 0x2.f0537bd7860435c334p-60, -0x2.cb81253cfc14ap-4, -0x3.403f045366b5ep-4, -0x3.afca5508b9da2p-4, -0x4.5bfa49e9b3f08p-4, 0x4.3p-4 }, /* i=33 108.6 */
  { -0x4.5dd2ed5a1f3e4p-4, 0x1.a2fb3e84f654a73995p-56, -0x1.09d696a680a83p+0, 0x2.47c0929f0472e3c1ep-56, -0x2.6a1e4187baa92p-4, 0xa.e0e7f5e23873ca784p-60, -0x3.b016e8ca46396p-4, -0x3.384ca8faf9401f12dcp-60, -0x2.352cbaa0e9a52p-4, 0x3.9d37c9105b583df7acp-60, -0x2.b017e97ca701ep-4, 0xa.34ebe7a665a7a0b2cp-64, -0x2.7106e469a87fp-4, 0x4.a35d6e7decb4363dp-60, -0x2.d722139cbd4p-4, -0x5.4fec35f6f6a2ae3f4p-64, -0x3.08b43fd1f14e2p-4, -0x3.8ddd0ba261114p-4, -0x4.15387a248137cp-4, -0x4.df43d7ad25f5p-4, 0x4.5p-4 }, /* i=34 108.5 */
  { -0x4.7f17864c5624cp-4, 0xa.588412338764d188p-68, -0x1.0a73e6bff8b8p+0, 0x3.73c3cc4f16550cb484p-56, -0x2.8074a086f1088p-4, -0xc.6f32d68571adb308cp-64, -0x3.c22d5f4c3ad6ap-4, 0x7.1a154b9017c680211cp-60, -0x2.50a350297ed8ep-4, 0x3.40fffb9ed531128348p-60, -0x2.ce583f1b7d038p-4, 0x9.16bee573a017148034p-60, -0x2.9a281b70bd686p-4, 0xa.93b887fbe3cb66f344p-60, -0x3.09bcda0a8226p-4, -0x2.50a6e72de63fdebd7cp-60, -0x3.4ba6812e3417ap-4, -0x3.e3dd1ccbf3774p-4, -0x4.86a873b7747f4p-4, -0x5.73cce361b686p-4, 0x4.7p-4 }, /* i=35 108.3 */
  { -0x4.a070232da6fap-4, -0x2.340dca0c21a412c9fp-60, -0x1.0b16da398fd6cp+0, -0x6.5297fa72c22cd62bap-60, -0x2.973a23066887ep-4, 0x5.15269d5ed7ff29f15p-60, -0x3.d5245ead25ae2p-4, 0x4.34b0d7e81e89913dccp-60, -0x2.6d524556bd1cep-4, -0x9.ea4c62cfa22823fca8p-60, -0x2.ee9735f33027ep-4, -0xc.f88672bf824ced282cp-60, -0x2.c62bfb018f0dcp-4, -0x1.b5b2819713a3ed375cp-60, -0x3.40b8da38234b4p-4, 0xd.caa15b6486c8ccabd8p-60, -0x3.94f79b147c6dep-4, -0x4.433ed8112596p-4, -0x5.05b054f975cd4p-4, -0x6.1c104851e653cp-4, 0x4.9p-4 }, /* i=36 108.1 */
  { -0x4.c1dd7a2dbbae4p-4, -0x1.0b08737cd3bb19be828p-56, -0x1.0bbf8d8707978p+0, -0x5.7ea6a165241f7047fep-56, -0x2.ae742a26a4e8ap-4, 0x4.154069c9b9d97a9cf8p-60, -0x3.e905fb8ac84fap-4, 0xf.6b559b50cd20a39d74p-60, -0x2.8b4e3e7b2a12cp-4, -0xe.60dd8715de8f9d38a8p-60, -0x3.10f8e942b983cp-4, -0x4.c8b36c7550bd5dfd94p-60, -0x2.f552b71aa94dep-4, -0xa.6e66975ea2eb72764cp-60, -0x3.7c817d3877d5cp-4, 0x9.6c4e2898c21005f8e8p-60, -0x3.e55a634d58bb2p-4, -0x4.ad23d35140274p-4, -0x5.9420ee94d4658p-4, -0x6.daecd187ed12p-4, 0x4.bp-4 }, /* i=37 107.9 */
  { -0x4.e36045208cda4p-4, -0x6.fd9112162b0f9265e8p-60, -0x1.0c6e1e7c2324dp+0, -0x5.410e2454c525f3b8cap-56, -0x2.c628557eca058p-4, 0x6.05694bf57525186e1cp-60, -0x3.fddcf572cabf8p-4, -0xc.af1e2211e4fc5d7474p-60, -0x2.aaad587e8c45cp-4, 0x2.269e36b3f502fe846p-60, -0x3.35a49b3994068p-4, 0xc.5cd7f882f95efd295p-60, -0x3.27e2b4c7cee08p-4, -0x7.cc1c62f46174401fa4p-60, -0x3.bd8e036b04dbcp-4, 0x7.4368b0d9779e2624f8p-60, -0x4.3d97625041fd4p-4, -0x5.22d48b3db64p-4, -0x6.340f1bc91b7dp-4, -0x7.b3b64c8e5a86p-4, 0x4.dp-4 }, /* i=38 107.7 */
  { -0x5.04f941ab613b8p-4, -0x1.71db7c5df95849dda4p-56, -0x1.0d22ac5cc81dep+0, -0x2.3aa9d121ca63604f54p-56, -0x2.de5c87428b502p-4, 0xe.8505b354394a92ea18p-60, -0x4.13b4c329743d8p-4, -0xb.c2f3d1c0dfe84df798p-60, -0x2.cb8749e424328p-4, -0xd.32a02d64bc99a758f4p-60, -0x3.5cc5034e33d68p-4, 0x3.d69498399ff39b02a8p-60, -0x3.5e2939d00d50cp-4, -0x5.df01d900b72ef3b0a8p-60, -0x4.0462f6919d25cp-4, -0x1.a6e26789701839dea88p-56, -0x4.9e8fc1c83075cp-4, -0x5.a5c61e161a24p-4, -0x6.e7dea8d7c3e28p-4, -0x8.aa49c43bc20f8p-4, 0x4.fp-4 }, /* i=39 107.5 */
  { -0x5.26a93173e2498p-4, -0x1.c1d2f6ce8524972b8fp-56, -0x1.0ddd57ee32684p+0, 0x5.60c4175a3bbca84a36p-56, -0x2.f716e8b4f55c2p-4, -0x2.b8f5fdc59ec82bc12cp-60, -0x4.2a99a00544604p-4, 0x1.857d286f49098ea79e8p-56, -0x2.edf5870b14bdap-4, -0x3.783022d722664bf724p-60, -0x3.8688a54f659cep-4, -0x6.9acd36e498acf809bcp-60, -0x3.987b31f8f79fcp-4, -0x5.b2fa1b5b86c057f528p-60, -0x4.5193ce31e24fp-4, 0xf.94babdf63251b30788p-60, -0x5.0940aa500605p-4, -0x6.37a0f2c69d964p-4, -0x7.b24f1edcadep-4, -0x9.c32584684d0b8p-4, 0x5.1p-4 }, /* i=40 107.4 */
  { -0x5.4870da51697c8p-4, 0xa.f3f00c5933244cd12p-60, -0x1.0e9e43894f02dp+0, 0x1.fe18b5e7a90293b0d6p-56, -0x3.105deeeedefep-4, -0x8.64d0e1aaa6b57ae928p-60, -0x4.42989a7aa9258p-4, -0x1.c89e9060121412dc588p-56, -0x3.12136a059013ep-4, 0xf.1d4109a53db39c4adp-60, -0x3.b3223244de9aap-4, -0xe.0534f4060134750b4p-64, -0x3.d7360cd25c95ep-4, -0xa.b0e65b0c17438d2e78p-60, -0x4.a5c4cdc2890acp-4, 0x4.d9e757ba9613229c5p-60, -0x5.7ec7226f6c508p-4, -0x6.da4875c98504p-4, -0x8.968ad074a0a28p-4, -0xb.0385a9ced20bp-4, 0x5.3p-4 }, /* i=41 107.2 */
  { -0x5.6a510680ac7f4p-4, 0x1.35e8dbcb0816dda8298p-56, -0x1.0f65932e53afap+0, -0x9.0802b635672891fbap-60, -0x3.2a3860007bf4ep-4, -0x5.15265b489ff58876f4p-60, -0x4.5bbfa3f62053p-4, 0x1.4ece06c9f4263144e38p-56, -0x3.37fe5e6ee258ep-4, 0xc.b5b4a24a8b914a7aacp-60, -0x3.e2c8f45945354p-4, 0x3.d437351c678e2cae38p-60, -0x4.1ac0b76f21618p-4, -0x1.9ef0c5ca7010cb0246p-56, -0x5.01ad255d7975p-4, -0x1.6682f3d265ed42cea38p-56, -0x6.0064840deb804p-4, -0x7.8fe41d0280c98p-4, -0x9.9838951c8f3b8p-4, -0xc.71862b9971a28p-4, 0x5.5p-4 }, /* i=42 107.0 */
  { -0x5.8c4a84d9f15fp-4, 0x1.e8ea4df76e5db565p-60, -0x1.10336c99ab3f9p+0, -0x5.7a7f42452c8142d712p-56, -0x3.44ad58764b17p-4, -0x1.9b92b9eb996185b54p-60, -0x4.761da2268694cp-4, -0xa.bb3c172a1e749df9c8p-60, -0x3.5fd611b58fd5p-4, 0x7.facdf930857397df0cp-60, -0x4.15b94733b4fbcp-4, 0xb.30bd717825f8349478p-60, -0x4.638cb5f3d70e8p-4, 0x1.4cd2106ddd95676dd8p-60, -0x5.66195f209f848p-4, -0x1.740d074d19a658bf91p-56, -0x6.8f83a15a279a8p-4, -0x8.5ae9ed3aed6a8p-4, -0xa.bb90b7c5675p-4, -0xe.144b8020c43ep-4, 0x5.7p-4 }, /* i=43 106.8 */
  { -0x5.ae5e2909f6e54p-4, 0xd.8ee857e2bcf3b2675p-60, -0x1.1107f75a50748p+0, 0x4.c3dbfa6999c755e966p-56, -0x3.5fc4514a6ff64p-4, 0x7.e271422cf24cfbaafcp-60, -0x4.91c281dd3afcp-4, 0x1.0a443e6aeb49f630adp-56, -0x3.89bca85dae836p-4, -0x9.245287cd529efd5464p-60, -0x4.4c351e592b874p-4, 0x1.ad06c394272f3dd72ap-56, -0x4.b217617d7b168p-4, 0x1.4124567e5e8e4591fb8p-56, -0x5.d3ee251a258bp-4, -0xe.d50b7ca2cc58bcd51p-60, -0x7.2dbeb6a634bdcp-4, -0x9.3e2ac749498bp-4, -0xc.0575b8a3c2d5p-4, -0xf.f4333e9eb369p-4, 0x5.9p-4 }, /* i=44 106.6 */
  { -0x5.d08ccbcdc0adp-4, -0xc.3f474f7bd97c2dd928p-60, -0x1.11e35ce9b3cedp+0, 0x8.e374d3b0ae4b072b2p-60, -0x3.7b85264c76c1ap-4, 0x5.7439e8004d4f7fb608p-60, -0x4.aebf4bae216a8p-4, -0x1.6ade5cf49a71524d2f8p-56, -0x3.b5d6f8d0b339ep-4, 0x5.a961a2590568f798c8p-60, -0x4.86849b688328cp-4, -0x1.c8a0e9272c2111d1a48p-56, -0x5.06eb4f968e428p-4, 0x1.44c144de47dd20514e8p-56, -0x6.4c2b6d9ebd288p-4, 0x1.7c14fa60dd0b80c1458p-56, -0x7.dce64c1eaa5b8p-4, -0xa.3ce0d0b38a358p-4, -0xd.7b91a3f455048p-4, -0x1.21b0e6f1060c5p+0, 0x5.bp-4 }, /* i=45 106.4 */
  { -0x5.f2d74b317a3b4p-4, 0x4.6d49b267f40f1e9a58p-60, -0x1.12c5c8c54b3d2p+0, 0x3.1eb5c819784a14692p-56, -0x3.97f81d0a8bfd2p-4, -0x7.d58da1523c69dfd26cp-60, -0x4.cd263a7e8496p-4, -0xb.0ca9207e2f8ce63e28p-60, -0x3.e44ccc636d8f6p-4, 0x5.5590632799fe7b691p-60, -0x4.c4f6b646f72a4p-4, -0x1.9ccd99fbbd8b7dabe2p-56, -0x5.62a1e94304dep-4, 0x1.a8df74ab4852f127cp-56, -0x6.cff01e491510cp-4, 0x4.0837df25046c4ce838p-60, -0x8.9f0930b815d78p-4, -0xb.5ac05c4c79bd8p-4, -0xf.2478e8be5beep-4, -0x1.494679c1b0585p+0, 0x5.dp-4 }, /* i=46 106.2 */
  { -0x6.153e8ad2a868cp-4, 0x6.8fbd13590e196fbd6p-60, -0x1.13af6889ed89ep+0, 0x6.bb52720a3f089b8f5p-56, -0x3.b525ec485c2a8p-4, 0xe.b8962707447960bcacp-60, -0x4.ed0ad43775714p-4, -0x1.3a07bc876f6e62fe8f8p-56, -0x4.1549275145fccp-4, -0xc.6312689175b8fd1bdp-60, -0x5.07e1f9a133d38p-4, 0x1.b43eaaac308d989ad8p-56, -0x5.c5e538a23356p-4, -0x1.1cddb1d4d55dedc28b8p-56, -0x7.607e38b37a024p-4, 0xf.4b42cf83d04f0faf7p-60, -0x9.767dbf6ff25bp-4, -0xc.9c0bcb92192e8p-4, -0x1.107d3cd657638p+0, -0x1.76dd735689583p+0, 0x5.fp-4 }, /* i=47 106.0 */
  { -0x6.37c37425e4e1p-4, -0x8.53ee398078eac375f8p-60, -0x1.14a06c111e9abp+0, 0x1.576036fe881f2758ap-60, -0x3.d317c4111ddfep-4, -0x7.9ee9fca5c9c33c9adp-60, -0x5.0e8204d6a29dp-4, -0x3.63895582e73339cb38p-60, -0x4.48fa9885734dp-4, 0x1.7480b66193a4dcb5a9p-56, -0x5.4fa5567f8571cp-4, 0x1.cbeab9ff4470960633p-56, -0x6.3171f5479150cp-4, 0x1.eb8eef8ae36724d7cc8p-56, -0x7.ff3fa756c991p-4, 0x1.4a380399c690a8f4dep-56, -0xa.65eca95307fc8p-4, -0xe.05aafa41a849p-4, -0x1.32e8fde6df549p+0, -0x1.ab76969d8a808p+0, 0x6.1p-4 }, /* i=48 105.8 */
  { -0x6.5a66f6c0644e4p-4, 0x1.b53a9536565c3cfe55p-56, -0x1.159905907522cp+0, -0x1.2ab039a4dd5bef6922p-56, -0x3.f1d75673bfe0ap-4, 0xd.9feedc27251a377f98p-60, -0x5.31a23c1fdb2fp-4, -0xe.cb8ddf0402274f97p-64, -0x4.7f9392284f734p-4, -0x2.8043021e6e18e8481p-60, -0x5.9ca9121509eb4p-4, -0xf.8e5b920f3d973c3e08p-60, -0x6.a619d8c02b9fp-4, 0x1.2d7c958df8249bddbap-56, -0x8.adcbc4f82b35p-4, -0x3.6b9d43ecf246fe573p-56, -0xb.705d89fb56818p-4, -0xf.9d46e3b5653bp-4, -0x1.5a31b012142dap+0, -0x1.e84196012190fp+0, 0x6.3p-4 }, /* i=49 105.6 */
  { -0x6.7d2a08a58d1ecp-4, 0x1.71ef7327550cd438838p-56, -0x1.169969bb465b6p+0, -0x6.2868ede5b9c003a64ep-56, -0x4.116ee0f9da134p-4, -0x4.261e15125a91db9338p-60, -0x5.56838e39a5c6cp-4, -0xe.a453c41af73cf0d92p-60, -0x4.b94acc08d8834p-4, 0x3.15f21a5ac38b4909f8p-60, -0x5.ef5fd16c78ba4p-4, -0x3.26243b89f9eb5bf278p-60, -0x7.24c6464cc360cp-4, 0xe.5c650dbe668ee2ff1p-60, -0x9.6dedbdd3797fp-4, 0x2.02a1dad8accc2d9eafp-56, -0xc.994599d08fe3p-4, -0x1.1696a567a37ccp+0, -0x1.871aa3378a081p+0, -0x2.2ea66a3b041b8p+0, 0x6.5p-4 }, /* i=50 105.3 */
  { -0x6.a00da698eaaap-4, 0x4.de9e53952b1db5546p-60, -0x1.17a1cfe6c8b8dp+0, -0x1.a5eccfa2a616d84b2p-56, -0x4.31e936ebda1ccp-4, -0x5.10bce18cd1d4e48cb8p-60, -0x5.7d3fd788addb4p-4, -0x9.993a8a508577eab8p-60, -0x4.f65bb1217dba4p-4, -0xa.bdfeb076b3be78dcb8p-60, -0x6.4847c72448b7cp-4, 0x3.0cbe45f1ed98df0abp-60, -0x7.ae7b52b3d5c24p-4, 0x8.6d5d81e1c242c65198p-60, -0xa.41abef647e8d8p-4, -0x3.0d3feb175ea5951829p-56, -0xd.e498e1cec387p-4, -0x1.371a8b7be0a7cp+0, -0x1.ba88e799161fp+0, -0x2.8050aca9f9794p+0, 0x6.7p-4 }, /* i=51 105.1 */
  { -0x6.c312d474cee18p-4, -0x1.beb0350ac9f2184874p-56, -0x1.18b27230e36f8p+0, 0x3.2bb1d164c3df158772p-56, -0x4.5351cc76fa33p-4, 0x5.41dee430f0ff3148p-68, -0x5.a5f2e4287257p-4, -0x1.c076f617343e58b19a8p-56, -0x5.3706d9a479b9p-4, 0x4.d6f8e3b1230355f488p-60, -0x6.a7ec081372f28p-4, -0x1.9af47f4bb1b6bd88bf8p-56, -0x8.445b3b4191c38p-4, -0x2.bae3e05f1482056b0bp-56, -0xb.2b5072882d748p-4, 0x3.5f2bbf548f49ccb2c9p-56, -0xf.56de68970264p-4, -0x1.5becc0fcabeecp+0, -0x1.f589b0cede34dp+0, -0x2.df3d6d22f185ap+0, 0x6.9p-4 }, /* i=52 104.9 */
  { -0x6.e63a9d85fbce8p-4, -0xf.28344ce2b74e3c76bp-60, -0x1.19cb8da9e60cbp+0, -0x6.58873022d31ebfc1e8p-56, -0x4.75b4c2cbeafa4p-4, -0x1.7b46723fb3e861e3e68p-56, -0x5.d0ba9b6bce64cp-4, 0x1.8f9bef9d13bb6dd332p-56, -0x5.7b929330329a4p-4, -0x9.8716e98891aab3e85p-60, -0x7.0ee60c744e6ap-4, -0x1.27fee63fc381cc508d8p-56, -0x8.e7aa5da7a7b7p-4, -0x1.a03a5f9d2c306857ffp-56, -0xc.2d7303e798da8p-4, -0x1.60a9cdb5865d7202c2p-56, -0x1.0f547e8e66b4ap+0, -0x1.85b0bdd4565bap+0, -0x2.395a09a9134eap+0, -0x3.4dcc193d4e186p+0, 0x6.bp-4 }, /* i=53 104.7 */
  { -0x7.098614ecb5e14p-4, 0x8.a709737c8d31240f5p-60, -0x1.1aed628169723p+0, 0x1.70ca5a4bc35244accap-56, -0x4.991ef54fc1b14p-4, 0x7.94c97bc7258dae0d3p-60, -0x5.fdb72fdde53p-4, 0x1.e4a37cdadfb5ae1c8dp-56, -0x5.c44b7908a0818p-4, -0x1.7a0ff5e5d31279f3b98p-56, -0x7.7ddf64196b524p-4, 0x1.20ef92ad67d6c953698p-56, -0x9.99d3c589412cp-4, -0x3.341b990b368d1e64b6p-56, -0xd.4b048885efcep-4, 0x1.5eca46891bf9ba64b6p-56, -0x1.2c5cdbf4623e8p+0, -0x1.b524e819b4463p+0, -0x2.8770244685c46p+0, -0x3.ced331311974p+0, 0x6.dp-4 }, /* i=54 104.5 */
  { -0x7.2cf65603a8aep-4, -0x1.d9730350eb04931fcfp-56, -0x1.1c183436a246fp+0, 0xb.85360fe51c0909004p-60, -0x4.bd9e07fbbdf8p-4, -0xd.82e8346b83375743b8p-60, -0x6.2d0b544bf3914p-4, -0x1.df628c9dfc437adaaep-56, -0x6.11851e7908d88p-4, 0xe.05d0d983e70c2c5d1p-60, -0x7.f593a53f72a68p-4, -0x1.fa48707713e2b88ed98p-56, -0xa.5c6e683266ea8p-4, -0x1.99b32d390d69bf3392p-56, -0xe.875c7840eeaa8p-4, 0x5.39fa8ae4931eba1d2p-60, -0x1.4cf4fe1210b81p+0, -0x1.eb27782029e0ep+0, -0x2.e186a3d0c99d2p+0, -0x4.65b9b6d430f1p+0, 0x6.fp-4 }, /* i=55 104.2 */
  { -0x7.508c84cd112b4p-4, 0x1.c74e53df13c5e4059fp-56, -0x1.1d4c49cc747fdp+0, -0x9.942b1b1ecc27ab3c4p-60, -0x4.e340770be35a8p-4, 0x2.e99063efc8b8684278p-60, -0x6.5edc7672e3f7p-4, -0x1.b33547bf36009734f38p-56, -0x6.639acdd32cfp-4, -0x6.e7dc420c8d0390a298p-60, -0x8.76d29ecec0458p-4, 0x1.a9c28f91da5b9f3ep-56, -0xb.31432b65923c8p-4, 0x3.60807d2760da05d572p-56, -0xf.e648868e8c8c8p-4, 0x2.f5de4259e217fb536dp-56, -0x1.719bcdc04db07p+0, -0x2.28bc55af74b54p+0, -0x3.49aa542f6228cp+0, -0x5.1696811eca154p+0, 0x7.1p-4 }, /* i=56 104.0 */
  { -0x7.7449ce66aaf9cp-4, -0xe.835d60abc43f4d49cp-60, -0x1.1e89ee01a019cp+0, -0x2.ad7149335d0a1fe688p-56, -0x5.0a15a82041c18p-4, -0xb.1a65db2e7abbf47a28p-60, -0x6.935300009d9dp-4, 0xb.5a107375bed94ed938p-60, -0x6.baf05ee071f6p-4, 0x9.4001a7c1df05fecd2p-60, -0x9.0282d8593b928p-4, 0x1.6f456e41470c04ea09p-56, -0xc.1a53d9754a838p-4, -0x1.cdc30a9ede741b17a1p-56, -0x1.16c1ef356afdap+0, 0x7.0f9137a13255a64962p-56, -0x1.9ae4012b0f4fap+0, -0x2.6f14246d06f46p+0, -0x3.c24ae0eba3b18p+0, -0x5.e656edd602c4cp+0, 0x7.3p-4 }, /* i=57 103.8 */
  { -0x7.982f6984e9e3cp-4, -0xe.153898d98a350f5bp-64, -0x1.1fd16f8d68a0ep+0, -0x7.400f5083a7eb66c82ap-56, -0x5.322dfcf92f124p-4, -0x1.9c448e0268e10d16a68p-56, -0x6.ca9a9eb15e9c8p-4, -0x1.a54ad8c37e4b1e7cf4p-56, -0x7.17f3280903424p-4, -0x1.c59d4873fd287141e08p-56, -0x9.99a46be06fe98p-4, 0x2.b1f952322f1770b0ccp-56, -0xd.19e32b3ab069p-4, -0x3.431be98b1f2b551dfdp-56, -0x1.31dd4032c36bfp+0, 0x7.7e0ef36adbec1b116p-56, -0x1.c9778b142bf9bp+0, -0x2.bf94c238270a8p+0, -0x4.4e4f41232d628p+0, -0x6.daeed0c241848p+0, 0x7.5p-4 }, /* i=58 103.5 */
  { -0x7.bc3e96f615894p-4, -0x1.7b9360c3b82f34f4ef8p-56, -0x1.2123216123cd1p+0, 0x7.a28968ed6428a2990ep-56, -0x5.5b9ae7fbb865cp-4, 0x1.a1592351c7ba0f6bf28p-56, -0x7.04e2946d8cc1p-4, -0x1.7c68277d9d76bc245p-60, -0x7.7b1b0df77befp-4, 0x1.928b66e9d6381debde8p-56, -0xa.3d54478329628p-4, 0x2.7a93d95c425006dcb6p-56, -0xe.327e182028f2p-4, -0x1.e2d3c7d97a496f8a57p-56, -0x1.5011338438e55p+0, -0x2.30dc7f5b2a8b6cc104p-56, -0x1.fe1bb258a5094p+0, -0x3.1be38a2d455ecp+0, -0x4.f12ebcffa822p+0, -0x7.fb9403108ebbcp+0, 0x7.7p-4 }, /* i=59 103.3 */
  { -0x7.e078a22deb88p-4, 0x1.0803b3e82eddbc8b01p-56, -0x1.227f5aef2780cp+0, 0x4.16f507a6d4ecd6f6a4p-56, -0x5.866f02b14b018p-4, 0xb.2c4fb9cead1e6bb1ap-60, -0x7.425e106cf1d28p-4, 0xa.98b003e850f9d277fp-60, -0x7.e4ebb618da388p-4, 0x1.846e9d5cdd461044178p-56, -0xa.eecfe7aa1511p-4, 0x3.35dfb4d35b68d24dc1p-56, -0xf.6706a5b28ca9p-4, -0x3.c92520700083d74b43p-56, -0x1.71c5d0576da1ap+0, 0x3.02d3cfc1e3fbd320ap-60, -0x2.39b5f5077ee7ep+0, -0x3.85f1c316dc9ap+0, -0x5.af0fac9f3fb6cp+0, -0x9.51089f486fea8p+0, 0x7.9p-4 }, /* i=60 103.0 */
  { -0x8.04dee1da7c3b8p-4, 0x1.3064f6236756b8a9dbp-56, -0x1.23e678779d056p+0, 0x5.63783e8d83e13c1a4ep-56, -0x5.b2be267c215cp-4, -0xd.5aec252040d7b824f8p-60, -0x7.8344928a135d4p-4, 0x9.a8d69ccb7514ea703p-60, -0x8.55f5e10cb5b38p-4, 0x2.e25f882e83e8b2d73ap-56, -0xb.af799d3d205dp-4, -0x3.3baed139eb1285d95dp-56, -0x1.0bac07b569c76p+0, -0x3.b6ab4c0fa87ee9918p-60, -0x1.97729dbdb45cbp+0, 0x4.04c556d53dbe858c74p-56, -0x2.7d51e26e3dee2p+0, -0x4.000bb9682749p+0, -0x6.8ced5c79a4c7cp+0, -0xa.e5f7d2be77a48p+0, 0x7.bp-4 }, /* i=61 102.8 */
  { -0x8.2972b88307eap-4, 0x3.92a14219ff6137e4bp-56, -0x1.2558db5bceab9p+0, -0x3.ebc3d57b5eaf241456p-56, -0x5.e09d87c180cc8p-4, 0x5.e6ba8cd662253b88d8p-60, -0x7.c7d25a1c30238p-4, -0xb.9599fc71f13dcbed38p-60, -0x8.ced8f2ee29998p-4, -0x1.368aacdcf4b7a212f4p-56, -0xc.80dd76f16448p-4, -0x1.27ae29341b45dc6c97p-56, -0x1.2315f8c4daab1p+0, -0x6.449a99a24001933212p-56, -0x1.c1a13a3aacac7p+0, 0x4.bb596392381b38327ep-56, -0x2.ca2821967b29ap+0, -0x4.8ceb24f521e3cp+0, -0x7.90c6e51e9e1ccp+0, -0xc.c76a4cd68eca8p+0, 0x7.dp-4 }, /* i=62 102.6 */
  { -0x8.4e359531b5db8p-4, -0xd.ea1e9a3200cf0d924p-60, -0x1.26d6ea789648cp+0, -0x1.16f6b7d6efb71c02aep-56, -0x6.1023d3d371198p-4, 0xd.5d66587f875804e3fp-60, -0x8.1048e1e18136p-4, -0x2.0b38cc3907496ecd7cp-56, -0x9.5044b058fe218p-4, 0x2.d62ee4e9b9c8acf165p-56, -0xd.64b6e8f918b28p-4, -0x6.53c844f55d6b43487p-60, -0x1.3cf193ab14b55p+0, 0x5.ca89f73a03669acbf2p-56, -0x1.f0f06ecd5ff36p+0, -0x4.5ee92fe852d688ebb8p-56, -0x3.21a6e653f36c8p+0, -0x5.2fcda36496468p+0, -0x8.c1d9425108cbp+0, -0xf.0558cc82b0408p+0, 0x7.fp-4 }, /* i=63 102.3 */
  { 0x1.7af961ef39d23p+0, 0x1.7a0a884eb5ed49d27ep-56, -0x2.6d50b66b084e4p-4, -0xb.90201654085ef46fe4p-60, 0xb.3e1c32d40fabp-8, 0x1.0579cca1ffa5f743d5p-60, -0x4.5ee7d4a59065p-8, -0x8.812f079071ecc24bc8p-64, 0x1.f4b0a2c3879e3p-8, -0x7.b0af306ba75a443b4ep-64, -0xf.34930b8c1f0d8p-12, -0xd.8cb19026383c5bccfp-68, 0x7.cc7c604badf28p-12, -0x1.6fcfd5ece8a00dafd9p-64, -0x4.27dce47e41678p-12, 0x1.4caa3ad84917ab084ap-64, 0x2.473d20538eaaep-12, -0x1.46d8ef5a3db71p-12, 0xb.a65f48ebbe6d8p-16, -0x6.bd31ee5e96e7p-16, 0x8.1p-4 }, /* i=64 121.6 */
  { 0x1.7aabe4ae052a9p+0, 0x2.d9b9e43acfb16f4ep-56, -0x2.6a8472a78c1cep-4, 0x6.f2d0503fb0bc3370bp-60, 0xb.241168d4df21p-8, -0x2.f9cf91cf5205cee702p-60, -0x4.4f680592aaf64p-8, 0xa.bd10f61500464779a8p-64, 0x1.eb4cbd935ae08p-8, 0x1.e4456a1ff5ec6f994ap-64, -0xe.d85658cfc0edp-12, -0x3.ddb50ff0b16517928bp-64, 0x7.934a2607cd244p-12, -0x1.00e44c72c91527c7148p-64, -0x4.041e3a1bbd0d4p-12, -0xa.3f1ca68af9311ae9p-72, 0x2.30c2ca3d644bap-12, -0x1.38a4049141523p-12, 0xb.1619da740868p-16, -0x6.614defb93a4f8p-16, 0x8.3p-4 }, /* i=65 121.7 */
  { 0x1.7a5ec08d99337p+0, -0x8.08d97c04118cc8ecp-60, -0x2.67bea6095f8b6p-4, -0xc.fd2ece58f95128a3d8p-60, 0xb.0a62be1c7ed28p-8, 0x3.dc79537703aae54b91p-60, -0x4.40327148a50ecp-8, -0x1.020d24b7cb23d11f88p-64, 0x1.e221aa41fa701p-8, 0x2.6cc9d4ac5bc4e0d664p-64, -0xe.7ebc6d25f867p-12, 0x3.ed5dc82ecac6578bb8p-64, 0x7.5c02a8657e28p-12, -0x2.d7dfd22014087aae78p-68, -0x3.e1bf58b19bb2ap-12, 0x8.c8df31c03299a9be98p-68, 0x2.1b41f2d0c3b16p-12, -0x1.2b1e990fc00dfp-12, 0xa.8d7ec4e4ae07p-16, -0x6.0abe1256cf12cp-16, 0x8.5p-4 }, /* i=66 121.8 */
  { 0x1.7a11f4c07f71ap+0, 0x4.458d4425cf0f9a136p-60, -0x2.64ff39bfec2a6p-4, 0xd.12369a22b145f51a2p-60, 0xa.f10e7a90d50bp-8, 0x3.97e99ff224eff115c4p-60, -0x4.314557bc7e488p-8, -0x1.f6f30f9b7a3e395841p-60, 0x1.d92dca2b755dbp-8, 0x1.ea6af080340e8fb742p-64, -0xe.27aeb975dd95p-12, -0x2.f59f06dcd868fdd781p-64, 0x7.269315c81e518p-12, 0x1.fa9b6b1ac0389908568p-64, -0x3.c0b10947ba90ap-12, 0xc.5060d669958b980528p-68, 0x2.06ae97a5fceaep-12, -0x1.1e3f5faa715b2p-12, 0xa.0c1c848a2201p-16, -0x5.b92cd8af16958p-16, 0x8.7p-4 }, /* i=67 121.8 */
  { 0x1.79c5807c14aa5p+0, -0x5.d41ec3abaf6020fc0ap-56, -0x2.62461767553e4p-4, -0xc.b4beb0aa360d8f81f8p-60, 0xa.d812f0719a6cp-8, 0x2.0877f73e7cf09386d1p-60, -0x4.229f05a0aa27cp-8, -0x9.1f225d0fd29d4aebcp-64, 0x1.d06f8c7fbf422p-8, 0x4.56701e09bcaa23f534p-64, -0xd.d3178b90929bp-12, -0x1.f0231a7598c838d5d4p-64, 0x6.f2e96c6b9e24p-12, 0x1.84205ebdd18672b42fp-64, -0x3.a0e4cfeb3781ep-12, -0x2.766c996af34a7268ap-68, 0x1.f2fd58e07f381p-12, -0x1.11fd949cf46adp-12, 0x9.9188ad87842b8p-16, -0x5.6c4a863dc25b4p-16, 0x8.9p-4 }, /* i=68 121.9 */
  { 0x1.797962f87b74bp+0, -0x5.f1b3abc49ba293cbe4p-56, -0x2.5f932905eabecp-4, 0xf.da270b58f6e6872ee4p-60, 0xa.bf6e7c0d30dp-8, 0x3.99eaf80561839acc2cp-60, -0x4.143dd3f8913ap-8, 0x1.5346632151165938018p-60, 0x1.c7e56dbb9cc71p-8, -0x2.cd53fa51407a678bd2p-64, -0xd.80e204af622e8p-12, 0x1.74639a0af3599edae7p-64, 0x6.c0f4706ff1b6p-12, -0x1.21953f343f8aa9cccdp-64, -0x3.824ce1d1667c8p-12, 0x9.304b5c9d2f066b019p-68, 0x1.e0236fd39f661p-12, -0x1.0650f4f8310ap-12, 0x9.1d5f757900b9p-16, -0x5.23ccb6122895p-16, 0x8.bp-4 }, /* i=69 122.0 */
  { 0x1.792d9b708f1c5p+0, 0x7.368c5a300dbb09ee88p-56, -0x2.5ce65909aecd8p-4, 0x7.85070e8eda187b16c4p-60, 0xa.a71f8377f8abp-8, 0x7.5cdc8f8b1302438fp-64, -0x4.062027b0336f4p-8, -0x1.1c55d229efa5e0e3da8p-60, 0x1.bf8df7275cb1cp-8, 0x6.d5a861bda4f6a026a4p-64, -0xd.30fa10662875p-12, 0x2.765a119c5cfe7a1064p-64, 0x6.90a3a26a95db8p-12, 0x1.19a6d296ebd71d7888p-64, -0x3.64dc1c0bc4c24p-12, -0x8.71d81e519084923484p-68, 0x1.ce16a63c62266p-12, -0xf.b31b6a3a631e8p-16, 0x8.af433cca830ep-16, -0x4.df6df81631d94p-16, 0x8.dp-4 }, /* i=70 122.1 */
  { 0x1.78e22921d6d1bp+0, 0x7.8e6707b366f34a06fap-56, -0x2.5a3f9245ed0ccp-4, 0x6.b5075ddc8fcc8995e4p-60, 0xa.8f2476460ca88p-8, 0x1.5ccf72638d360c734dp-60, -0x3.f8447137be66ap-8, 0x8.f95dc2f12b17ec09p-64, 0x1.b767be5b16aebp-8, 0x7.9bd44b2d9672d37cfp-64, -0xc.e34c5c03ebcb8p-12, 0xa.f64332967c51d7f96p-68, 0x6.61e736767bee4p-12, 0x4.9a6fd78a7ed4c5f96p-68, -0x3.4885fac3ea45p-12, -0xa.f7479cc774ddfd43a4p-68, 0x1.bccd4e065276p-12, -0xf.09880e722796p-16, 0x8.46dc292438568p-16, -0x4.9eed774574e6cp-16, 0x8.fp-4 }, /* i=71 122.2 */
  { 0x1.78970b4c79267p+0, 0x3.265a87154419ef8974p-56, -0x2.579ebff0e33acp-4, -0xf.1d26bc2099be836d2cp-60, 0xa.777bcd474c58p-8, -0x2.90c91c3b5066763c8p-64, -0x3.eaa92c22ec8bap-8, 0xe.57bf669839ea0046cp-68, 0x1.af7164c82cd01p-8, -0x3.341c734ee706b78d18p-64, -0xc.97c64e5be459p-12, -0x3.83ac59015abb25ae11p-64, 0x6.34b00bbb27bfp-12, 0x1.885a7789a8097565d5p-64, -0x3.2d3e90f713438p-12, 0xe.b0f7f9cfe24e50a23cp-68, 0x1.ac3e399227916p-12, -0xe.67e657330371p-16, 0x7.e3d7c267e9574p-16, -0x4.620ea5fd69d1p-16, 0x9.1p-4 }, /* i=72 122.3 */
  { 0x1.784c41332fd3cp+0, 0x7.5fc971d200f945be8ap-56, -0x2.5503cda17a846p-4, 0x8.da9d6e582337274758p-60, 0xa.60240a459fa8p-8, -0x1.ac4fe19f084caae082p-60, -0x3.dd4cdecc03f4ep-8, 0xe.1380019445efd3ca38p-64, 0x1.a7a99747d031p-8, 0x2.4772395aa1097fea6cp-64, -0xc.4e55fff18512p-12, 0x3.474b75352a1abd40bbp-64, 0x6.08efa4643ce6cp-12, 0x5.6c9455b86155b04c3p-68, -0x3.12fa80a96ac24p-12, 0x8.d6b52520ff788e6f64p-68, 0x1.9c60b475a23f5p-12, -0xd.cdcd9df3ad39p-16, 0x7.85e89a1754f74p-16, -0x4.2898f105399d4p-16, 0x9.3p-4 }, /* i=73 122.3 */
  { 0x1.7801ca1b3bc8bp+0, -0x4.38ddc40444b8f850eep-56, -0x2.526ea74d11006p-4, -0x7.9016643506f6a118bp-60, 0xa.491bb7c55dedp-8, -0xe.c6a534566df9cf124p-64, -0x3.d02e19fa4dd8ap-8, 0xa.7d133899199c30a59cp-64, 0x1.a00f0dae4c7a6p-8, 0x3.a6ac83b282ee4165ecp-64, -0xc.06ea3382676a8p-12, -0x2.29ded2716725d44382p-64, 0x5.de981e0322bf4p-12, 0x1.60e3dc65d1da98d7cp-64, -0x2.f9aef389c303cp-12, -0xe.fa31eb07c67e5a2ad4p-68, 0x1.8d2c7caca7a99p-12, -0xd.3adb19520591p-16, 0x7.2cc5f39341a9cp-16, -0x3.f25777adee846p-16, 0x9.5p-4 }, /* i=74 122.4 */
  { 0x1.77b7a54c597ddp+0, -0x4.37614ae12d5a8124f4p-56, -0x2.4fdf394552d0cp-4, 0x9.1b9e33d907eabef02p-60, 0xa.326168c7c40f8p-8, -0x1.e04b36084aea51bp-60, -0x3.c34b788be14aap-8, -0x2.e26c5b3ec96325e9ccp-64, 0x1.98a08a62e120ep-8, -0x4.b8b5332fc43c419ca8p-64, -0xb.c1724ee944d38p-12, -0x3.ee65c53a209a5f4fc3p-64, 0x5.b59c2a54c9744p-12, -0x1.15a783cbd91efd8f108p-64, -0x2.e15193fee44a2p-12, -0x5.d6f7abbbaba01948cp-68, 0x1.7e99bc342c7bep-12, -0xc.aeb1822814ba8p-16, 0x6.d82b767eac1ap-16, -0x3.bf18c99f41a16p-16, 0x9.7p-4 }, /* i=75 122.5 */
  { 0x1.776dd210b58d7p+0, -0x7.1bfc77b3b477370c14p-56, -0x2.4d55703622646p-4, 0x1.d9c8db80b76bd2740cp-60, 0xa.1bf3b88f6658p-8, -0x3.83d592fce66f4a78fep-60, -0x3.b6a39f229dae2p-8, -0xe.0ac94eee640f3b8d1cp-64, 0x1.915cd9fbf224dp-8, 0x5.df8c8875f3b780f148p-64, -0xb.7dde54556e568p-12, 0x3.84121393476fe3d862p-64, 0x5.8def0865f7358p-12, 0x3.19136064b09ad2587p-68, -0x2.c9d886980cb1cp-12, -0xf.615d6eaa8df51bb4e8p-68, 0x1.70a1030809ff6p-12, -0xc.28f8be8b3974p-16, 0x6.87d8e8fc790cp-16, -0x3.8eaeaa7c767eap-16, 0x9.9p-4 }, /* i=76 122.6 */
  { 0x1.77244fb4e18d5p+0, -0x7.1eb4ca51ef90e020bap-56, -0x2.4ad139238f60ap-4, -0x9.263bc285ff2c243a3cp-60, 0xa.05d14a668b23p-8, 0x1.f937ecb8cabbc73925p-60, -0x3.aa353bd432f06p-8, -0xb.db3b507becdfb405bcp-64, 0x1.8a42d2df5ce0fp-8, 0x4.4a0aa91a2255ceecaap-64, -0xb.3c1edbd273d9p-12, -0x2.e9cea79b820eba105p-64, 0x5.67847e10db328p-12, -0x1.e26cda7d342854a02ap-64, -0x2.b33a63daa67a4p-12, 0x2.b576ae53124f6acc04p-68, 0x1.633b417d535d3p-12, -0xb.a95d924c513b8p-16, 0x6.3b91e7985bd48p-16, -0x3.60edd84199d94p-16, 0x9.bp-4 }, /* i=77 122.7 */
  { 0x1.76db1d87c928ep+0, 0x2.cfe7d3eb3ad9b6749cp-56, -0x2.48528167dbb8p-4, 0x2.b41955bcc59288dc8p-60, 0x9.eff8c9675c8d8p-8, -0x2.69ea4b89092fb0c7bp-64, -0x3.9dff05dd173p-8, -0x8.68144c3a1c394677fp-64, 0x1.835154e6c011cp-8, 0x3.2815a9b9782f1892c8p-64, -0xa.fc250d1be9bdp-12, -0x3.9bbc400c092ab8c461p-64, 0x5.4250d1cef4f3cp-12, 0x1.08bf20dd1d279960fb8p-64, -0x2.9d6e32679d2b8p-12, 0xf.6de3f790e2357a6acp-72, 0x1.5661c2f32f3fep-12, -0xb.2f915497591a8p-16, 0x5.f31da7767c774p-16, -0x3.35add71cfda42p-16, 0x9.dp-4 }, /* i=78 122.7 */
  { 0x1.76923adaa77a1p+0, -0x3.ad86440ee90b38e5f8p-56, -0x2.45d936b18e7b4p-4, -0x3.7956442f87501a90e4p-60, 0x9.da68e845dfep-8, 0xc.d007fb4215a735bedp-64, -0x3.91ffbd564af2cp-8, -0xf.f71fd0eb07d0820ce4p-64, 0x1.7c87490788ba2p-8, -0x7.dcd73f5472d61189ep-64, -0xa.bde299b97507p-12, -0x1.c3008be90c8fcc5554p-64, 0x5.1e48c4daaa6fcp-12, 0x1.0bf482ceaef1d30a2ap-64, -0x2.886b6173164b8p-12, -0x1.e9db741df7c66f6008p-68, 0x1.4a0e28d4a7008p-12, -0xa.bb49aa583f0a8p-16, 0x5.ae46bbb98f774p-16, -0x3.0cc8c0d6ccb0ap-16, 0x9.fp-4 }, /* i=79 122.8 */
  { 0x1.7649a700fc9e7p+0, 0x6.e76c8a8217af57eb7p-56, -0x2.4365470193f68p-4, 0x8.5531344ece0f8e7f4cp-60, 0x9.c520611ba244p-8, 0x1.ef17ad8cbe5d76541ap-60, -0x3.86362aedce744p-8, -0x4.2642efa23067b8fb44p-64, 0x1.75e3a0fe97dd4p-8, -0xb.ec152cd6b4a924eacp-68, -0xa.8149b75f7c648p-12, 0x3.52a2e5d806aefce17ep-64, 0x4.fb618d9c2e09p-12, -0xb.dc9b52f8ef1d1cc3d8p-68, -0x2.7429c389a83c2p-12, 0xf.382b60eb012d1ed564p-68, 0x1.3e3a65e63cd52p-12, -0xa.4c4045113368p-16, 0x5.6cdadffbf577p-16, -0x2.e61b17ccbe7fcp-16, 0xa.1p-4 }, /* i=80 122.9 */
  { 0x1.7601615083886p+0, -0x5.76787f5f94abd2f7e4p-56, -0x2.40f6a0a96aadp-4, -0xa.c41d85522e445be758p-60, 0x9.b01df5350ae98p-8, 0x3.7dd9f401af7d3bf51cp-60, -0x3.7aa11fa1abef2p-8, -0x4.36d4e13bf40ec85e6cp-64, 0x1.6f6556ff573d2p-8, 0x3.a09c6a9eb3e1411bp-64, -0xa.464d1a910f368p-12, 0x1.7c9d0b1fb1bf219a6dp-64, 0x4.d990d25d9804p-12, 0xb.9486d3899eec790c3p-68, -0x2.60a1899e79dfp-12, 0x6.959febd39aed34e298p-68, 0x1.32e0b9da767e4p-12, -0x9.e232a5d4e571p-16, 0x5.2eaac1cc10f64p-16, -0x2.c1839cb42cebp-16, 0xa.3p-4 }, /* i=81 123.0 */
  { 0x1.75b969212808cp+0, -0x7.d077f3826218b69862p-56, -0x2.3e8d32495cceap-4, -0x2.9f4c46011788fc8124p-60, 0x9.9b606ce04375p-8, -0x2.652b4d21d7c895c2f2p-60, -0x3.6f3f747d7c0a2p-8, 0xe.7690591e67c0ef4554p-64, 0x1.690b6d661560cp-8, 0x7.b441a4b288ca802af4p-64, -0xa.0cdff17fc3aep-12, 0x8.ed2421c1e8f52b4f4p-68, 0x4.b8cca4425518cp-12, 0x1.a1e81167a6aac95e368p-64, -0x2.4dcb3e5e002cap-12, 0x2.383c994bcd8f422824p-68, 0x1.27fbad28db261p-12, -0x9.7ce1e41cb211p-16, 0x4.f389d3e38729cp-16, -0x2.9ee3281e83faep-16, 0xa.5p-4 }, /* i=82 123.1 */
  { 0x1.7571bdccfd11bp+0, -0x4.9b83cc71f549f0b6d8p-56, -0x2.3c28eacec5c08p-4, 0x4.348988e2f26540b5ep-60, 0x9.86e6973da822p-8, 0x3.b5d85adbeb9c0a1f72p-60, -0x3.64100a5a4ac56p-8, 0x9.9d48271e3a2b3cb8p-68, 0x1.62d4ee6d83208p-8, 0x3.bdc61dc186e314e36p-64, -0x9.d4f5df267a088p-12, 0x8.f233cbbc529be0463p-68, 0x4.990b7a7e45eep-12, 0x1.d378189d119b1dcc958p-64, -0x2.3b9fc1c154682p-12, -0xe.622373479646382144p-68, 0x1.1d860d2334bf4p-12, -0x9.1c12783804868p-16, 0x4.bb4e20c6386dp-16, -0x2.7e1c866cc141p-16, 0xa.7p-4 }, /* i=83 123.1 */
  { 0x1.752a5eb0332f5p+0, -0x1.8f4de18b34f96a732ep-56, -0x2.39c9b972635b4p-4, -0x1.5121cde8f98c73d0b4p-60, 0x9.72af4a11b1aa8p-8, -0x1.b18007eeaf3e7c3ccap-60, -0x3.5911c9a0c46eap-8, -0x8.889cc9c65fe6db9828p-64, 0x1.5cc0ebe72ed35p-8, 0x2.344ec92b00756b6bbcp-64, -0x9.9e82f69c1d6ep-12, 0x3.eb23decc10506761a6p-64, 0x4.7a442dc921d1p-12, 0xf.0707c32d5956556d8p-72, -0x2.2a1844de5e696p-12, -0xd.e34774df2482cf10cp-68, 0x1.137ae8452b69ap-12, -0x8.bf8c0916f27cp-16, 0x4.85d01efdff1acp-16, -0x2.5f1455d80c258p-16, 0xa.9p-4 }, /* i=84 123.2 */
  { 0x1.74e34b290f35cp+0, 0x5.964e8d013a9900312p-60, -0x2.376f8db6b280ap-4, 0x5.02db14e778b012d1f8p-60, 0x9.5eb961984b7ap-8, -0x3.f39dd31bf0cb83fe9ep-60, -0x3.4e43a20d9336cp-8, -0x9.a5b6f020460fce318cp-64, 0x1.56ce7ef6dafc9p-8, -0x6.0fbb3527c41ab3feb4p-64, -0x9.697bb69ba2d7p-12, 0x8.ea89f5ad709927dbdp-68, 0x4.5c6df40ae20b8p-12, 0xe.c6a4611206310d2568p-68, -0x2.192e45f14ae4ap-12, 0x7.6fdbc8861dd1a87074p-68, 0x1.09d58aba93558p-12, -0x8.67193d3685d1p-16, 0x4.52ea906ca862cp-16, -0x2.41b0e8666c57ap-16, 0xa.bp-4 }, /* i=85 123.3 */
  { 0x1.749c8297e122ep+0, 0x7.6051f418afdb27ce28p-56, -0x2.351a576656afp-4, 0x4.37fae376c3762123dp-60, 0x9.4b03c059895d8p-8, -0x1.e5cd45d51b94de18f4p-60, -0x3.43a48a77d7112p-8, -0x5.fcdb542be076989218p-64, 0x1.50fcc7d090054p-8, 0x4.874da5515c947c3252p-64, -0x9.35d5053eac068p-12, 0x3.7a81b4dc38a0a7083dp-64, 0x4.3f805c3e2b1f8p-12, 0x1.ae9c9d9647d067baf68p-64, -0x2.08db8c9bfdfdcp-12, 0xe.330d31b7775b705514p-68, 0x1.00917b190956p-12, -0x8.12878e794f1ap-16, 0x4.227a571f5a118p-16, -0x2.25da25f468b8ep-16, 0xa.dp-4 }, /* i=86 123.4 */
  { 0x1.7456045efb329p+0, 0x5.a85f01650a0c59a7cap-56, -0x2.32ca06928c3d6p-4, -0x1.7d4ad3799e31897e5cp-60, 0x9.378d4effb0478p-8, -0x2.c7a0e3668cbbb6127ep-60, -0x3.39338099a2806p-8, 0x1.e2f4f32d8601327e58p-64, 0x1.4b4aed794a21cp-8, -0x4.7b5b345f330d61c364p-64, -0x9.03842be858078p-12, 0x3.57ee03d038450a2df5p-64, 0x4.23734a85d612cp-12, 0x1.791b1236329b1ecf4a8p-64, -0x1.f91a26586716ep-12, 0x3.4e6fb89613f92ffaf2p-68, 0xf.7aa7749ac75c8p-16, -0x7.c1a720ba90fap-16, 0x3.f45e579a5ff4cp-16, -0x2.0b7971f710cfcp-16, 0xa.fp-4 }, /* i=87 123.4 */
  { 0x1.740fcfe2a923cp+0, 0x3.d8938b59feccc93fp-60, -0x2.307e8b91a4ebcp-4, 0x1.b76b28ca17bdd78ddp-60, 0x9.2454fc2e864a8p-8, 0x1.df741e981adecf6606p-60, -0x3.2eef88da67e5ap-8, 0xd.63f456c462bb9f7854p-64, 0x1.45b81d8a26027p-8, 0x1.fc6938e6773411ced4p-64, -0x8.d27ed35dec16p-12, -0x1.63a83da6bad31a7e77p-64, 0x4.083ef472e617cp-12, -0x1.755b5240041d9816dbp-64, -0x1.e9e4631acbedbp-12, -0x7.0e0bbda869c355f60ap-68, 0xe.f1c719fff9848p-16, -0x7.744a9aede6588p-16, 0x3.c87758aee23a2p-16, -0x1.f27992741d949p-16, 0xb.1p-4 }, /* i=88 123.5 */
  { 0x1.73c9e48927ad8p+0, 0x2.f862a7522d2c91ea66p-56, -0x2.2e37d6fd8e77ap-4, -0xd.aeac0f3f434764f224p-60, 0x9.1159bc5bde67p-8, -0x1.77330b5e73a8fbf971p-60, -0x3.24d7ae1b43ccap-8, 0xc.62c30cc4e02869c7d4p-64, 0x1.40438bf5f05c4p-8, 0x5.62798e96844114fdfp-64, -0x8.a2bb000b1f48p-12, -0x3.12d39fdb9dedbd298dp-64, 0x3.eddbdd785e646p-12, -0xd.ccc573cdf57ed0d7c4p-68, -0x1.db34d2214f011p-12, 0x2.6726941687b6a0a1acp-68, 0xe.6e38e1b2c97ep-16, -0x7.2a4702a07ac38p-16, 0x3.9ea7e636fa49ap-16, -0x1.dac698d2c915p-16, 0xb.3p-4 }, /* i=89 123.6 */
  { 0x1.738441ba9c22bp+0, 0x5.942d2e2f8ce10954c4p-56, -0x2.2bf5d9b262e7ep-4, -0x2.0f834eed89d14e33dcp-60, 0x8.fe9a89a9552e8p-8, -0x3.f13712c833b9a547cfp-60, -0x3.1aeb01851189ap-8, 0x7.d28c9f9316df050758p-64, 0x1.3aec72d0fdaabp-8, -0x6.e8a19ec9fe56c0e522p-64, -0x8.742f0e6ff1038p-12, -0x8.65c7adbac45edf91ap-68, 0x3.d442d38a9122ep-12, 0xd.09e3b2c96e65f8e9bp-68, -0x1.cd063eee20191p-12, 0x4.41afa65775782f5768p-68, 0xd.efc1fcf124c3p-16, -0x6.e37399b3ef76cp-16, 0x3.76d433db595cp-16, -0x1.c44dcc0d95615p-16, 0xb.5p-4 }, /* i=90 123.7 */
  { 0x1.733ee6e10c42ep+0, -0x2.a441245403ac9589cp-60, -0x2.29b884cd02436p-4, -0x7.6251a2331425418cd8p-60, 0x8.ec1663bf33bdp-8, 0x1.15ca1b4065582ae689p-60, -0x3.11289a58384cep-8, 0x5.d59f11763ee7f3aa9cp-64, 0x1.35b2121b30d57p-8, 0x2.1b280dc35515366362p-64, -0x8.46d1afb60e518p-12, -0x1.7793b2692f029d119ep-64, 0x3.bb6cebe7a339p-12, -0x6.9987efd3fc3d0d23ccp-68, -0x1.bf53ae69edde5p-12, 0x3.e3dfdaa639e860e9b6p-68, 0xd.762a672b0a06p-16, -0x6.9fa9be2dcb898p-16, 0x3.50e206193302p-16, -0x1.aefd94f8a1336p-16, 0xb.7p-4 }, /* i=91 123.7 */
  { 0x1.72f9d36856376p+0, 0x3.d41b9e85b2e8b21d5cp-56, -0x2.277fc9a9b56c6p-4, -0xc.a6864301a590ee6578p-60, 0x8.d9cc4fa86ee5p-8, 0x2.6734f2f22de5b1d94ep-60, -0x3.078f95be1f964p-8, -0x8.bb1761fab9f35455p-64, 0x1.3093af8c1898cp-8, -0x6.e82933430d6a6c3a16p-64, -0x8.1a99e66be4898p-12, -0x1.6a0085b64cc6eae686p-64, 0x3.a3538007215b4p-12, 0xd.65f7ae4f80d2f9e9b8p-68, -0x1.b2185c2c54e1fp-12, 0x8.6fd1260fb789ec57p-72, 0xd.013cc11b12ebp-16, -0x6.5ec4cbf8ef0b4p-16, 0x3.2cb8964d984d2p-16, -0x1.9ac56aef5f783p-16, 0xb.9p-4 }, /* i=92 123.8 */
  { 0x1.72b506be28bbcp+0, -0x1.aa226731ca9f314574p-56, -0x2.254b99e2d9d8cp-4, -0xa.c0413d33f90214adp-64, 0x8.c7bb57afb8d1p-8, -0x1.48d1cd2af52db7706ap-60, -0x2.fe1f169c3abp-8, 0xf.8a0fb836ca202b0c58p-64, 0x1.2b9096610cb27p-8, -0x2.1ab98df2bd4ce2513ep-64, -0x7.ef7f036398bccp-12, -0x6.7348980d74ca667b48p-68, 0x3.8bf02aaea19dep-12, -0xc.cd3f40d94a5283eb54p-68, -0x1.a54fb7e82b808p-12, 0x7.ca9b99d888f26c95dp-68, 0xc.90c62deebcafp-16, -0x6.20a20078d807cp-16, 0x3.0a40810a4c554p-16, -0x1.8795c34d7c071p-16, 0xb.bp-4 }, /* i=93 123.9 */
  { 0x1.72708051fb6fcp+0, 0x5.c9e2fc1548c73359dep-56, -0x2.231be74f95e4ap-4, 0xe.8a6d89342cbab44284p-60, 0x8.b5e28b3d9bc4p-8, -0x1.b9deb19773bd138c4p-64, -0x2.f4d645689b894p-8, 0x6.1694779a19a2ec374p-64, 0x1.26a8172d34ef2p-8, 0x7.545d1ceb6979ab9192p-64, -0x7.c578a2b432dacp-12, 0x1.5d4e04f92d5a29e6048p-64, 0x3.753cc529898eep-12, -0xd.eb29cc915cb9daae38p-68, -0x1.98f562f9aacd8p-12, -0x5.3dfdae7a9d1e7f891ap-68, 0xc.2496325eec4cp-16, -0x5.e5205fd0cd948p-16, 0x2.e963ae4c84f4p-16, -0x1.7560008c51bccp-16, 0xb.dp-4 }, /* i=94 123.9 */
  { 0x1.722c3f950752ep+0, -0x7.54529dfdb2775a694p-56, -0x2.20f0a402957e4p-4, -0x8.9f13b6d73d32501534p-60, 0x8.a440feb794fap-8, 0x1.a0bad18fb6cd12db59p-60, -0x2.ebb44ffffdf82p-8, -0xb.17f5d03dbf2469fb8p-64, 0x1.21d987ab65376p-8, -0x2.e43e3722384971a714p-64, -0x7.9c7ea8db61bdcp-12, 0x3.ccbb07051eb29fe9ep-68, 0x3.5f3364a22c12p-12, -0x1.766571489c6087518p-68, -0x1.8d052e14935f8p-12, 0x6.703d498e6b1ac8c0dap-68, 0xb.bc7e959af0cap-16, -0x5.ac209bc3035acp-16, 0x2.ca0d3b52237a4p-16, -0x1.641662f3828fp-16, 0xb.fp-4 }, /* i=95 124.0 */
  { 0x1.71e843fa3f66fp+0, 0x4.2467dd5d3ac3a32262p-56, -0x2.1ec9c248ceecap-4, 0xc.218a5a3d779ac10aap-60, 0x8.92d5cb6026fd8p-8, -0x3.4e4efd83f413aab30ep-60, -0x2.e2b8697d3d08ep-8, 0x2.3ff213250d3f1066ap-64, 0x1.1d244291bab86p-8, -0x4.2ee7f107e142883cap-64, -0x7.74893ffe525p-12, -0x5.87c7923b22913984p-68, 0x3.49ce579a8bb4cp-12, -0xb.d777eec7b02e8ec784p-68, -0x1.817b171089849p-12, 0x7.de70287b9c4cdd8c8ap-68, 0xb.585343ea2aeap-16, -0x5.7584fc0db28a4p-16, 0x2.ac296df9957d2p-16, -0x1.53abfb767ad52p-16, 0xc.1p-4 }, /* i=96 124.1 */
  { 0x1.71a48cf6497abp+0, 0x3.75cd762c6a2e5813c4p-56, -0x2.1ca734a84f71cp-4, -0x7.df0c9d8f40dfdfaeb4p-60, 0x8.81a00f37db208p-8, 0x1.85647c0834444b29e1p-60, -0x2.d9e1ca12249ecp-8, -0x4.0513a3f978db2e8c34p-64, 0x1.1887a766e731ap-8, 0x3.65df6f536100ae6c22p-64, -0x7.4d90d54827cccp-12, -0x1.585650e0496f35e5ef8p-64, 0x3.3508238326352p-12, -0xf.2dd0882cd123cb9088p-68, -0x1.765346d1ff48cp-12, 0x5.7b918ccb120c48eb9ap-68, 0xa.f7ea32ea8e81p-16, -0x5.4131482df315p-16, 0x2.8fa59f03c97cep-16, -0x1.44149d884721fp-16, 0xc.3p-4 }, /* i=97 124.1 */
  { 0x1.716119ff77194p+0, 0x3.56bc3e4590c80c446p-60, -0x2.1a88eddf0f91ep-4, 0x9.bbbc459bf5f06ed5b4p-60, 0x8.709eecdf2a1dp-8, -0x8.0c1987fe03753924ap-64, -0x2.d12faee1923f8p-8, -0x2.6daea2545967dc452p-64, 0x1.14031a591850bp-8, 0x3.b8af5210674150e914p-64, -0x7.278e1664b5e34p-12, -0x9.2f15b47761d6578f5p-68, 0x3.20db826e448ap-12, -0xa.a1b4781d4d571b4ebcp-68, -0x1.6b8a0f4e1bde2p-12, 0x1.2ee1b7a1858a47e8d4p-68, 0xa.9b1b47537c7p-16, -0x5.0f0ab271b0b1cp-16, 0x2.74702998edd8cp-16, -0x1.3544d33dde181p-16, 0xc.5p-4 }, /* i=98 124.3 */
  { 0x1.711dea8dbe9f4p+0, 0x2.9806250e4940f52f5ep-56, -0x2.186ee0e1cec2p-4, -0x6.7096addbcb93200728p-60, 0x8.5fd18b79441b8p-8, 0x2.bfa16b048be72b38fep-60, -0x2.c8a159dad86c8p-8, 0x9.0db6c2f864340f351p-64, 0x1.0f96041669db4p-8, -0x6.26e4e992308504b6bap-64, -0x7.0279ef162f35p-12, 0x1.cbbd207a83f9062b338p-64, 0x3.0d4360de5ff94p-12, -0xe.fee3d0bdd7eb9ccd34p-68, -0x1.611be9a8278adp-12, 0x2.add039dd1839656ef4p-68, 0xa.41c03c26243b8p-16, -0x4.def7c443aa674p-16, 0x2.5a78603dda726p-16, -0x1.2731d22287146p-16, 0xc.7p-4 }, /* i=99 124.3 */
  { 0x1.70dafe1ab473fp+0, -0x7.f78554098a0424efp-56, -0x2.165900daf64b8p-4, -0x4.94a6376e3a8b674834p-60, 0x8.4f37168fb0c3p-8, 0x2.c8177e5e3311d2faa1p-60, -0x2.c03611965876ep-8, 0xe.63210cfe8aecf5e7f8p-64, 0x1.0b3fd1a6d232cp-8, -0x4.fa039ca8b53d71862p-64, -0x6.de4d86e48b6bcp-12, 0x1.c7a363d1950a3b0612p-64, 0x2.fa3adbae40c8ep-12, -0x4.dd1db766732851955p-68, -0x1.5705746719575p-12, 0x7.010c05377d2e4dbbp-68, 0x9.ebb48b363018p-16, -0x4.b0e04b9dd167p-16, 0x2.41ae79eb948fep-16, -0x1.19d1701d5d446p-16, 0xc.9p-4 }, /* i=100 124.4 */
  { 0x1.7098542184652p+0, -0x2.fa326dd1132bfe77a2p-56, -0x2.14474129832bap-4, -0xc.5225bad10fc42e494p-60, 0x8.3ecebdf6c0228p-8, 0x3.9a236b62a384e6fcf7p-60, -0x2.b7ed2133473acp-8, 0x3.5ba9261d3d47a2e78p-68, 0x1.06fff447797ddp-8, 0x6.3de479fcc9b9f00412p-64, -0x6.bb023ee577fa4p-12, -0x1.394f308b79378dcf25p-64, 0x2.e7bd3e118ded4p-12, -0xd.a1335e1cd9eef1d998p-68, -0x1.4d4371c1f86c5p-12, 0x7.0f9ca7458c10eec676p-68, 0x9.98d556f682598p-16, -0x4.84ad498fa7034p-16, 0x2.2a0387cef1338p-16, -0x1.0d1a19ac3a2a7p-16, 0xc.bp-4 }, /* i=101 124.5 */
  { 0x1.7055ec1eeb262p+0, -0x2.34dd6883b16beecbp-56, -0x2.1239955ff6becp-4, 0xb.57aca64f6a5459bf98p-60, 0x8.2e97b5b2c59c8p-8, 0x3.0d74a421f4e265f812p-60, -0x2.afc5d83691a8ep-8, -0x6.ae31fae8712c51b2b8p-64, 0x1.02d5e1476c7a9p-8, -0x4.bfd7c08a4e512d39p-68, -0x6.9891af9bb595cp-12, 0x5.55477ec7b08c1fa8e8p-68, 0x2.d5c5ffac96cfep-12, 0x5.c760cad9232f4d2a58p-68, -0x1.43d2c601d678ep-12, 0x6.b6e988ccd170c9c65cp-68, 0x9.490155775b9a8p-16, -0x4.5a48e1c7770c8p-16, 0x2.136966ceecee8p-16, -0x1.0102c88bd54fap-16, 0xc.dp-4 }, /* i=102 124.5 */
  { 0x1.7013c5912fef8p+0, 0x7.948d424df5a55f3704p-56, -0x2.102ff1434e032p-4, 0xd.5c38e69e07d72fc2p-60, 0x8.1e9135de1631p-8, -0xb.f183eaf8f7790f08dp-64, -0x2.a7bf8a6ad67a8p-8, -0xf.3fc27e800f20bdafd4p-64, 0xf.ec111e59cab58p-12, -0x5.55e60914a1851de66p-68, -0x6.76f5a6ecd2a2cp-12, -0x1.5bec5e4a195838c819p-64, 0x2.c450c2c23102p-12, 0x4.15d6e5cde25727dbap-68, -0x1.3ab075f83aaefp-12, 0x5.0f169e32bfee65ff54p-68, 0x8.fc18bc8430c28p-16, -0x4.319e4b0e6b3d4p-16, 0x1.fdd2b52c1089cp-16, -0xf.582fb08f7f1p-20, 0xc.fp-4 }, /* i=103 124.6 */
  { 0x1.6fd1dff81e402p+0, -0x5.9134ceae9d9839d7aap-56, -0x2.0e2a48c9ff458p-4, -0x8.196a1aa4d72712b648p-60, 0x8.0eba7a8fc3c7p-8, 0x3.43567fe2d44d815373p-60, -0x2.9fd98fc16adccp-8, 0x3.eba80699219863d164p-64, 0xf.ac1033020362p-12, -0x2.95e1e5d3c7e12937c6p-64, -0x6.5628262c4041p-12, -0x3.80575e19b6f0f65d98p-68, 0x2.b359527693672p-12, 0xd.c28b1bd03b6a061eecp-68, -0x1.31d9a588e5f94p-12, -0x1.aba66f9083de43aa7p-68, 0x8.b1fd2ed0b6388p-16, -0x4.0a99c0a882d74p-16, 0x1.e932c63b954d2p-16, -0xe.a92abcfe31d58p-20, 0xd.1p-4 }, /* i=104 124.7 */
  { 0x1.6f903ad4ffbd6p+0, 0x1.6822c081fed6975d4cp-56, -0x2.0c28901afe076p-4, -0x4.341eb435abad97c2f4p-60, 0x7.ff12c3c30f55p-8, -0x1.b145651c160e3fdc37p-60, -0x2.9813443460492p-8, -0xc.04def8aee284c50ef4p-64, 0xf.6d535e4a46a38p-12, -0x2.a237b81c8ce3625945p-64, -0x6.3623603acc4a8p-12, -0x1.8803944c4563a0ebbp-68, 0x2.a2dba126273e6p-12, -0x4.6696161d09de36cb98p-68, -0x1.294b9645e9153p-12, -0x1.1a28cec99e389ad0b4p-68, 0x8.6a91aa258bc18p-16, -0x3.e528748a73f36p-16, 0x1.d57d9922e564ap-16, -0xe.02a4a5bfb8548p-20, 0xd.3p-4 }, /* i=105 124.8 */
  { 0x1.6f4ed5aa96338p+0, -0x3.505c1d46315ef6dfb4p-56, -0x2.0a2abb8cc4f2p-4, 0x2.91bb355644d0d4bd0cp-60, 0x7.ef99553f8bf44p-8, -0x1.f5f1d08149dc76c2e58p-60, -0x2.906c07a9822a4p-8, -0xa.4bbfbad856d06c346cp-64, 0xf.2fd2e52067798p-12, -0x1.e4c0c8e3b16e60121dp-64, -0x6.16e1b7b996ac4p-12, 0x1.7bd7529dbb36717a0b8p-64, 0x2.92d3c6cf5568cp-12, 0x9.6e5b45ce5e015e409p-68, -0x1.2103a61d14467p-12, 0x6.2e7b309fa8e53de0b2p-68, 0x8.25ba767dfc558p-16, -0x3.c13882586581ap-16, 0x1.c2a7d013f3dd2p-16, -0xd.642b418d4c2d8p-20, 0xd.5p-4 }, /* i=106 124.7 */
  { 0x1.6f0daffd15b35p+0, -0x1.484aa516d8e6e46c6p-56, -0x2.0830bfa465abep-4, -0x4.cd7484a9799c1b89cp-64, 0x7.e04d7681ed38p-8, -0x1.020d1fe76efafc9fea8p-60, -0x2.88e33dd642488p-8, 0x5.c375728bb0f0eb1a7p-64, 0xe.f38743b06706p-12, -0x3.d775cc3d5ea66a0b88p-64, -0x5.f85dbd4fa3d9p-12, 0x1.a1399633b9817b7c55p-64, 0x2.833dff8e637fap-12, 0xf.45db62273952e054f4p-68, -0x1.18ff4e15d67bap-12, -0x4.8cf8408d638ede608p-68, 0x7.e35d1609319fp-16, -0x3.9eb8e3214dbdcp-16, 0x1.b0a6a23a0317ap-16, -0xc.cd52d16de84a8p-20, 0xd.7p-4 }, /* i=107 124.9 */
  { 0x1.6eccc9521ece5p+0, 0x2.cae6e35841f902e62cp-56, -0x2.063a91149e65ap-4, 0xb.947496e03731759f6p-60, 0x7.d12e72a57b3f4p-8, 0x1.68db92522924907712p-60, -0x2.81784e248b5c4p-8, -0x2.bfc8085413e18daf9cp-64, 0xe.b8692ba0935p-12, 0x2.aa303ebd116cf90cbp-64, -0x5.da922e0138e74p-12, -0x1.4bdcd9b43e6d3fe089p-64, 0x2.7416aa2a80b9ep-12, -0x3.5f655e1bca9dbdcp-80, -0x1.113c211eaeff9p-12, 0x2.38f84e1a9c68c6b66ep-68, 0x7.a3603601cd21p-16, -0x3.7d9961cb81aa2p-16, 0x1.9f6fdab62bceep-16, -0xc.3db5aafcad9bp-20, 0xd.9p-4 }, /* i=108 124.9 */
  { 0x1.6e8c2130b8ef7p+0, -0x7.4fe72109176175708ap-56, -0x2.044824bcf4f54p-4, 0xd.543789a246f9e71428p-60, 0x7.c23b984e273b4p-8, 0x1.7fcdbd44ef2b12046ap-60, -0x2.7a2aa3987179ep-8, 0xd.adf3d55c30f80be908p-64, 0xe.7e71825de6cbp-12, -0x2.f0e9902e713c13995cp-64, -0x5.bd79f1983820cp-12, 0x1.d188beec6253a67226p-64, 0x2.655a46b32e5ecp-12, -0xa.92c17e4af8b5b3202p-68, -0x1.09b7cae961483p-12, 0x6.079dc26706af93eed6p-68, 0x7.65aba0501d20cp-16, -0x3.5dca9027cd994p-16, 0x1.8ef9c983ad9e9p-16, -0xb.b4f3cf5dee878p-20, 0xd.bp-4 }, /* i=109 125.0 */
  { 0x1.6e4bb7214ccfep+0, 0x5.fec2690f3b325f8b3p-56, -0x2.02596fa8d7596p-4, 0xb.d70c342f67068897dp-60, 0x7.b37439933b4dp-8, -0x1.94516f3b25040baf268p-60, -0x2.72f9acb6b85f6p-8, 0x5.7acbe6a9f32d81be9cp-64, 0xe.45995f780259p-12, 0x3.5f6f01e7d67890fb0ep-64, -0x5.a110191cbe78p-12, -0xa.f014b293e88d2f264p-68, 0x2.5705752d3a96ep-12, 0x6.1432b6ca7a400c7d14p-68, -0x1.02700ed5262d5p-12, 0x7.a8bee6652cd332e128p-68, 0x7.2a282deb4b25cp-16, -0x3.3f3dbca10e6aep-16, 0x1.7f3b405c94ef3p-16, -0xb.32b29bc541fp-20, 0xd.dp-4 }, /* i=110 125.0 */
  { 0x1.6e0b8aad9f084p+0, 0x2.9efd2b776c82e73738p-56, -0x2.006e670ec17d2p-4, -0x2.72f9076bb7e832c4a4p-60, 0x7.a4d7abeaa0ce8p-8, 0x6.8ab3eebe66dddeeb6p-64, -0x2.6be4db6c2801p-8, -0xa.60ebfe3c4e1e471a48p-64, 0xe.0dda0b0c22e7p-12, 0x3.1c9c98a503b362f03ap-64, -0x5.854fdd5d4b91p-12, -0xb.b7c82dc145fbc1ddp-72, 0x2.4914f44e8eep-12, 0x6.2163f31748597249d4p-68, -0xf.b62c6e620d0d8p-16, 0x1.a7352f03725a5b73e3p-68, 0x6.f0bfb9ef0aba4p-16, -0x3.21e4e87ef3bfp-16, 0x1.702b892264734p-16, -0xa.b69c73549a9d8p-20, 0xd.fp-4 }, /* i=111 125.2 */
  { 0x1.6dcb9b60cabbcp+0, -0x5.5b8613fcf2e4159972p-56, -0x1.fe87004f68188p-4, -0x6.f61b59b07aa9a3428ap-60, 0x7.96654814b84a4p-8, 0x1.b426d2f674cf7cd868p-60, -0x2.64eba4f5a7fb2p-8, -0x7.69bc9f0a01d21455a4p-64, 0xd.d72cfc3e7a92p-12, 0xe.e4d8b4cc600dd92dp-72, -0x5.6a349d85c6af4p-12, 0xa.e147c52a1efb7ff9f8p-68, 0x2.3b85a0481d464p-12, -0xe.d8cfb7be78188085ccp-68, -0xf.48de2c967dd88p-16, -0x1.b4551fb89ddc11c3bp-72, 0x6.b95d155b9cep-16, -0x3.05b2beb322b84p-16, 0x1.61c26257ffebfp-16, -0xa.4060794bdecbp-20, 0xe.1p-4 }, /* i=112 125.2 */
  { 0x1.6d8be8c73c5e4p+0, 0x4.4100f0cc3c0cc83f5p-56, -0x1.fca330f4e888ap-4, -0x1.d995259992e599f1e4p-60, 0x7.881c6a08be888p-8, 0xb.b72128aac182d5577p-64, -0x2.5e0d81c91add2p-8, 0x6.e848ab17dead60313p-64, 0xd.a18bd7c15c04p-12, -0x3.00ee4eb32dd083d668p-64, -0x5.4fb9ddc4bb2d8p-12, 0xd.35c55cd74533fb703p-68, 0x2.2e54719d40d98p-12, 0x2.ef7b95368f5f47f068p-68, -0xe.def66e4fd7c08p-16, -0x2.c2a86d1eb75857833fp-68, 0x6.83ebfb76ddb7cp-16, -0x2.ea9a8b2887feep-16, 0x1.53f7f3ee5cabp-16, -0x9.cfb23ea1f3a2p-20, 0xe.3p-4 }, /* i=113 125.2 */
  { 0x1.6d4c726eac944p+0, -0x3.0c005fdd4e5d5ed16p-56, -0x1.fac2eeb1fd7ddp-4, -0x6.cd9a4308f7c0c1df2p-64, 0x7.79fc70e1ba4b4p-8, 0x1.8042474f55f24cf0918p-60, -0x2.5749ed7ef2876p-8, 0xc.948eff7789900b7a7p-64, 0xd.6cf06e69acf08p-12, 0x2.1b3cc4976780e768fep-64, -0x5.35db45fe2eed8p-12, 0xb.a96403914814a6d4p-72, 0x2.217e7c07eea3p-12, 0x9.d534dae411c7b5064cp-68, -0xe.7856b731986d8p-16, -0x2.9211255b30e7daa5cdp-68, 0x6.505906c54cd6cp-16, -0x2.d090328d213bap-16, 0x1.46c4cd3dd31e2p-16, -0x9.644987d4c7fc8p-20, 0xe.5p-4 }, /* i=114 125.2 */
  { 0x1.6d0d37e61b2b3p+0, 0x5.622cea774714dc2984p-56, -0x1.f8e62f61385dap-4, -0xe.01b50ad49cb3d7ed4p-64, 0x7.6c04becbee76p-8, 0x1.96244448ff9deb8a418p-60, -0x2.50a066bc771a2p-8, 0xd.ba5dd91a799e4a4e9cp-64, 0xd.3954bbd01aa58p-12, -0x4.b2f185a15955c1c32p-68, -0x5.1c94a08b7ca18p-12, -0x1.c674faa437d5fe44f2p-64, 0x2.1500ed691cae8p-12, -0x3.9ba2314dd9fd708c5cp-68, -0xe.14e1ba832223p-16, -0x2.d86d7fef87a3d00db5p-68, 0x6.1e91a692dcc6p-16, -0x2.b7882a8f0df38p-16, 0x1.3a21ddc3c840cp-16, -0x8.fde20aad9148p-20, 0xe.7p-4 }, /* i=115 125.4 */
  { 0x1.6cce38bdca2b4p+0, 0x7.63bedd9ef8f1de529ep-56, -0x1.f70ce9043f382p-4, 0x3.19ea6c6a61389f622cp-60, 0x7.5e34b8f2cc82cp-8, -0x1.69841536fde98309db8p-60, -0x2.4a106f1eba33ep-8, -0xd.56071d5b8c95cc715p-64, 0xd.06b2e4fe91548p-12, -0x1.74c58d693d9e767ap-64, -0x5.03e1d907a2ddcp-12, 0x1.61ace928d4f4cb6b66p-64, 0x2.08d90cc4cc9aap-12, 0xf.9590b435d3d8104p-80, -0xd.b47b4e336f918p-16, -0x2.af7ed1802c0d781739p-68, 0x5.ee841503954ep-16, -0x2.9f77728622e0ap-16, 0x1.2e0870fae4c64p-16, -0x8.9c3b34d5c9e88p-20, 0xe.9p-4 }, /* i=116 125.5 */
  { 0x1.6c8f748739006p+0, 0x5.da3781e570bdd788acp-56, -0x1.f53711c30f30dp-4, -0x7.5ca626a9c385c7046ap-60, 0x7.508bc76f634d8p-8, -0x7.e23d3cd6b51b481578p-64, -0x2.43998b26307dap-8, 0x8.d9d97300eca00a3fe8p-64, 0xc.d50537297c3dp-12, -0x2.123b2409ac957a72fap-64, -0x4.ebbefb277f8f8p-12, -0xd.bf7caa869eedc1b3ap-68, 0x1.fd0439492e90cp-12, 0x7.797cdcb60c711abffap-68, -0xd.57085e77b027p-16, 0x1.412de9c41dceb983bfp-68, 0x5.c01f4da4ad91cp-16, -0x2.88538c83aaf58p-16, 0x1.227228b21b496p-16, -0x8.3f17f3a7ba18p-20, 0xe.bp-4 }, /* i=117 125.5 */
  { 0x1.6c50ead51fbb2p+0, -0x6.3e63d5db2040214862p-56, -0x1.f3649feb433d2p-4, -0x2.94f737b2a19a36dd88p-60, 0x7.430955374664p-8, -0xb.c5cc2ea76057df299p-64, -0x2.3d3b4222ebbbcp-8, 0x5.912f204ec5dd36a8bp-64, 0xc.a44626745a038p-12, 0x3.6866c5703cafa77a1dp-64, -0x4.d428319d75d98p-12, 0x7.d496378f6016448a8p-68, 0x1.f17fe960588aap-12, -0x6.0f7c4384cc8a89cb08p-68, -0xc.fc6ee1fcfe09p-16, 0x1.b14dfe63db3f55fe5p-68, 0x5.9353047720428p-16, -0x2.721276c26497ep-16, 0x1.1758f787b308ep-16, -0x7.e63e7e57267bcp-20, 0xe.dp-4 }, /* i=118 125.5 */
  { 0x1.6c129b3b6a682p+0, 0x3.b51399eee26774b35ap-56, -0x1.f19589ef5f19cp-4, -0x5.a44b5db43db2ac66fcp-60, 0x7.35acd00bea23p-8, -0x1.967634028b474853b6p-60, -0x2.36f51e216fd9p-8, 0xa.beb543f532ebe1aap-68, 0xc.74704cc13584p-12, -0x3.de9c90904a2a411495p-64, -0x4.bd19c50802518p-12, -0x3.d10b2eeb3dafa2dedp-68, 0x1.e649a9cc14727p-12, 0x3.4d433fa89cdd54f5ecp-68, -0xc.a495cea5cb318p-16, 0x6.48942fb7c66adcc08p-72, 0x5.680f9d6d21a68p-16, -0x2.5caaa57126ee8p-16, 0x1.0cb71df9bc2afp-16, -0x7.91782847aa228p-20, 0xe.fp-4 }, /* i=119 125.7 */
  { 0x1.6bd4854f347e8p+0, -0x3.3a8337f26c9bfedf48p-56, -0x1.efc9c6661e5b7p-4, 0x5.e7294602d56a7a16ep-64, 0x7.2875a86a7113p-8, 0x1.0a5c5f3ffaaffe5286p-60, -0x2.30c6abd81d90ep-8, -0xc.8181e928677c6b398p-64, 0xc.457e688a981bp-12, -0x9.d4f076fb7ba433766p-68, -0x4.a6901aeac7658p-12, -0xf.50a7a2b6c0b2c8dbep-68, 0x1.db5f1ccb4df3dp-12, 0x2.346e4b0436d095ff52p-68, -0xc.4f650ecc0928p-16, -0x3.cd65404e295f92fae3p-68, 0x5.3e462454524a4p-16, -0x2.4812fcd2f58eap-16, 0x1.028725fa13a56p-16, -0x7.4091330bb3ca4p-20, 0xf.1p-4 }, /* i=120 125.8 */
  { 0x1.6b96a8a6c4633p+0, 0x5.0a970e7f2cd9ed9aaep-56, -0x1.ee014c09c77e1p-4, 0x7.d464d03b871440292cp-60, 0x7.1b63517be711cp-8, 0xb.07d499fba45384ca48p-64, -0x2.2aaf7a952d98cp-8, 0xc.e58d68a00a4968bc44p-64, 0xc.176b5bc78efbp-12, 0x3.29de1408c4a433783cp-64, -0x4.9087b4b1a1144p-12, 0x1.7682e4495ab33a6ff7p-64, 0x1.d0bdf948aec3cp-12, 0x1.38dcd162dd945a8008p-68, -0xb.fcc577016efa8p-16, 0x1.0eecb072f72eb815dp-68, 0x5.15e84526d5918p-16, -0x2.3442cbaf99448p-16, 0xf.8c3de5be7607p-20, -0x6.f358a21549f88p-20, 0xf.3p-4 }, /* i=121 125.8 */
  { 0x1.6b5904d98702bp+0, 0x1.c5c8c14396e8dd3a82p-56, -0x1.ec3c11b782d7dp-4, -0x5.57671e0774fad60926p-60, 0x7.0e754105e6f18p-8, 0x1.beb33416803cdb5a548p-60, -0x2.24af1c2d37552p-8, 0x4.bcc5f03c718719c83cp-64, 0xb.ea322ad961538p-12, 0x3.a1681b4c8092bc376ep-64, -0x4.7afd2ebd5458p-12, 0x1.09e5222f9f04fd7515p-64, 0x1.c6640a11fdc9fp-12, 0x4.37d9d39795ce05ccbp-68, -0xb.aca0bc479bae8p-16, 0x3.e21afb0a9b0ef2d2fap-68, 0x4.eee844bdc93b8p-16, -0x2.2131c61021a2ep-16, 0xe.f68575d780188p-20, -0x6.a9a011e3f68dp-20, 0xf.5p-4 }, /* i=122 125.7 */
  { 0x1.6b1b99800b7edp+0, 0x4.164447102409afcd2p-60, -0x1.ea7a0e6eb5564p-4, -0x7.ebf320a56064bfb1cp-64, 0x7.01aaef5ba75bcp-8, 0xf.ed6b1f4610752b63d8p-64, -0x2.1ec524ea3e5cep-8, 0xc.6f1ed7ccef62ba1fdp-64, 0xb.bdcdfb829944p-12, -0xb.00a642dc338f5c8abp-68, -0x4.65ed3f79738dcp-12, 0x1.5f84ad02da05e8a896p-64, 0x1.bc4f2d17dae72p-12, -0x5.a8c62428723945p-72, -0xb.5ee16aba28dbp-16, -0x2.1874803ca4519ceff7p-68, 0x4.c938f9dff1eacp-16, -0x2.0ed80042faeb4p-16, 0xe.66fdf2d360d18p-20, -0x6.633b93f3caef8p-20, 0xf.7p-4 }, /* i=123 125.9 */
  { 0x1.6ade6633fef17p+0, 0x5.d16674c9af0b94fd42p-56, -0x1.e8bb39505eeadp-4, 0x2.5f231a2b6f186bd012p-60, 0x6.f503d74f6bd8p-8, 0x1.9f5d6daa73b0630315p-60, -0x2.18f12b7b42348p-8, 0xf.7ae8c36cd99d65c018p-64, 0xb.923a13e705738p-12, 0x3.8bfca2b60d5b8068f2p-64, -0x4.5154b67b15cap-12, 0x1.83a44bb07f7e10930ep-64, 0x1.b27d52b576204p-12, 0x5.4bf61b3d7052a4fb14p-68, -0xb.1372dca5119e8p-16, 0x5.24547e2ffc5ab358p-72, 0x4.a4cdc6a7b610cp-16, -0x1.fd2dea23882d9p-16, 0xd.dd5ff8838a598p-20, -0x6.20018b8113e8cp-20, 0xf.9p-4 }, /* i=124 125.9 */
  { 0x1.6aa16a9028432p+0, -0x7.8414a0f94ed0af18fp-56, -0x1.e6ff899e7c8fap-4, 0x3.9e11e6af39b1b69c4cp-60, 0x6.e87f762456fa4p-8, -0x1.6d3e0c710666c64dc28p-60, -0x2.1332c8e44bdd8p-8, -0x9.c3bf9e9af144375d7cp-64, 0xb.6771d9945ceb8p-12, -0x2.cc5bc54a4c4735ede4p-64, -0x4.3d307ba80384cp-12, -0x3.d6fca78bdba7f15bcp-68, 0x1.a8ec7cffe5bd7p-12, 0x5.e000fcde0c04a6d8ap-72, -0xa.ca4132021e1e8p-16, -0x3.ee15f9c5ad67cb6b49p-68, 0x4.819a923bd50b8p-16, -0x1.ec2c4aa17005dp-16, 0xd.59678aeefe5bp-20, -0x5.dfca887f5932p-20, 0xf.bp-4 }, /* i=125 126.1 */
  { 0x1.6a64a6306414dp+0, -0x1.2728c849ad57b51148p-56, -0x1.e546f6bb6dcb8p-4, -0x6.78bcc9b64aceab1d7cp-60, 0x6.dc1d4b809acfp-8, 0xb.e063d6aca19e3be5f8p-64, -0x2.0d89986ef4fcap-8, -0xf.20e58a3606b2c2e1a4p-64, 0xb.3d70d09332588p-12, -0x3.61cb62faa1391338e1p-64, -0x4.297d8e65ff3c4p-12, -0xb.5cad6b584356f728dp-68, 0x1.9f9abf1cc3708p-12, 0x7.5b7c7241a81002a1e6p-68, -0xa.833948584b7cp-16, -0x2.6e6ff17b906b3b0fd1p-68, 0x4.5f93c2d66b9fcp-16, -0x1.dbcc3b84025fdp-16, 0xc.dad414bd0d0c8p-20, -0x5.a2712f93001cp-20, 0xf.dp-4 }, /* i=126 126.2 */
  { 0x1.6a2818c0bd494p+0, -0x4.56db480b4368ceeaf6p-56, -0x1.e39178965b6c5p-4, -0x4.20bb3f4eeda06f112ap-60, 0x6.cfdcdc6bf4918p-8, 0x1.854e6a9a87f2c589998p-60, -0x2.07f538fdead68p-8, 0xf.6663706c5be74c573p-64, 0xb.1432a4b775e7p-12, 0x3.37638caafde049b51p-64, -0x4.16390995676c4p-12, -0x1.951a08db4b9fb024f88p-64, 0x1.96863edd5ccb1p-12, 0x4.c97a59d57ef626d41ep-68, -0xa.3e48c3f324a28p-16, -0x2.f34dd5d39f24ddaa2ep-68, 0x4.3eae402c67548p-16, -0x1.cc072944ca63dp-16, 0xc.61682c3d319p-20, -0x5.67d221e8a35b4p-20, 0xf.effff8p-4 }, /* i=127 126.2 */
};

typedef union { double x; uint32_t i[2]; } union_t;

// i[HIGH] contains the exponent, i[LOW] the low part of the significant
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define HIGH 1
#define LOW 0
#else
#define HIGH 0
#define LOW 1
#endif

// begin_acospi
// exceptional cases for |x| < 0.5, should be sorted by increasing values
#define EXCEPTIONS 114
static const double exceptions[EXCEPTIONS][2] = {
    {-0x1.ac26cc49b0264p-3, 0x1.22530cb92c8f1p-1},
    {-0x1.f8374b05d52b2p-5, 0x1.0a09933a4589ap-1},
    {-0x1.7b9482807fe3cp-5, 0x1.078de0c9ad10cp-1},
    {-0x1.df5c9cfec76e3p-6, 0x1.04c4dbe40b566p-1},
    {-0x1.d8a6b018a513dp-7, 0x1.0259d1a82f985p-1},
    {-0x1.f0a606701a3e5p-9, 0x1.009e16a90fcbep-1},
    {-0x1.f7b97997721efp-10, 0x1.00502b98793edp-1},
    {-0x1.1c68c1ab0ea57p-10, 0x1.002d43e032ad6p-1},
    {-0x1.59ec5d1d2e86p-11, 0x1.001b87178f037p-1},
    {-0x1.2878e662f31bcp-12, 0x1.000bcbd77c281p-1},
    {-0x1.83d790f27f3e5p-14, 0x1.0003dba1b6271p-1},
    {-0x1.dd61dedc265f5p-15, 0x1.00025fd270a53p-1},
    {-0x1.29320aa46f06ep-15, 0x1.00017a66a9fb7p-1},
    {-0x1.99bc3ca2a27p-17, 0x1.0000826c3c68fp-1},
    {-0x1.8ec2e3d383d64p-19, 0x1.00001fbb7f8eap-1},
    {-0x1.7679a41f39c67p-19, 0x1.00001dccbecdcp-1},
    {-0x1.427b32daed80dp-20, 0x1.00000cd4c4593p-1},
    {-0x1.2a4da90d1ca6p-21, 0x1.000005ef3f1d5p-1},
    {-0x1.c0149992fc291p-22, 0x1.00000475070d1p-1},
    {-0x1.783beb73b1516p-23, 0x1.000001df093cep-1},
    {-0x1.5b9604ff5f529p-23, 0x1.000001ba8f6d1p-1},
    {-0x1.3a21ab6af61a5p-23, 0x1.0000018ff6f5p-1},
    {-0x1.71732d4eb3deap-25, 0x1.00000075997cbp-1},
    {-0x1.c68b45cd67cb4p-28, 0x1.0000001215f29p-1},
    {-0x1.dbbf2a5ec6647p-29, 0x1.0000000976f55p-1},
    {-0x1.d0e597b6ba3b6p-30, 0x1.000000049fd9ap-1},
    {-0x1.10c87354e03c8p-31, 0x1.000000015b517p-1},
    {-0x1.c277920f3fbadp-32, 0x1.000000011ec6bp-1},
    {-0x1.74c46decf3374p-32, 0x1.00000000ed4fap-1},
    {-0x1.65a1dd290660fp-36, 0x1.000000000e3adp-1},
    {-0x1.635e3d74befcap-38, 0x1.000000000388fp-1},
    {-0x1.635e3d74befcap-39, 0x1.0000000001c47p-1},
    {-0x1.6c6cbc45dc8dep-40, 0x1.0000000000e8p-1},
    {-0x1.5a4fbea3a16b6p-40, 0x1.0000000000dc7p-1},
    {-0x1.6c6cbc45dc8dep-41, 0x1.000000000074p-1},
    {-0x1.6c6cbc45dc8dep-42, 0x1.00000000003ap-1},
    {-0x1.6c6cbc45dc8dep-43, 0x1.00000000001dp-1},
    {-0x1.6c6cbc45dc8dep-44, 0x1.00000000000e8p-1},
    {-0x1.6c6cbc45dc8dep-45, 0x1.0000000000074p-1},
    {-0x1.6c6cbc45dc8dep-46, 0x1.000000000003ap-1},
    {-0x1.6c6cbc45dc8dep-47, 0x1.000000000001dp-1},
    {-0x1.921fb54442d18p-51, 0x1.0000000000002p-1},
    {-0x1.2d97c7f3321d2p-51, 0x1.0000000000001p-1},
    {-0x1.921fb54442d18p-52, 0x1.0000000000001p-1},
    {-0x1.921fb54442d18p-53, 0x1p-1},
    {0x0p+0, 0x1p-1},
    {0x1.921fb54442d19p-54, 0x1.fffffffffffffp-2},
    {0x1.3a28c59d5433bp-49, 0x1.ffffffffffff3p-2},
    {0x1.6c6cbc45dc8dep-49, 0x1.ffffffffffff1p-2},
    {0x1.3a28c59d5433bp-48, 0x1.fffffffffffe7p-2},
    {0x1.6c6cbc45dc8dep-48, 0x1.fffffffffffe3p-2},
    {0x1.6c6cbc45dc8dep-47, 0x1.fffffffffffc6p-2},
    {0x1.6c6cbc45dc8dep-46, 0x1.fffffffffff8cp-2},
    {0x1.21cfda23b228p-45, 0x1.fffffffffff47p-2},
    {0x1.6c6cbc45dc8dep-45, 0x1.fffffffffff18p-2},
    {0x1.6c6cbc45dc8dep-44, 0x1.ffffffffffe3p-2},
    {0x1.6c6cbc45dc8dep-43, 0x1.ffffffffffc6p-2},
    {0x1.b4e0b2cec917ep-43, 0x1.ffffffffffba7p-2},
    {0x1.6c6cbc45dc8dep-42, 0x1.ffffffffff8cp-2},
    {0x1.90a6b78a52d2ep-42, 0x1.ffffffffff807p-2},
    {0x1.6c6cbc45dc8dep-41, 0x1.ffffffffff18p-2},
    {0x1.6c6cbc45dc8dep-40, 0x1.fffffffffe3p-2},
    {0x1.67e57cdd4dc54p-39, 0x1.fffffffffc6b8p-2},
    {0x1.67e57cdd4dc54p-38, 0x1.fffffffff8d71p-2},
    {0x1.0dec1da5fa53fp-37, 0x1.fffffffff5429p-2},
    {0x1.93c05c9ed3cbcp-36, 0x1.ffffffffdfdedp-2},
    {0x1.39c6fd67805a7p-35, 0x1.ffffffffce0f9p-2},
    {0x1.a9adcc7f96cfp-35, 0x1.ffffffffbc405p-2},
    {0x1.39c6fd67805a7p-34, 0x1.ffffffff9c1f2p-2},
    {0x1.44bdb557e1dc1p-34, 0x1.ffffffff98a1bp-2},
    {0x1.bf9b3c6059d24p-34, 0x1.ffffffff7185cp-2},
    {0x1.ca91f450bb53ep-34, 0x1.ffffffff6e085p-2},
    {0x1.57bfc3f20ac4fp-32, 0x1.fffffffe4a533p-2},
    {0x1.74c46decf3374p-32, 0x1.fffffffe2560cp-2},
    {0x1.10c87354e03c8p-31, 0x1.fffffffd495d2p-2},
    {0x1.7ff87542efc93p-31, 0x1.fffffffc2e3a2p-2},
    {0x1.b09687e430521p-29, 0x1.ffffffeec9b1dp-2},
    {0x1.dbbf2a5ec6647p-29, 0x1.ffffffed12156p-2},
    {0x1.cc4070a3023a7p-28, 0x1.ffffffdb5fd43p-2},
    {0x1.8d69f36b1cce6p-27, 0x1.ffffffc0bfe6fp-2},
    {0x1.bdbf63f170fep-27, 0x1.ffffffb90e9bbp-2},
    {0x1.ac57910d7e867p-26, 0x1.ffffff77a78c9p-2},
    {0x1.71732d4eb3deap-25, 0x1.ffffff14cd06ap-2},
    {0x1.24e3786d7459ap-24, 0x1.fffffe8b152c5p-2},
    {0x1.94ee8417af86ep-23, 0x1.fffffbf8d9921p-2},
    {0x1.2a4da90d1ca6p-21, 0x1.fffff42181c56p-2},
    {0x1.769cbdfa9e1d8p-21, 0x1.fffff1183b0ffp-2},
    {0x1.427b32daed80dp-20, 0x1.ffffe656774dap-2},
    {0x1.73e9e2254c98fp-20, 0x1.ffffe267702edp-2},
    {0x1.8ec2e3d383d64p-19, 0x1.ffffc08900e2cp-2},
    {0x1.2226500110692p-18, 0x1.ffffa3a47a927p-2},
    {0x1.4c2c402a28e67p-18, 0x1.ffff96441ee89p-2},
    {0x1.59abdcf048a9bp-17, 0x1.ffff23f0548e1p-2},
    {0x1.5ef5605b74f7p-16, 0x1.fffe41255058bp-2},
    {0x1.969df9fda1b9p-16, 0x1.fffdfa476ec5cp-2},
    {0x1.29320aa46f06ep-15, 0x1.fffd0b32ac091p-2},
    {0x1.6638f8a2185cbp-15, 0x1.fffc6fcb3553bp-2},
    {0x1.dd61dedc265f5p-15, 0x1.fffb405b1eb59p-2},
    {0x1.41c4a3005149fp-14, 0x1.fff9993f9b0cfp-2},
    {0x1.17485bab7607bp-13, 0x1.fff4e34013127p-2},
    {0x1.a0ea629bbf42p-13, 0x1.ffef69553769ep-2},
    {0x1.2878e662f31bcp-12, 0x1.ffe8685107afep-2},
    {0x1.cf88e9268bdcfp-12, 0x1.ffdb1cf235d97p-2},
    {0x1.0d0c56d741955p-9, 0x1.ff54b7ee08148p-2},
    {0x1.f0a606701a3e5p-9, 0x1.fec3d2ade0684p-2},
    {0x1.730786d8eea35p-8, 0x1.fe27969253c05p-2},
    {0x1.bfa7fc1b1e67cp-8, 0x1.fdc605a201b7bp-2},
    {0x1.271adcddb2534p-7, 0x1.fd1082e4a2bcdp-2},
    {0x1.1f6bbb1fac712p-6, 0x1.fa481a48acacep-2},
    {0x1.20588554d6e24p-6, 0x1.fa43642302402p-2},
    {0x1.5fa453f6bb40ep-6, 0x1.f900f7075f3f5p-2},
    {0x1.69a68ac73eaa2p-4, 0x1.e32ee8bcc5f06p-2},
    {0x1.da83e335e379ap-3, 0x1.b3c936985f665p-2},
    {0x1.f608aa4e62781p-2, 0x1.58fc14707d797p-2},
  };
static const int8_t exceptions_rnd[EXCEPTIONS] = {
    1, /* -0x1.ac26cc49b0264p-3 */
    -1, /* -0x1.f8374b05d52b2p-5 */
    -1, /* -0x1.7b9482807fe3cp-5 */
    -1, /* -0x1.df5c9cfec76e3p-6 */
    -1, /* -0x1.d8a6b018a513dp-7 */
    -1, /* -0x1.f0a606701a3e5p-9 */
    1, /* -0x1.f7b97997721efp-10 */
    -1, /* -0x1.1c68c1ab0ea57p-10 */
    1, /* -0x1.59ec5d1d2e86p-11 */
    1, /* -0x1.2878e662f31bcp-12 */
    1, /* -0x1.83d790f27f3e5p-14 */
    1, /* -0x1.dd61dedc265f5p-15 */
    1, /* -0x1.29320aa46f06ep-15 */
    -1, /* -0x1.99bc3ca2a27p-17 */
    -1, /* -0x1.8ec2e3d383d64p-19 */
    -1, /* -0x1.7679a41f39c67p-19 */
    1, /* -0x1.427b32daed80dp-20 */
    1, /* -0x1.2a4da90d1ca6p-21 */
    -1, /* -0x1.c0149992fc291p-22 */
    -1, /* -0x1.783beb73b1516p-23 */
    -1, /* -0x1.5b9604ff5f529p-23 */
    -1, /* -0x1.3a21ab6af61a5p-23 */
    1, /* -0x1.71732d4eb3deap-25 */
    -1, /* -0x1.c68b45cd67cb4p-28 */
    1, /* -0x1.dbbf2a5ec6647p-29 */
    -1, /* -0x1.d0e597b6ba3b6p-30 */
    1, /* -0x1.10c87354e03c8p-31 */
    1, /* -0x1.c277920f3fbadp-32 */
    1, /* -0x1.74c46decf3374p-32 */
    -1, /* -0x1.65a1dd290660fp-36 */
    -1, /* -0x1.635e3d74befcap-38 */
    1, /* -0x1.635e3d74befcap-39 */
    1, /* -0x1.6c6cbc45dc8dep-40 */
    1, /* -0x1.5a4fbea3a16b6p-40 */
    1, /* -0x1.6c6cbc45dc8dep-41 */
    1, /* -0x1.6c6cbc45dc8dep-42 */
    1, /* -0x1.6c6cbc45dc8dep-43 */
    1, /* -0x1.6c6cbc45dc8dep-44 */
    1, /* -0x1.6c6cbc45dc8dep-45 */
    1, /* -0x1.6c6cbc45dc8dep-46 */
    1, /* -0x1.6c6cbc45dc8dep-47 */
    -1, /* -0x1.921fb54442d18p-51 */
    1, /* -0x1.2d97c7f3321d2p-51 */
    -1, /* -0x1.921fb54442d18p-52 */
    -1, /* -0x1.921fb54442d18p-53 */
    0, /* 0x0p+0 */
    1, /* 0x1.921fb54442d19p-54 */
    1, /* 0x1.3a28c59d5433bp-49 */
    1, /* 0x1.6c6cbc45dc8dep-49 */
    -1, /* 0x1.3a28c59d5433bp-48 */
    -1, /* 0x1.6c6cbc45dc8dep-48 */
    -1, /* 0x1.6c6cbc45dc8dep-47 */
    -1, /* 0x1.6c6cbc45dc8dep-46 */
    1, /* 0x1.21cfda23b228p-45 */
    -1, /* 0x1.6c6cbc45dc8dep-45 */
    -1, /* 0x1.6c6cbc45dc8dep-44 */
    -1, /* 0x1.6c6cbc45dc8dep-43 */
    1, /* 0x1.b4e0b2cec917ep-43 */
    -1, /* 0x1.6c6cbc45dc8dep-42 */
    1, /* 0x1.90a6b78a52d2ep-42 */
    -1, /* 0x1.6c6cbc45dc8dep-41 */
    -1, /* 0x1.6c6cbc45dc8dep-40 */
    1, /* 0x1.67e57cdd4dc54p-39 */
    -1, /* 0x1.67e57cdd4dc54p-38 */
    1, /* 0x1.0dec1da5fa53fp-37 */
    1, /* 0x1.93c05c9ed3cbcp-36 */
    -1, /* 0x1.39c6fd67805a7p-35 */
    -1, /* 0x1.a9adcc7f96cfp-35 */
    -1, /* 0x1.39c6fd67805a7p-34 */
    1, /* 0x1.44bdb557e1dc1p-34 */
    -1, /* 0x1.bf9b3c6059d24p-34 */
    1, /* 0x1.ca91f450bb53ep-34 */
    1, /* 0x1.57bfc3f20ac4fp-32 */
    -1, /* 0x1.74c46decf3374p-32 */
    -1, /* 0x1.10c87354e03c8p-31 */
    -1, /* 0x1.7ff87542efc93p-31 */
    1, /* 0x1.b09687e430521p-29 */
    -1, /* 0x1.dbbf2a5ec6647p-29 */
    1, /* 0x1.cc4070a3023a7p-28 */
    1, /* 0x1.8d69f36b1cce6p-27 */
    -1, /* 0x1.bdbf63f170fep-27 */
    -1, /* 0x1.ac57910d7e867p-26 */
    -1, /* 0x1.71732d4eb3deap-25 */
    1, /* 0x1.24e3786d7459ap-24 */
    1, /* 0x1.94ee8417af86ep-23 */
    -1, /* 0x1.2a4da90d1ca6p-21 */
    -1, /* 0x1.769cbdfa9e1d8p-21 */
    -1, /* 0x1.427b32daed80dp-20 */
    -1, /* 0x1.73e9e2254c98fp-20 */
    1, /* 0x1.8ec2e3d383d64p-19 */
    -1, /* 0x1.2226500110692p-18 */
    -1, /* 0x1.4c2c402a28e67p-18 */
    1, /* 0x1.59abdcf048a9bp-17 */
    1, /* 0x1.5ef5605b74f7p-16 */
    -1, /* 0x1.969df9fda1b9p-16 */
    1, /* 0x1.29320aa46f06ep-15 */
    -1, /* 0x1.6638f8a2185cbp-15 */
    1, /* 0x1.dd61dedc265f5p-15 */
    1, /* 0x1.41c4a3005149fp-14 */
    -1, /* 0x1.17485bab7607bp-13 */
    -1, /* 0x1.a0ea629bbf42p-13 */
    -1, /* 0x1.2878e662f31bcp-12 */
    -1, /* 0x1.cf88e9268bdcfp-12 */
    -1, /* 0x1.0d0c56d741955p-9 */
    1, /* 0x1.f0a606701a3e5p-9 */
    1, /* 0x1.730786d8eea35p-8 */
    1, /* 0x1.bfa7fc1b1e67cp-8 */
    1, /* 0x1.271adcddb2534p-7 */
    -1, /* 0x1.1f6bbb1fac712p-6 */
    -1, /* 0x1.20588554d6e24p-6 */
    -1, /* 0x1.5fa453f6bb40ep-6 */
    -1, /* 0x1.69a68ac73eaa2p-4 */
    -1, /* 0x1.da83e335e379ap-3 */
    1, /* 0x1.f608aa4e62781p-2 */
  };
// end_acospi

/* accurate path, assumes |x| < 1 */
static double
accurate_path (double x)
{
  double absx, y, h, l, u, v;
  union_t w;
  const double *p;
  absx = x > 0 ? x : -x;
  w.x = 1.0 + absx; /* 1 <= w.x <= 2 */
  /* Warning: w.x might be 2 for rounding up or nearest. */
  int i = (w.x == 2.0) ? 127 : (w.i[HIGH] >> 13) & 127;
  if (i < 64) /* |x| < 0.5 */
  {
    // begin_acospi
    // deal with exceptional cases
    int a, b, c;
    for (a = 0, b = EXCEPTIONS; a + 1 != b;)
    {
      c = (a + b) / 2;
      if (exceptions[c][0] <= x)
        a = c;
      else
        b = c;
    }
    if (x == exceptions[a][0])
    {
      double hi = exceptions[a][1];
      int8_t del = (hi > 0) ? exceptions_rnd[a] : -exceptions_rnd[a];
      return hi + hi * 0x1p-54 * (double) del;
    }
    // for |x| <= 0x1.921fb54442d18p-54, acospi(x) rounds to 0.5 to nearest
    if (absx <= 0x1.921fb54442d18p-54)
      return __builtin_fma (x, -0.125, 0.5);
    // end_acospi

    p = T2[i];
    y = absx - p[DEGREE+LARGE+1]; /* exact */
    h = p[DEGREE+LARGE];
    l = 0;
    for (int j = DEGREE - 1; j >= 0; j--)
    {
      if (j >= LARGE) /* use double only */
        h = p[LARGE+j] + h * y, l = 0;
      else /* use double-double */
      {
        /* multiply by y */
        mul2_1 (&u, &v, h, l, y);
        /* add coefficient of degree j */
        /* Warning: for i=0, the coefficients p[2*j] are tiny for j even,
           and fast_two_sum(h,l,a,b) requires |a| >= |b|, or at least
           exp(a) >= exp(b).  */
        if (__builtin_fabs (p[2*j]) >= __builtin_fabs (u))
          fast_two_sum (&h, &l, p[2*j], u);
        else
          fast_two_sum (&h, &l, u, p[2*j]);
        l += p[2*j+1] + v;
      }
    }
    /* acos(x) ~ pi/2 + (h + l) for x > 0, pi/2 - (h + l) for x < 0 */
    if (x < 0)
      h = -h, l = -l;
    fast_two_sum (&u, &v, pi_hi / 2, h);
    v += pi_lo / 2 + l;

    // acospi_begin
    // multiply by 1/pi
    d_mul (&u, &v, u, v, ONE_OVER_PIH, ONE_OVER_PIL);
    // acospi_end

    return u + v;
  }
  else /* 0.5 <= |x| < 1 */
  {
    double h1, l1;

    // exceptional cases
    if (x == 0x1.b32b7ac93ddefp-1)
      return 0x1.69c0e1dfbf177p-3 - 0x1.16beadd718bafp-108;
    if (x == 0x1.e55a7fa9a24c4p-1)
      return 0x1.a67c4d04a9236p-4 - 0x1.edc5fcb35e5e3p-110;

    h1 = 1.0 - absx; /* exact since |x| >= 0.5 */
    h1 = sqrt_dbl_dbl (h1, &l1);
    p = T2[i];
    y = absx - p[DEGREE+LARGE+1]; /* exact */
    h = p[DEGREE+LARGE];
    l = 0;
    for (int j = DEGREE - 1; j >= 0; j--)
    {
      if (j >= LARGE) /* use double only */
        h = p[LARGE+j] + h * y, l = 0;
      else /* use double-double */
      {
        /* multiply by y */
        mul2_1 (&u, &v, h, l, y);
        /* add coefficient of degree j */
        fast_two_sum (&h, &l, p[2*j], u);
        l += p[2*j+1] + v;
      }
    }
    /* acos(x) ~ (h1 + l1) * (h + l) */
    a_mul (&u, &v, h1, h);
    v += l1 * h + h1 * l;
    if (x < 0)
    {
      fast_two_sum (&u, &l, pi_hi, -u);
      v = l + pi_lo - v;
    }

    // acospi_begin
    // multiply by 1/pi
    d_mul (&u, &v, u, v, ONE_OVER_PIH, ONE_OVER_PIL);
    // acospi_end

    return u + v;
  }
}

double
__acospi (double x)
{
  union_t u, v;
  int32_t k;
  u.x = x;
  u.i[HIGH] &= 0x7fffffff; /* set sign bit to 0 */
  double absx = u.x;
  k = u.i[HIGH];
  if (k < 0x3fe80000) { /* |x| < 0.75 */
    // avoid spurious underflow:
    // for |x| <= 0x1.921fb54442d18p-54, acospi(x) rounds to 0.5 to nearest
    if (__builtin_expect (k < 0x3c9921fb, 0))
    {
      if (__builtin_expect (k == 0 && u.i[LOW] == 0, 0)) return 0.5; // x = 0
      // acospi(x) ~ 1/2 - x/pi
      return 0.5 - __builtin_copysign (0x1p-55, x);
    }
    /* approximate acos(x) by p(x-xmid), where [0,0.75) is split
       into 192 sub-intervals */
    v.x = 1.0 + absx; /* 1 <= v.x < 2 */
    /* v.i[HIGH] contains 20 significant bits in its low bits, we shift by 12
       to get the upper 8 (ignoring the implicit leading bit) */
    int i = (v.i[HIGH] >> 12) & 255;
    if (__builtin_expect (i == 192, 0))
      i = 191;
    const double *p = T[i];
    double y = absx - p[7]; /* p[7] = xmid */
    double zh, zl;
    double yy = y * y;
    /* evaluate in parallel p[1] + p[2] * y and p[3] + p[4] * y, and
       p[5] + p[6] * y using Estrin's scheme */
    double p56 = __builtin_fma (p[6], y, p[5]);
    double p34 = __builtin_fma (p[4], y, p[3]);
    zh = __builtin_fma (p56, yy, p34);
    zh = __builtin_fma (zh, y, p[2]);
    fast_two_sum (&zh, &zl, p[1], y * zh);
    double du, dv;
    fast_two_sum (&du, &dv, p[0], zh * y);
    dv = __builtin_fma (zl, y, dv);
    /* Special case for i=0, since we are obliged to use xmid=0 (so that
       x-xmid is exact) thus we can't use Gal's trick.  This costs about
       0.5 cycle in the average time (for both branches).  */
    if (i == 0)
      dv += 0x4.6989e4b05fa3p-56;
    /* acos(x) ~ du + dv for x > 0, pi - (u + v) for x < 0 */
    if (x < 0) /* acos(-x) = pi-acos(x) */
    {
      fast_two_sum (&du, &zl, pi_hi, -du);
      dv = pi_lo + zl - dv;
    }

    // acospi_begin
    /* We multiply the approximation u+v, with maximal error say 2^-e
       by 1/pi. The maximal value of |u+v| is less than 2.42 (for x=-0.75).
       The maximal error is the sum of several terms:
       * 2^-e * (ONE_OVER_PIH + ONE_OVER_PIL) < 2^-e * 2^-1.651
       * (u+v)*|ONE_OVER_PIH+ONE_OVER_PIL-1/pi| < 2.42*2^-109.523 < 2^-108
       * the ignored term v*ONE_OVER_PIL in d_mul. The maximal observed value
         of v is 0x1.06d413839cafcp-51 for x=-0x1.6a01f2fb71p-1 (rndd),
         we conjecture |v| < 2^-50. Then |v*ONE_OVER_PIL| < 2^-105
       * the rounding error in d_mul. The d_mul call decomposes into:
         a_mul (u, lo1, u_in, ONE_OVER_PIH)
         lo2 = __builtin_fma (u_in, ONE_OVER_PIL, lo1)
         v = __builtin_fma (v_in, ONE_OVER_PIH, lo2)
         Since |u| <= acos(-0.75)/pi < 0.8 we have |lo1| <= ulp(0.8) <= 2^-53.
         Then since |u_in| <= 2.42, |lo2| <= |2.42*ONE_OVER_PIL|+2^-53
                                          < 2^-52.485
         Then |v| <= 2^-50+ONE_OVER_PIH*2^-52.485 < 2^-49.920.
         The rounding error is bounded by ulp(lo2)+ulp(v) <= 2^-105+2^-102
         < 2^-101.83.
       The total error is thus bounded by:
       2^-e * 2^-1.651 + 2^-108 + 2^-105 + 2^-101.83 < Err[i]
       */
    d_mul (&du, &dv, du, dv, ONE_OVER_PIH, ONE_OVER_PIL);
    // acospi_end
    
    const double err = Err[i]; // acospi_specific
    double left  = du + (dv - err), right = du + (dv + err);
    if (__builtin_expect (left != right, 0))
      return accurate_path (x); /* hard to round case */
    return left;
  }
  /*--------------------------- 0.75 <= |x| < 1 ---------------------*/
  else
  if (k < 0x3ff00000) { /* |x| < 1 */
    /* approximate acos(x) by sqrt(1-x)*p(x-xmid) where p is a polynomial,
       and [0.75,1) is split into 64 sub-intervals */
    v.x = 1.0 + absx; /* 1 <= v.x <= 2 */
    /* The low 20 bits of v.i[HIGH] are the upper bits (except the
       implicit leading bit) of the significand of 1+|x|.
       Warning: v.x might be 2 for rounding up or nearest. */
    int i = (v.x == 2.0) ? 255 : (v.i[HIGH] & 0xff000) >> 12;
    const double *p = T[i];
    double y = absx - p[6]; /* exact (p[6] = xmid) */
    double h1, l1;
    h1 = 1.0 - absx; /* exact since |x| >= 0.5 */
    h1 = sqrt_dbl_dbl (h1, &l1);
    double zh, zl;
    /* use Estrin's scheme to evaluate p2 + p3*y + p4*y^2 + p5*y^3 */
    double yy = y * y;
    double p45 = __builtin_fma (p[5], y, p[4]);
    double p23 = __builtin_fma (p[3], y, p[2]);
    zh = __builtin_fma (p45, yy, p23);
    zh = __builtin_fma (zh, y, p[1]);
    fast_two_sum (&zh, &zl, p[0], zh * y);
    double l1zh = l1 * zh; /* compute earlier */
    double h1zl = h1 * zl;
    /* acos(x) ~ (h1 + l1) * (zh + zl) */
    double du, dv;
    a_mul (&du, &dv, h1, zh);
    dv += l1zh + h1zl;
    if (x < 0) /* acos(x) = pi - (u+v) */
    {
      fast_two_sum (&du, &zl, pi_hi, -du);
      /* acos(x) = u + zl + pi_lo - v */
      dv = zl + pi_lo - dv;
    }

    // acospi_begin
    /* Similar analysis as above.
       We multiply the approximation u+v, with maximal error 2^-e
       by 1/pi. The maximal value of |u+v| is pi (for x=-1).
       The maximal error is the sum of several terms:
       * 2^-e * (ONE_OVER_PIH + ONE_OVER_PIL) < 2^-e * 2^-1.651
       * (u+v)*|ONE_OVER_PIH+ONE_OVER_PIL-1/pi| < pi*2^-109.523 < 2^-107
       * the ignored term v*ONE_OVER_PIL in d_mul. The maximal observed value
         of v is 0x1.4586d502c6913p-51 for x=-0x1.fffcc87dece8p-1 (rndd),
         we conjecture |v| < 2^-50. Then |v*ONE_OVER_PIL| < 2^-105
       * the rounding error in d_mul. The d_mul call decomposes into:
         a_mul (u, lo1, u_in, ONE_OVER_PIH)
         lo2 = __builtin_fma (u_in, ONE_OVER_PIL, lo1)
         v = __builtin_fma (v_in, ONE_OVER_PIH, lo2)
         Since |u| <= acospi(-1) < 1 we have |lo1| <= ulp(1-) <= 2^-53.
         Then since |u_in| <= pi, |lo2| <= |pi*ONE_OVER_PIL|+2^-53
                                          < 2^-52.361.
         Then |v| <= 2^-50+ONE_OVER_PIH*2^-52.361 < 2^-49.913.
         The rounding error is bounded by ulp(lo2)+ulp(v) <= 2^-105+2^-102
         < 2^-101.83.
       The total error is thus bounded by:
       2^-e * 2^-1.651 + 2^-107 + 2^-105 + 2^-101.83 < Err[i]
       */
    d_mul (&du, &dv, du, dv, ONE_OVER_PIH, ONE_OVER_PIL);
    // acospi_end

    const double err = Err[i]; // acospi_specific
    double left  = du + (dv - err), right = du + (dv + err);
    if (__builtin_expect (left != right, 0))
      return accurate_path (x); /* hard to round case */
    return left;
  }    /*   else  if (k < 0x3ff00000)    */

  /*---------------------------- |x|>=1 -----------------------*/
  else
    if (k==0x3ff00000 && u.i[LOW]==0) return (x>0) ? 0 : 1; // acospi_specific
  else
    if (k > 0x7ff00000 || (k == 0x7ff00000 && u.i[LOW] != 0))
      return x + x; // case x=nan
  else { // case x=Inf or x > 1
    u.i[HIGH]=0x7ff00000;
    v.i[HIGH]=0x7ff00000;
    u.i[LOW]=0;
    v.i[LOW]=0;
#ifdef CORE_MATH_SUPPORT_ERRNO
    errno = EDOM;
#endif
    return u.x/v.x;
  }
}
libm_alias_double (__acospi, acospi)
