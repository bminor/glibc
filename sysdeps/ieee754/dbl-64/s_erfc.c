/* Correctly-rounded complementary error function for the binary64 format

Copyright (c) 2022-2025 Alexei Sibidanov, Paul Zimmermann, Tom Hubrecht and
Claude-Pierre Jeannerod.

The original version of this file was copied from the CORE-MATH
project (file src/binary64/erfc/erfc.c, revision 55e9869e).

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

/* References:
   [1] The Mathematical Function Computation Handbook, Nelson H.F. Beebe,
       Springer, 2017.
   [2] Handbook of Mathematical Functions, Abramowitz, M., and Stegun, I.,
       Dover, 1973.
   [3] The functions erf and erfc computed with arbitrary precision and
       explicit error bounds, Sylvain Chevillard, Research Report, 2009,
       https://inria.hal.science/ensl-00356709v3.
   [4]
   https://stackoverflow.com/questions/39777360/accurate-computation-of-scaled-complementary-error-function-erfcx
   [5] Chebyshev Approximation of (1 + 2 x) exp(x^2) erfc x in 0 ≤ x < Inf,
       M. M. Shepherd and J. G. Laframboise, Mathematics of Computation,
       Volume 36, No. 153, January 1981, pp. 249-253
   [6] Modern Computer Arithmetic, Richard Brent and Paul Zimmermann,
       Cambridge University Press, 2011.
*/

#include <math.h>
#include <stdint.h>
#include <errno.h>
#include <libm-alias-double.h>
#include "math_config.h"

static const double C[94][13] = {
  { 0x1.b0081148a873ap-4, -0x1.f0295f16ba5d8p-61, 0x1.1e565bca400d4p+0,
    -0x1.62d0ac26c78d3p-54, -0x1.ad8189af6013dp-4, -0x1.7712743c42914p-2,
    0x1.aafd4760d7634p-5, 0x1.ba14988b4127ep-4, -0x1.1afcdb244078ap-6,
    -0x1.9d72ee25cf211p-6, 0x1.19502f7beca8fp-8, 0x1.3b955bfd46624p-8,
    -0x1.a4e2d4d32228bp-11 }, /* i=1 69.005 */
  { 0x1.662a0bdf7a89fp-3, -0x1.ef7bc5856c2d4p-59, 0x1.19e5e92b964abp+0,
    0x1.cca4dec08a64p-57, -0x1.605f63767bdd6p-3, -0x1.6582e9b69c9a9p-2,
    0x1.5aa32b580ec64p-4, 0x1.97594c2593d3ep-4, -0x1.c69c62749fb7fp-6,
    -0x1.6fa7f611aacdcp-6, 0x1.bf1e628a4606ep-8, 0x1.0e50e4329e8a9p-8,
    -0x1.68ca9c1954b4cp-10 }, /* i=2 70.057 */
  { 0x1.f190aa85540e2p-3, -0x1.e522ac9f718e6p-57, 0x1.135e3075d076bp+0,
    -0x1.e2d8ed30e4a48p-57, -0x1.e1e4d4ce2ccfbp-3, -0x1.4c04e66e0d59bp-2,
    0x1.d2855d59988e8p-4, 0x1.659a35f29781ap-4, -0x1.2cf6266a634c8p-5,
    -0x1.2ef4180b1f3fap-6, 0x1.23199a6da60e3p-7, 0x1.9e80d13a3368cp-9,
    -0x1.ba4e4eff641ddp-10 }, /* i=3 70.889 */
  { 0x1.3c9aa8b84bedap-2, 0x1.38ec27d3e582p-58, 0x1.0ae54fa490723p+0,
    -0x1.d016b7bc67433p-54, -0x1.2c41f99922807p-2, -0x1.2b900b640a201p-2,
    0x1.1c6c7eef8fa14p-3, 0x1.277ad7822021ep-4, -0x1.66c9b2023b9dfp-5,
    -0x1.bf7e7b4e8559ep-7, 0x1.53005de4b5751p-7, 0x1.0737c6ba405fp-9,
    -0x1.06ccc916b15dcp-9 }, /* i=4 70.343 */
  { 0x1.7e15944d9d3e4p-2, -0x1.95f819cf77862p-57, 0x1.00abcf3e187a9p+0,
    0x1.5860d868dc542p-55, -0x1.60ec3cf561a89p-2, -0x1.05599bafe4eccp-2,
    0x1.451ef6280e70fp-3, 0x1.c06c6e434be6fp-5, -0x1.8e2d73679096fp-5,
    -0x1.0ea4a60550d9cp-7, 0x1.6c911882cc99cp-7, 0x1.8c65a9990353bp-11,
    -0x1.1e8a88301a7b5p-9 }, /* i=5 69.074 */
  { 0x1.bccfec24855b8p-2, -0x1.472ab1c2b898cp-56, 0x1.e9d5a8e4c934ep-1,
    -0x1.9a002a2814a72p-56, -0x1.8dfd9939e37afp-2, -0x1.b588d8dc5bb96p-3,
    0x1.62338788aee97p-3, 0x1.26cf85bc6dff9p-5, -0x1.a1bcaa91da902p-5,
    -0x1.5b4a7d42d0f64p-9, 0x1.6edef7de2b68dp-7, -0x1.037b458e2da8cp-11,
    -0x1.e8d6001a54334p-10 }, /* i=6 70.183 */
  { 0x1.f86faa9428f9dp-2, 0x1.9996c0c376e32p-56, 0x1.cfc41e36c7df9p-1,
    -0x1.9be994724ea34p-56, -0x1.b2c7dc535b619p-2, -0x1.5a9de93f9c0d5p-3,
    0x1.7317958d24aaep-3, 0x1.133e02ab7d777p-6, -0x1.a155bbde32db8p-5,
    0x1.72049c0cc8525p-9, 0x1.5adde5c722d85p-7, -0x1.b0a7ec5dc80fcp-10,
    -0x1.aa9393b806535p-10 }, /* i=7 70.135 */
  { 0x1.1855a5fd3dd5p-1, 0x1.8f6964e67d61ap-55, 0x1.b3aafcc27502ep-1,
    -0x1.a9dd26edea8a2p-56, -0x1.cee5ac8e9c531p-2, -0x1.fa02983c853d1p-4,
    0x1.77cd75ec731p-3, -0x1.fa6f82f9333b7p-10, -0x1.8e0db5528e559p-5,
    0x1.00bf7062212bcp-7, 0x1.3319e670adc9fp-7, -0x1.58833e091aa36p-9,
    -0x1.8f99b6e81e8f5p-10 }, /* i=8 69.816 */
  { 0x1.32a54cb8db67bp-1, -0x1.96221f7e18978p-57, 0x1.96164fafd8de3p-1,
    0x1.0887f82841accp-56, -0x1.e23a7ea0d187ep-2, -0x1.3f5ee1564be49p-4,
    0x1.70e469de06907p-3, -0x1.3da6878ae6fd8p-6, -0x1.6a0d076468415p-5,
    0x1.8cf081f1fc304p-7, 0x1.f6d62866525e6p-8, -0x1.b93149d5701a4p-9,
    -0x1.1a6c1a9f7ea73p-10 }, /* i=9 70.229 */
  { 0x1.4b13713ad3513p-1, 0x1.e944ee1b212e4p-57, 0x1.7791b886e7403p-1,
    -0x1.da43cb53d911cp-57, -0x1.ecef42310f844p-2, -0x1.15c3c5ce705dfp-5,
    0x1.5f6890affa468p-3, -0x1.1da642fabd4dap-5, -0x1.385991202c7ebp-5,
    0x1.fa4f37fc7c6d4p-7, 0x1.7156b4e430998p-8, -0x1.f546a4377d648p-9,
    -0x1.32e4e5abb1e1ap-11 }, /* i=10 70.743 */
  { 0x1.61955607dd15dp-1, 0x1.98ff39319ab83p-55, 0x1.58a445da7c74cp-1,
    0x1.08ec8e156809bp-55, -0x1.ef6c246a12e7ep-2, 0x1.e83e0da03048p-8,
    0x1.44cc65df8bfc7p-3, -0x1.87d3c8dd62c82p-5, -0x1.f9271a8a1d4e2p-6,
    0x1.225234c1c0a0ep-6, 0x1.c0b0e055a0c48p-9, -0x1.0585251f84919p-8,
    -0x1.85bfb02436e0fp-13 }, /* i=11 70.022 */
  { 0x1.762870f720c6fp-1, 0x1.118b1ba6da9a7p-55, 0x1.39ccc1b136d5ap-1,
    0x1.faa9371c0dd8p-58, -0x1.ea4feea4e5addp-2, 0x1.715e595343353p-5,
    0x1.22cdbdb4cdd0cp-3, -0x1.da50ae547e69ep-5, -0x1.75578f87f217dp-6,
    0x1.353319c65f251p-6, 0x1.39db53a2d03d5p-10, -0x1.fc0364ce1787p-9,
    0x1.272bc18b0f2cep-12 }, /* i=12 70.545 */
  { 0x1.88d1cd474a2ep-1, 0x1.6f571ada77d52p-55, 0x1.1b7e98fe26217p-1,
    0x1.952bd607eb12ep-56, -0x1.de65a22ce0587p-2, 0x1.40686a3f3dc2bp-4,
    0x1.f6b0cb6926c42p-4, -0x1.09c7caecd317dp-4, -0x1.da668f759eaeap-7,
    0x1.364e72035e80ap-6, -0x1.d421975736447p-11, -0x1.cc98454e96141p-9,
    0x1.a8860fdf17259p-11 }, /* i=13 71.537 */
  { 0x1.999d4192a5715p-1, -0x1.c888a5759a92cp-55, 0x1.fc3ee5d1524bp-2,
    -0x1.27e60faac0278p-58, -0x1.cc990045b293fp-2, 0x1.b37338e6ac814p-4,
    0x1.a0d11fe9ba61ap-4, -0x1.19bb2ca3816bap-4, -0x1.a0b7d94791f03p-8,
    0x1.274a59774d5e6p-6, -0x1.64adea7b36f57p-9, -0x1.83684bd8ef173p-9,
    0x1.38905afd229ffp-10 }, /* i=14 70.033 */
  { 0x1.a89c850b7d54dp-1, -0x1.e2752ebf0cd02p-55, 0x1.c40b0729ed548p-2,
    -0x1.c4c1c4927306dp-56, -0x1.b5eaaef09de9dp-2, 0x1.0847c7dad86afp-3,
    0x1.47de0a4f796cap-4, -0x1.1d9de8b54a3ecp-4, 0x1.33252fb810c7cp-10,
    0x1.0ab3e329ded2fp-6, -0x1.12d82076274edp-8, -0x1.287bb4a78d728p-9,
    0x1.57d31bd574dap-10 }, /* i=15 70.519 */
  { 0x1.b5e62fce16095p-1, 0x1.bc3cff4400364p-56, 0x1.8eed36b886d93p-2,
    0x1.ea7e17b96436dp-56, -0x1.9b64a06e4b1p-2, 0x1.2bb6e2c74d4fep-3,
    0x1.dee322c062364p-5, -0x1.169960d5a983dp-4, 0x1.feab4ad0bfc14p-8,
    0x1.c76eb94b07a5fp-7, -0x1.584474ae8f994p-8, -0x1.88df75be9251fp-10,
    0x1.4edef5031709p-10 }, /* i=16 72.437 */
  { 0x1.c194b1d49a184p-1, -0x1.6770a58b27668p-57, 0x1.5d4fd33729015p-2,
    -0x1.6db7d76e9e97bp-56, -0x1.7e0f4f0454d97p-2, 0x1.444bc66c35bc4p-3,
    0x1.356dbb543255p-5, -0x1.0643de6e8c574p-4, 0x1.b2e1f789415e4p-7,
    0x1.6ba6d9f4af32fp-7, -0x1.8138bf4573a6ap-8, -0x1.7e6e52a583322p-11,
    0x1.0f87322fa18a3p-10 }, /* i=17 70.211 */
  { 0x1.cbc54b476248dp-1, 0x1.1a5083b01ec0dp-55, 0x1.2f7cc3fe6f423p-2,
    0x1.9fbb4b774e85dp-56, -0x1.5ee8429e30a49p-2, 0x1.52a8395f9627p-3,
    0x1.313759f199499p-6, -0x1.dcf844d90282cp-5, 0x1.1e45f25ab54a1p-6,
    0x1.091cb68a58665p-7, -0x1.8ea40b0ac8b7bp-8, -0x1.6b91b1bf985f2p-17,
    0x1.158d9c0e1c327p-10 }, /* i=18 73.007 */
  { 0x1.d4970f9ce00d9p-1, -0x1.56704209fca7p-56, 0x1.059f59af7a906p-2,
    -0x1.0ce27da57f153p-56, -0x1.3eda354ddd5ffp-2, 0x1.57b85ad436067p-3,
    0x1.8e90c2a157e8dp-10, -0x1.a2893b28f4033p-5, 0x1.4d6af4484a1cbp-6,
    0x1.4ccee8c8b1f57p-8, -0x1.83304b9e2e312p-8, 0x1.40cb679d0a832p-11,
    0x1.d6b5f4bdef24bp-11 }, /* i=19 75.628 */
  { 0x1.dc29fb60715afp-1, 0x1.ab029f047a087p-55, 0x1.bf8e1b1ca2279p-3,
    0x1.0426e10a38p-65, -0x1.1eb7095e57e16p-2, 0x1.549ea6f7a013fp-3,
    -0x1.b10f20d110552p-7, -0x1.61420b5b34a55p-5, 0x1.677b7ea46c6f2p-6,
    0x1.24f9940ffd84p-9, -0x1.6304445e5f6cap-8, 0x1.222fabfa75bbp-10,
    0x1.fdcf55be3c03ep-12 }, /* i=20 70.096 */
  { 0x1.e29e22a89d766p-1, 0x1.bcc9d569ed217p-55, 0x1.7bd5c7df3fe9cp-3,
    0x1.488f3b06e1394p-57, -0x1.fe674493fde22p-3, 0x1.4a9feacf7e222p-3,
    -0x1.a0082c90a1b0dp-6, -0x1.1cf0e7655f99ap-5, 0x1.6e3396f04262p-6,
    -0x1.3a2d2cdd5650dp-12, -0x1.334add14b9a31p-8, 0x1.7e12864580191p-10,
    0x1.dae75c3e2be46p-12 }, /* i=21 74.177 */
  { 0x1.e812fc64db369p-1, 0x1.3c66a6a23d9a5p-55, 0x1.3fda6bc016994p-3,
    0x1.586ddaff31a18p-57, -0x1.c1cb27861fc79p-3, 0x1.3b1051230b982p-3,
    -0x1.1e645a2a638ffp-5, -0x1.b1f643b14fd89p-6, 0x1.64297d7a66c2p-6,
    -0x1.3e365adfbccaep-9, -0x1.f2aa2b3ef5ec2p-9, 0x1.b3339ee2c8c49p-10,
    0x1.0ef571022311p-13 }, /* i=22 71.398 */
  { 0x1.eca6ccd709544p-1, 0x1.f3de8f195347p-57, 0x1.0b3f52ce8c383p-3,
    0x1.d1234b508bcfbp-57, -0x1.8885019f5df29p-3, 0x1.274275fc87eaep-3,
    -0x1.57f7386bfd263p-5, -0x1.30769f45aaa8bp-6, 0x1.4c8231709cfeep-6,
    -0x1.0c2c99c75913fp-8, -0x1.7514483efc09p-9, 0x1.c3ebcf121a533p-10,
    0x1.de2f1801b848p-17 }, /* i=23 73.855 */
  { 0x1.f0762fde45ee6p-1, 0x1.9c3612a14fb77p-55, 0x1.bb1c972f23e5p-4,
    0x1.ba69c564971e1p-58, -0x1.5341e3c0177b6p-3, 0x1.107929f6e7528p-3,
    -0x1.7e1b362eacfe6p-5, -0x1.73b61e487b8a9p-7, 0x1.2aa763e0343a9p-6,
    -0x1.59a388fd2272dp-8, -0x1.eea3c7f50e8dep-10, 0x1.b5026fd87d0cap-10,
    -0x1.0f2c660125dc6p-12 }, /* i=24 71.363 */
  { 0x1.f39bc242e43e6p-1, -0x1.dbae0fd9b967dp-55, 0x1.6c7e64e7281cbp-4,
    0x1.aa87392dc4c2p-58, -0x1.2274b86833f6ep-3, 0x1.efb890e5b6633p-4,
    -0x1.92c7dbb880b5cp-5, -0x1.4547708842f2bp-8, 0x1.02047ab6c08c4p-6,
    -0x1.888355239e9ecp-8, -0x1.0313bb85e86e1p-10, 0x1.8ced9ddf3d834p-10,
    -0x1.2d520499bd799p-12 }, /* i=25 73.472 */
  { 0x1.f62fe80272419p-1, -0x1.b7c2d17fc31d3p-55, 0x1.297db960e4f63p-4,
    -0x1.22bea9385fad9p-58, -0x1.ecb83b087b37bp-4, 0x1.bce18363bbbb9p-4,
    -0x1.985aaf97891cbp-5, 0x1.cd95f2aa8601ap-12, 0x1.ab9d43270d20fp-7,
    -0x1.9b93410d46789p-8, -0x1.9b530b472cadfp-13, 0x1.52f54de527458p-10,
    -0x1.6844d43c7d693p-12 }, /* i=26 72.129 */
  { 0x1.f848acb544e95p-1, -0x1.b27aa2c376c3cp-55, 0x1.e1d4cf1e2450ap-5,
    -0x1.783e14555c1e9p-59, -0x1.9e12e1fde7354p-4, 0x1.8a27806de834fp-4,
    -0x1.91674e13a339ap-5, 0x1.3bc75e8f9d448p-8, 0x1.51b4d09ac47b8p-7,
    -0x1.96dc7b5f9bd66p-8, 0x1.e16520532bde9p-12, 0x1.0e742b323f434p-10,
    -0x1.ac319bfed91d4p-12 }, /* i=27 72.979 */
  { 0x1.f9f9ba8d3c733p-1, 0x1.cd5790ff03ab3p-55, 0x1.83298d717210ep-5,
    0x1.740e2b04276bfp-59, -0x1.58d101f909971p-4, 0x1.58f1456f7db5ep-4,
    -0x1.808d17b33b814p-5, 0x1.0c1bdce673b1p-7, 0x1.f5ff1c06e9df2p-8,
    -0x1.7f26b8865f398p-8, 0x1.f87060e6f646p-11, 0x1.8c6056bea9223p-11,
    -0x1.e3499a90b84f5p-12 }, /* i=28 73.403 */
  { 0x1.fb54641aebbc9p-1, -0x1.79975513f67e7p-55, 0x1.34ac36ad8dafep-5,
    0x1.902fb5363d36p-63, -0x1.1c8ec267fe9e2p-4, 0x1.2a52c5d83c05p-4,
    -0x1.68541b2c0582cp-5, 0x1.5afe422155ad5p-7, 0x1.56303c111cd8ap-8,
    -0x1.597ead749c06ap-8, 0x1.57b0870a7b4cfp-10, 0x1.ffc0efb0ac024p-12,
    -0x1.9e3ea349ab39ep-12 }, /* i=29 73.624 */
  { 0x1.fc67bcf2d7b8fp-1, -0x1.0d2748f976e8cp-55, 0x1.e85c449e377f3p-6,
    -0x1.cb7ccd2616394p-60, -0x1.d177f166cce53p-5, 0x1.fe23b75845cdfp-5,
    -0x1.4b120f9dde895p-5, 0x1.8d9906d138bd5p-7, 0x1.9201b7e469e83p-9,
    -0x1.2aceacb2954fp-8, 0x1.8d4e8140dc518p-10, 0x1.00a33f7e93047p-12,
    -0x1.72b7adfeee575p-12 }, /* i=30 74.601 */
  { 0x1.fd40bd6d7a785p-1, 0x1.60d45e630998fp-55, 0x1.7f5188610ddc8p-6,
    -0x1.60e8565137ecbp-60, -0x1.7954423f89a51p-5, 0x1.af5baae337ae6p-5,
    -0x1.2ad77b77d17dcp-5, 0x1.a7b8c4a8d53fep-7, 0x1.4593adc5d737ap-10,
    -0x1.ef1cf14455c9cp-9, 0x1.a1a04ce289b4bp-10, 0x1.3d14f37840954p-15,
    -0x1.50b861df174eep-12 }, /* i=31 73.251 */
  { 0x1.fdea6e062d0c9p-1, -0x1.64c70f379f67p-56, 0x1.2a875b5ffab56p-6,
    0x1.531231987c3b8p-63, -0x1.2f3178cd7aa03p-5, 0x1.68d1c45b96efep-5,
    -0x1.09648dd332653p-5, 0x1.ad8b148089c02p-7, -0x1.f00fa01e6ca19p-13,
    -0x1.8718785b346p-9, 0x1.9a7b0da775387p-10, -0x1.090258ede6532p-13,
    -0x1.b3980b454d442p-13 }, /* i=32 73.521 */
  { 0x1.fe6e1742f7cf6p-1, -0x1.cebced8a49e04p-55, 0x1.cd5ec93c12432p-7,
    -0x1.bb85326a5eff3p-61, -0x1.e2ff3aaae31e4p-6, 0x1.2aa4e5824252p-5,
    -0x1.d049824fc44dbp-6, 0x1.a34eda0fc336ep-7, -0x1.682d8d1801582p-10,
    -0x1.239bf51e17ea8p-9, 0x1.7e761274bf059p-10, -0x1.01e715d70d49fp-12,
    -0x1.4d89f3d9c30d5p-13 }, /* i=33 76.244 */
  { 0x1.fed37386190fbp-1, 0x1.72b1549ea44eep-55, 0x1.61beae53b72b7p-7,
    0x1.401790f84b248p-64, -0x1.7d6193f2417adp-6, 0x1.e947279e4a43bp-6,
    -0x1.9060301092cdcp-6, 0x1.8d14d4bdaa7f4p-7, -0x1.1f795ac88038p-9,
    -0x1.9222edb6bd145p-10, 0x1.53f95c7b01615p-10, -0x1.529b07d094e1dp-12,
    -0x1.5b533d0382e2p-14 }, /* i=34 74.706 */
  { 0x1.ff20e0a7ba8c2p-1, -0x1.03f86c5a13f78p-57, 0x1.0d1d69569b82dp-7,
    -0x1.a5e866bd1366ep-62, -0x1.2a8ca0dc14852p-6, 0x1.8cc071b719c43p-6,
    -0x1.54a148886e917p-6, 0x1.6e91361df3c9ep-7, -0x1.65c02e0d08291p-9,
    -0x1.e94b0adc3b1cap-11, 0x1.210781b57b089p-10, -0x1.7b88f8c82fbffp-12,
    -0x1.68df27e9a1688p-15 }, /* i=35 74.851 */
  { 0x1.ff5b8fb26f5f6p-1, -0x1.7e917ec20b615p-55, 0x1.9646f35a76624p-8,
    -0x1.f771f32fd191bp-62, -0x1.cf68ed932f081p-7, 0x1.3e8735b5b73b1p-6,
    -0x1.1e1611aabcbeap-6, 0x1.4afd8cd100d7p-7, -0x1.8c72005b1cfcfp-9,
    -0x1.c6a7216b336aap-12, 0x1.d577412afc2e2p-11, -0x1.836a0c0e10a99p-12,
    0x1.a8f39f410252ap-19 }, /* i=36 75.151 */
  { 0x1.ff87b1913e853p-1, -0x1.3ca98afc58454p-56, 0x1.30499b503957fp-8,
    -0x1.d1eabb1c04f5p-64, -0x1.6496420203331p-7, 0x1.fa73d7eb1b70dp-7,
    -0x1.daa3005c2d3fep-7, 0x1.250942c31c3adp-7, -0x1.997578dc240a8p-9,
    -0x1.3904177639e63p-15, 0x1.6a6ed488a1f54p-11, -0x1.71cf0c5789c7dp-12,
    0x1.43cb84231ab1cp-15 }, /* i=37 75.856 */
  { 0x1.ffa89fe5b3625p-1, 0x1.934b2bcb7f9a3p-55, 0x1.c4412bf4b8f0bp-9,
    -0x1.bbcc9dca4ec6p-67, -0x1.100f34713740dp-7, 0x1.8ebda0768e8e6p-7,
    -0x1.850c68e8e5c3cp-7, 0x1.fdac8346071b3p-8, -0x1.929de70d00321p-9,
    0x1.10c7101bc52d8p-12, 0x1.070f7e89ec1e2p-11, -0x1.4e4b3dcf4f08dp-12,
    0x1.f0d43b9869b19p-15 }, /* i=38 75.476 */
  { 0x1.ffc10194fcb64p-1, 0x1.ea14750ac9b59p-55, 0x1.4d78bba8ca5fdp-9,
    0x1.4d9a93566b5b4p-65, -0x1.9ba107a459ce4p-8, 0x1.36f273fbd909bp-7,
    -0x1.3b38708f7bef7p-7, 0x1.b3fdff1de2112p-8, -0x1.7d55d55d262d8p-9,
    0x1.eae5e05e74fccp-12, 0x1.5ebc1e53214a9p-12, -0x1.1fd7c1cd5d63ep-12,
    0x1.49559a04c8568p-14 }, /* i=39 76.483 */
  { 0x1.ffd2eae369a07p-1, -0x1.83b09df7f7db4p-57, 0x1.e7f232d9e263p-10,
    0x1.a26ac725599e5p-64, -0x1.34c7442de142bp-8, 0x1.e066bed09942fp-8,
    -0x1.f914f2c60b9bbp-8, 0x1.6f4662f6be13bp-8, -0x1.5e664591d6604p-9,
    0x1.3a1598d880f36p-11, 0x1.965b2e78a4544p-13, -0x1.d8db42b193729p-13,
    0x1.449172919598ep-14 }, /* i=40 76.603 */
  { 0x1.ffdff92db56e5p-1, -0x1.8aeef4ee0690ap-56, 0x1.6235fbd7a4345p-10,
    -0x1.11380fe434056p-65, -0x1.cb5e029ba8f3dp-9, 0x1.6fa4c7ef470e9p-8,
    -0x1.903a08305eebp-8, 0x1.30f12c83fdb23p-8, -0x1.39d769a774af1p-9,
    0x1.5d79439ceaefdp-11, 0x1.5326883e7dfebp-14, -0x1.7199782285958p-13,
    0x1.47181c8911603p-14 }, /* i=41 77.827 */
  { 0x1.ffe96a78a04a9p-1, -0x1.2816fe4528f9bp-55, 0x1.fe41cd9bb4eeep-11,
    0x1.e3be508cae7ecp-66, -0x1.52d7b2896626ap-9, 0x1.16c192d8803dcp-8,
    -0x1.39bfce9b4ecc2p-8, 0x1.f376a554e5decp-9, -0x1.12e67cb7aa486p-9,
    0x1.66d6e460b1614p-11, -0x1.54f70e4bde32bp-18, -0x1.10e125571fe1ep-13,
    0x1.2842d46eb9f29p-14 }, /* i=42 78.426 */
  { 0x1.fff0312b010b5p-1, 0x1.155dec9cdc96bp-55, 0x1.6caa0d3582fe9p-11,
    -0x1.97d95851163fcp-67, -0x1.efb729f4be121p-10, 0x1.a2da7cec01564p-9,
    -0x1.e6c27ad2b1cep-9, 0x1.93b1f34b17723p-9, -0x1.d8179cd2ad34fp-10,
    0x1.5cf51e0add9bbp-11, -0x1.16d8f4b5119c7p-14, -0x1.768557564f5f5p-14,
    0x1.f4fc9dde73f24p-15 }, /* i=43 78.437 */
  { 0x1.fff50456dab8cp-1, -0x1.a197a986f0dep-58, 0x1.0295ef6591848p-11,
    -0x1.262bd83520706p-66, -0x1.679880e93e5c4p-10, 0x1.37d38e3a705afp-9,
    -0x1.75b371a264745p-9, 0x1.4231c3bfe3e65p-9, -0x1.8e184d4921105p-10,
    0x1.45d5b5a7f77fap-11, -0x1.bf8ece4afedd2p-14, -0x1.ccd677aaa82f7p-15,
    0x1.9e5241d5b6b15p-15 }, /* i=44 80.203 */
  { 0x1.fff86cfd3e657p-1, -0x1.2e06adb26f84ep-56, 0x1.6be02102b352p-12,
    0x1.448bcfd3cfe0cp-68, -0x1.02b15777eb7c5p-10, 0x1.cc1d886874d5bp-10,
    -0x1.1bff70664651dp-9, 0x1.fc0f76c943696p-10, -0x1.4a22286622d3ep-10,
    0x1.268887688a6e6p-11, -0x1.0fa2692fd7da2p-13, -0x1.cc13d1a82f742p-16,
    0x1.4153e6537aae5p-15 }, /* i=45 79.393 */
  { 0x1.fffad0b901755p-1, 0x1.70d5c9a92b65cp-57, 0x1.fc0d55470cf51p-13,
    -0x1.6f2b03553d4c8p-67, -0x1.7121aff59f6a1p-11, 0x1.506d6992fc8ffp-10,
    -0x1.ab596015fc183p-10, 0x1.8bdd79a098723p-10, -0x1.0d88da9deb868p-10,
    0x1.031cdd07e4507p-11, -0x1.22fc41430a37dp-13, -0x1.b5cc9546afcecp-18,
    0x1.d7ea1c7b8fdb6p-16 }, /* i=46 79.358 */
  { 0x1.fffc7a37857d2p-1, -0x1.97b30fd4b6b48p-56, 0x1.5feada379d8b7p-13,
    -0x1.0546c4da57036p-67, -0x1.05304df546ed8p-11, 0x1.e79c081b79ebcp-11,
    -0x1.3e5dc1062db15p-10, 0x1.30eb20ccc1f98p-10, -0x1.b1b06c20a060dp-11,
    0x1.bd52fbd55e0efp-12, -0x1.214afb8835b23p-13, 0x1.19ae9d16650ap-17,
    0x1.42d933ee154fdp-16 }, /* i=47 79.5 */
  { 0x1.fffd9fdeabccep-1, 0x1.0c43c3bc59762p-55, 0x1.e3bcf436a1a95p-14,
    -0x1.6458a28a3f9b6p-69, -0x1.6e95311166825p-12, 0x1.5e3edf674e2dbp-11,
    -0x1.d5be6d15abe3ap-11, 0x1.d07da13e640c2p-11, -0x1.58106cc648748p-11,
    0x1.76c840985e5ebp-12, -0x1.111de112b1a2ep-13, 0x1.315fc34053fbdp-16,
    0x1.939439a75a553p-17 }, /* i=48 80.809 */
  { 0x1.fffe68f4fa777p-1, 0x1.2f21786b7644p-60, 0x1.49e17724f4d41p-14,
    0x1.747684f0023e4p-69, -0x1.fe48c44d2ab81p-13, 0x1.f2bd95d72a532p-12,
    -0x1.57389188a71a9p-11, 0x1.5decc4058f7a1p-11, -0x1.0d559cf0f2957p-11,
    0x1.3583904af6f83p-12, -0x1.efd7979333337p-14, 0x1.904cf9fa5c1f6p-16,
    0x1.a13a094bd56a2p-18 }, /* i=49 81.559 */
  { 0x1.fffef1960d85dp-1, -0x1.f7cc78053f6adp-55, 0x1.be6abbb10a5aap-15,
    -0x1.e50b219d40126p-70, -0x1.60403819b22b8p-13, 0x1.5fff1dde5305ep-12,
    -0x1.f0c93c73e7f42p-12, 0x1.04cbf67af6c26p-11, -0x1.a04893510426cp-12,
    0x1.f66b51a7bc4ap-13, -0x1.b410d7f2fd319p-14, 0x1.b99f9eb427956p-16,
    0x1.f26fcffb14441p-20 }, /* i=50 82.307 */
  { 0x1.ffff4db27f146p-1, 0x1.ddecdd5e1d408p-55, 0x1.2bb5cc22e5db6p-15,
    0x1.c5112eca8acdep-70, -0x1.e258948829ed1p-14, 0x1.ec8a8e59d9d5bp-13,
    -0x1.6425722b9f3cdp-12, 0x1.80a83a7103b4bp-12, -0x1.3dbb9374004f9p-12,
    0x1.913b301d37bdep-13, -0x1.7563b0d94459fp-14, 0x1.bc01eea9a10bep-16,
    -0x1.3df26463df6a5p-20 }, /* i=51 82.355 */
  { 0x1.ffff8b500e77cp-1, -0x1.1014e1f83ed4cp-56, 0x1.8f4ccca7fc90dp-16,
    0x1.a5d4ec8b9de43p-70, -0x1.478cffe1cd2edp-14, 0x1.559f04ad4de62p-13,
    -0x1.f9e163b15c466p-13, 0x1.18bda8b8c1315p-12, -0x1.df381bd3c058ep-13,
    0x1.3b94f531bb6bep-13, -0x1.385f32481ed94p-14, 0x1.a414bd2b7cb3cp-16,
    -0x1.ac2bbe30f8767p-19 }, /* i=52 83.754 */
  { 0x1.ffffb43555b5fp-1, 0x1.c17f83b8d73a2p-55, 0x1.07ebd2a2d2844p-16,
    0x1.d1bbdc704f49bp-70, -0x1.b93e442837f52p-15, 0x1.d5cf1514977f3p-14,
    -0x1.63f5eb46877fdp-13, 0x1.95a0411e668b1p-13, -0x1.652e5f2a88269p-13,
    0x1.e950ddb7f5444p-14, -0x1.ffeb9383bdb3dp-15, 0x1.7c24392346fddp-16,
    -0x1.1f3b3254d723p-18 }, /* i=53 83.181 */
  { 0x1.ffffcf23ff5fcp-1, -0x1.b18a8b25039c4p-55, 0x1.5a2adfa0b4bc4p-17,
    0x1.eb6d61aaaf95cp-71, -0x1.26c8826ed9e85p-15, 0x1.40473571d5383p-14,
    -0x1.f057dbf365c0ap-14, 0x1.2217929fed933p-13, -0x1.07324014ddb42p-13,
    0x1.762758a56d654p-14, -0x1.9ba250c662e9p-15, 0x1.4c25759179e3dp-16,
    -0x1.3e800358f1a7bp-18 }, /* i=54 83.812 */
  { 0x1.ffffe0bd3e852p-1, -0x1.d7ece4ab5315p-58, 0x1.c282cd3957edap-18,
    0x1.eb3cf4fd1428p-73, -0x1.86ad6df7ba401p-16, 0x1.b0f313eeb65a6p-15,
    -0x1.56e457745d637p-14, 0x1.9ad1f65a78253p-14, -0x1.7f92ad8542929p-14,
    0x1.1a5578c0d30b3p-14, -0x1.4548d876bb0a3p-15, 0x1.19e60bf53b25ap-16,
    -0x1.3f1745170e2d3p-18 }, /* i=55 85.046 */
  { 0x1.ffffec2641a9ep-1, -0x1.e7ba4fdaaa8c8p-55, 0x1.22df298214423p-18,
    -0x1.a9d49552152a4p-74, -0x1.00c902a4d5e27p-16, 0x1.22234eb745941p-15,
    -0x1.d57a2be01db67p-15, 0x1.200c2ffad65f1p-14, -0x1.147585d43f49ap-14,
    0x1.a4b07aec797e9p-15, -0x1.f9d088bbeff64p-16, 0x1.d2b2be4e42422p-17,
    -0x1.2bb57c0cf2941p-18 }, /* i=56 84.87 */
  { 0x1.fffff37d63a36p-1, -0x1.753e3241c01bp-57, 0x1.74adc8f4064d3p-19,
    0x1.de8a904d5c372p-73, -0x1.4ed4228b3da96p-17, 0x1.81918baca1979p-16,
    -0x1.3e81c09c29601p-15, 0x1.9004afed1bde9p-15, -0x1.8a40e183ee3fcp-15,
    0x1.359242a8b8c58p-15, -0x1.834b953bcb845p-16, 0x1.79e345fb0b20dp-17,
    -0x1.0bb2d323900cep-18 }, /* i=57 85.252 */
  { 0x1.fffff82cdcf1bp-1, 0x1.046bbe9897fd5p-55, 0x1.d9c73698fb1dcp-20,
    0x1.88de36481dfb5p-74, -0x1.b11017e7d5893p-18, 0x1.fc0dfadc2c6d6p-17,
    -0x1.ac4e1aa499ac6p-16, 0x1.131810ab2e2e3p-15, -0x1.1629d94abc864p-15,
    0x1.c22a71036c259p-16, -0x1.244452f74de31p-16, 0x1.2bf17664310c1p-17,
    -0x1.cd1b31a8349bep-19 }, /* i=58 84.869 */
  { 0x1.fffffb248c39dp-1, 0x1.9b9a41713558cp-55, 0x1.2acee2f5ecdb8p-20,
    -0x1.2d1692a9a105cp-76, -0x1.15cc5700a2341p-18, 0x1.4be757b934819p-17,
    -0x1.1d6ab6f8cbf7cp-16, 0x1.76c5a3035bdabp-16, -0x1.847332578dfacp-16,
    0x1.437f23f8d25ffp-16, -0x1.b305e625a092dp-17, 0x1.d3886ff986fefp-18,
    -0x1.81f2189b385a2p-19 }, /* i=59 85.208 */
  { 0x1.fffffd01f36afp-1, -0x1.d41915db812efp-55, 0x1.75fa8dbc84becp-21,
    0x1.a5cd79572a1a6p-76, -0x1.6186d9fc357c5p-19, 0x1.ae02322e08822p-18,
    -0x1.79082befd50cap-17, 0x1.f9c26e211b174p-17, -0x1.0c768235c378bp-16,
    0x1.cba7164e1064fp-17, -0x1.3f75c28c31ac8p-17, 0x1.663fcfff77e44p-18,
    -0x1.3a6da35f36ee6p-19 }, /* i=60 85.253 */
  { 0x1.fffffe2ba0ea5p-1, -0x1.26cd7908cba2bp-55, 0x1.d06ad6ecdf971p-22,
    -0x1.020b74d9d30fbp-76, -0x1.be46aa879edb2p-20, 0x1.143860c49d129p-18,
    -0x1.edabcbc3e620dp-18, 0x1.52139c87e9c82p-17, -0x1.6f567cd982028p-17,
    0x1.42ebd266abd62p-17, -0x1.cf2f0c6adfb3ep-18, 0x1.0e2c0ed67786cp-18,
    -0x1.f50cb81b9b19p-20 }, /* i=61 85.484 */
  { 0x1.fffffee3cc32cp-1, 0x1.e429188c949b8p-56, 0x1.1e1e857adc568p-22,
    0x1.2439f8a1649bbp-76, -0x1.1769ce59fb2c8p-20, 0x1.5fe5d47560794p-19,
    -0x1.405da04875e51p-18, 0x1.bfc96a938083dp-18, -0x1.f19ff5e59cbe9p-18,
    0x1.c0c4d50d275bfp-18, -0x1.4b9df120462aep-18, 0x1.916640ee35de4p-19,
    -0x1.874483d99c37ep-20 }, /* i=62 85.73 */
  { 0x1.ffffff54dab72p-1, -0x1.a443df643729ap-55, 0x1.5dcd669f2cd34p-23,
    -0x1.ceb1ec59e0c28p-78, -0x1.5b11cbd1ee799p-21, 0x1.bc91a6b1c1839p-20,
    -0x1.9c2c5d12dfa2cp-19, 0x1.25d1e3c70364fp-18, -0x1.4dbe26c88e4f7p-18,
    0x1.347bb8350b422p-18, -0x1.d51d3280da8ap-19, 0x1.25ed8e5b466b5p-19,
    -0x1.2b9c5d3390919p-20 }, /* i=63 86.036 */
  { 0x1.ffffff99b79d2p-1, -0x1.58ff1c425f8dep-56, 0x1.a854ea14102a9p-24,
    -0x1.21745e4b4fcb3p-78, -0x1.aba593e8384aep-22, 0x1.167c252a45678p-20,
    -0x1.06d78ca0424a3p-19, 0x1.7e0f59fcfa53dp-19, -0x1.bb4d48383b847p-19,
    0x1.a39f3ad9a397fp-19, -0x1.47e836879c374p-19, 0x1.a89244d14b829p-20,
    -0x1.c33e15a6dbe37p-21 }, /* i=64 86.403 */
  { 0x1.ffffffc355dfdp-1, 0x1.88cb60fd4511cp-57, 0x1.febc107d5efabp-25,
    -0x1.d9ed10902067cp-81, -0x1.055a3c70279a4p-22, 0x1.59ff37766e9a7p-21,
    -0x1.4c53adb9dcc4dp-20, 0x1.ec49242997849p-20, -0x1.23927ad6ac54fp-19,
    0x1.1a6e0676c7463p-19, -0x1.c5239f6a88a96p-20, 0x1.2e991308bf6fap-20,
    -0x1.4e276c09fe81bp-21 }, /* i=65 86.718 */
  { 0x1.ffffffdc4ad7ap-1, -0x1.d75de787812d4p-55, 0x1.30f93c3699079p-25,
    -0x1.8f941ab38e9dap-80, -0x1.3ce2f890bb01dp-23, 0x1.aa5010863c83bp-22,
    -0x1.a08ef1ca1636p-21, 0x1.3a4a6af3cafacp-20, -0x1.7be1e832218fp-20,
    0x1.784775c30c386p-20, -0x1.3593046482ce3p-20, 0x1.a9d448178fbfdp-21,
    -0x1.e77bb85451c65p-22 }, /* i=66 87.121 */
  { 0x1.ffffffeb24467p-1, 0x1.bff89ef33d6ddp-55, 0x1.6961b8d641d07p-26,
    -0x1.74a7fc97b1544p-80, -0x1.7d2510f1f969dp-24, 0x1.0476b165ac852p-22,
    -0x1.02d3a3b9d195ep-21, 0x1.8db3567bef1dfp-21, -0x1.ea3ef4e3a126bp-21,
    0x1.f03b0861a59acp-21, -0x1.a250ca467705ap-21, 0x1.27e9995f6dfcdp-21,
    -0x1.5e77b673c6d74p-22 }, /* i=67 87.557 */
  { 0x1.fffffff3e8892p-1, 0x1.befbf8d294678p-58, 0x1.a8e405e651ab7p-27,
    0x1.167a2d8cf6b18p-84, -0x1.c6c40e5083698p-25, 0x1.3ba47a17512fdp-23,
    -0x1.3ee334beef6ecp-22, 0x1.f2bf9e6c43e99p-22, -0x1.395c08ac8e281p-21,
    0x1.43ee4b521ccadp-21, -0x1.178f0deeb9b2p-21, 0x1.964e51b0f0532p-22,
    -0x1.f0cc4ecca5c2fp-23 }, /* i=68 87.991 */
  { 0x1.fffffff90b2e3p-1, -0x1.d82d94a90f1e4p-56, 0x1.efac5187b2864p-28,
    0x1.f1301ae680614p-83, -0x1.0d229044adeeep-25, 0x1.7b5bc9db47dp-24,
    -0x1.8588212e670c2p-23, 0x1.35f42db1989fap-22, -0x1.8cd98865c4ffp-22,
    0x1.a2b8587c48078p-22, -0x1.71aa2de99af9cp-22, 0x1.13a89805c15d9p-22,
    -0x1.5b53ca1bcf01ap-23 }, /* i=69 88.463 */
  { 0x1.fffffffc0748fp-1, 0x1.6ef7a9caef28p-57, 0x1.1edfa3c5f5ccbp-28,
    0x1.368f60e2e6cfap-83, -0x1.3c025a6810c37p-26, 0x1.c42f78a0989adp-25,
    -0x1.d7c6c3583c6e3p-24, 0x1.7dd6ccb5c93b4p-23, -0x1.f1ec2f699fdccp-23,
    0x1.0bf7a04407a8cp-22, -0x1.e3aafe6dfd4ep-23, 0x1.71bc3a55b63f4p-23,
    -0x1.df66b11724e7cp-24 }, /* i=70 88.939 */
  { 0x1.fffffffdbff2ap-1, 0x1.49438981099b2p-56, 0x1.4979ac8b28928p-29,
    -0x1.c2f44bcf3ce52p-83, -0x1.7015eec37753ap-27, 0x1.0b487791590cfp-25,
    -0x1.1b44b64c3c995p-24, 0x1.d23ff3ef8dd83p-24, -0x1.357d673d1ccfcp-23,
    0x1.53a563ce0e9e3p-23, -0x1.3921106a960f6p-23, 0x1.ea527d318f96ep-24,
    -0x1.46bd6cea7103dp-24 }, /* i=71 89.458 */
  { 0x1.fffffffebc1a9p-1, 0x1.e0e5facabfab4p-56, 0x1.77756ec9f78fbp-30,
    0x1.e20366d0e0306p-85, -0x1.a9530780ca70cp-28, 0x1.3962ecb10df65p-26,
    -0x1.51494525dee64p-25, 0x1.1a2961b90efbp-24, -0x1.7d35cd0b404bfp-24,
    0x1.aa596d9d73afbp-24, -0x1.91493d8d43ba2p-24, 0x1.4184505343c2dp-24,
    -0x1.b7d977f1a3402p-25 }, /* i=72 89.993 */
  { 0x1.ffffffff4b453p-1, 0x1.59b25048a61ccp-55, 0x1.a887bd2b4404fp-31,
    -0x1.2556d8ad4dd44p-87, -0x1.e78be33fb01dap-29, 0x1.6c6ef0b68629ep-27,
    -0x1.8e36e9a44c497p-26, 0x1.5286ee37c531ep-25, -0x1.d146395886537p-25,
    0x1.090902855d5fp-24, -0x1.fd0d1e8fcb6dfp-25, 0x1.a10f65c3c5a7bp-25,
    -0x1.24888c323daf3p-25 }, /* i=73 90.536 */
  { 0x1.ffffffff9bec8p-1, -0x1.6755054654b62p-56, 0x1.dc479de0ef004p-32,
    -0x1.c3434581af3b8p-86, -0x1.1535aee3eb1b2p-29, 0x1.a4547ed264758p-28,
    -0x1.d2308d0dead0fp-27, 0x1.929d46a9a7edcp-26, -0x1.195dbfd4afd19p-25,
    0x1.46630f49ccd2fp-25, -0x1.3fa4637c64ebcp-25, 0x1.0b98a6e0cfc02p-25,
    -0x1.8093f032972f3p-26 }, /* i=74 91.092 */
  { 0x1.ffffffffc901cp-1, 0x1.9c951c943961cp-57, 0x1.0916f04b6e18dp-32,
    0x1.1bdf9650721eap-87, -0x1.38b90f78fbe14p-30, 0x1.e0d7765326885p-29,
    -0x1.0e9760d0ac127p-27, 0x1.daad91166722dp-27, -0x1.513c51b9838edp-26,
    0x1.8e27fb85ba534p-26, -0x1.8d6f6bd99eaffp-26, 0x1.53c31e52fff08p-26,
    -0x1.f3bfd31796bcp-27 }, /* i=75 91.685 */
  { 0x1.ffffffffe202dp-1, 0x1.a54841f566a61p-55, 0x1.24caf2c32af16p-33,
    0x1.02e3358112fa1p-87, -0x1.5dfa962d49548p-31, 0x1.10ca1ff2af812p-29,
    -0x1.377c7e98dd9b4p-28, 0x1.156649e0b5dd2p-27, -0x1.9092f4db426c5p-27,
    0x1.e12a29b227972p-27, -0x1.e94e18d5271a9p-27, 0x1.aae38927ee69bp-27,
    -0x1.41121b0293be1p-27 }, /* i=76 92.286 */
  { 0x1.ffffffffefc57p-1, -0x1.8225a9658ef84p-57, 0x1.40dfd87456f4fp-34,
    -0x1.a6d5c55f8e63bp-88, -0x1.848f101ce14c8p-32, 0x1.32fed47f8dd28p-30,
    -0x1.638ff4a6975f2p-29, 0x1.416d25168a6b8p-28, -0x1.d78fb22f58668p-28,
    0x1.2009c6b4e61eap-27, -0x1.2a459e59c850bp-27, 0x1.096a3e8dac0eap-27,
    -0x1.97fba69de37d8p-28 }, /* i=77 92.91 */
  { 0x1.fffffffff748ep-1, 0x1.ae15e36044aacp-57, 0x1.5ce9ab1670dd6p-35,
    0x1.cc9bbfb723fc4p-91, -0x1.abf69bd9866f7p-33, 0x1.56ae1e8abbbbfp-31,
    -0x1.927ca04d1a7a8p-30, 0x1.713d3b07d7a36p-29, -0x1.1318f5d7d717bp-28,
    0x1.55ab94fdfd1f4p-28, -0x1.68216fb90717ap-28, 0x1.46ad5ce577d65p-28,
    -0x1.0065a20073e81p-28 }, /* i=78 93.545 */
  { 0x1.fffffffffb5bp-1, -0x1.50fb19119064fp-55, 0x1.7872d9fa10ab2p-36,
    -0x1.7760afdf543a4p-90, -0x1.d39eaac4a0b47p-34, 0x1.7b67ab8af33d6p-32,
    -0x1.c3ced54e694eap-31, 0x1.a4875d8a47f12p-30, -0x1.3e213e6f5c296p-29,
    0x1.919137301f897p-29, -0x1.aea6bd9b3493p-29, 0x1.8e06e4ab5925fp-29,
    -0x1.3ed1d979421b2p-29 }, /* i=79 94.198 */
  { 0x1.fffffffffd8b3p-1, -0x1.5182469c211ep-57, 0x1.92ff33023d5c3p-37,
    -0x1.2932180032bd1p-91, -0x1.fae4fe28d12ddp-35, 0x1.a0a80964d6e97p-33,
    -0x1.f6f47be478e2ap-32, 0x1.dad968cdacb13p-31, -0x1.6ca68a8bfdb81p-30,
    0x1.d3a79e5305b4ap-30, -0x1.fe1534ebf69c7p-30, 0x1.e01ee76d92779p-30,
    -0x1.883ed9069f3fdp-30 }, /* i=80 94.867 */
  { 0x1.fffffffffeb6p-1, -0x1.4d3f53e684bf8p-56, 0x1.ac0f5f322937ap-38,
    -0x1.8e8ab19224e58p-92, -0x1.108dc99cf03e5p-35, 0x1.c5db17016a0c6p-34,
    -0x1.159f41ea079c3p-32, 0x1.09ced3e9b7204p-31, -0x1.9e4dace0668p-31,
    0x1.0dd5e0e9749b6p-30, -0x1.2b3aa6599d0b5p-30, 0x1.1eb5e8e4ffe8fp-30,
    -0x1.dd8955967ed31p-31 }, /* i=81 95.555 */
  { 0x1.ffffffffff542p-1, 0x1.b57ed63ed811p-57, 0x1.c324c20e337e5p-39,
    0x1.53fd8abf42ed9p-93, -0x1.22c6b11327305p-36, 0x1.ea5f66f89cbd4p-35,
    -0x1.2ff1e0a81bedcp-33, 0x1.270ddbd8e501fp-32, -0x1.d2992b5c25c93p-32,
    0x1.3492d76bdf266p-31, -0x1.5bc7361853ddep-31, 0x1.53121ae3f1d2ep-31,
    -0x1.1fb0f7e3f242bp-31 }, /* i=82 96.256 */
  { 0x1.ffffffffffa73p-1, -0x1.6fead614b7934p-56, 0x1.d7c593130dd16p-40,
    -0x1.8e78574fe0514p-95, -0x1.33c1e2f16e037p-37, 0x1.06c53fdc74764p-35,
    -0x1.4a029a87915acp-34, 0x1.44bd86238ff0dp-33, -0x1.0474ac3a80072p-32,
    0x1.5db2a89e9bc47p-32, -0x1.906f4b51a7f75p-32, 0x1.8d189784c1f5p-32,
    -0x1.571a4760f483dp-32 }, /* i=83 96.973 */
  { 0x1.ffffffffffd27p-1, 0x1.19e1a84064c56p-56, 0x1.e9810295890f9p-41,
    0x1.f998d55766fdbp-95, -0x1.43262ab4b77b2p-38, 0x1.1756eae580a28p-36,
    -0x1.6359d5b0d251ep-35, 0x1.626391bd58994p-34, -0x1.203efc6c9f556p-33,
    0x1.88c0b111be9p-33, -0x1.c8ca211a38811p-33, 0x1.cc911f684d612p-33,
    -0x1.950e3edf09a71p-33 }, /* i=84 97.706 */
  { 0x1.ffffffffffe8dp-1, 0x1.e766e2c801398p-58, 0x1.f7f338086a87bp-42,
    -0x1.dfa0c27b527ep-98, -0x1.509f766d9f287p-39, 0x1.268e278ede221p-37,
    -0x1.7b7b43e9a1b0ep-36, 0x1.7f7aadab6b398p-35, -0x1.3c3cc6aafba0bp-34,
    0x1.b52c69b4ab6dep-34, -0x1.0222c438d1182p-33, 0x1.0888e14314f83p-33,
    -0x1.d96aaea63b362p-34 }, /* i=85 98.453 */
  { 0x1.fffffffffff45p-1, -0x1.5948eec884df5p-55, 0x1.01647ba79874ep-42,
    0x1.6d5d39dabc3p-103, -0x1.5be1cf20840dcp-40, 0x1.3418096320dafp-38,
    -0x1.91e9beb94b447p-37, 0x1.9b762261756a7p-36, -0x1.57f320a630c91p-35,
    0x1.e24b78ce82b11p-35, -0x1.2112fff5c77aap-34, 0x1.2cfdd93a41786p-34,
    -0x1.11ea1f35b4d2bp-34 }, /* i=86 99.218 */
  { 0x1.fffffffffffa2p-1, 0x1.d07509a1a944p-57, 0x1.04e15ecc7f401p-43,
    -0x1.0858e34f7a6a6p-98, -0x1.64ac1f9b95f96p-41, 0x1.3fa8302ade993p-39,
    -0x1.a62b70897719ep-38, 0x1.b5c619266e9fp-37, -0x1.72de32129cbb8p-36,
    0x1.07ae94305c398p-35, -0x1.40c45a9e95152p-35, 0x1.533d127efdf16p-35,
    -0x1.39dc242ba4cdap-35 }, /* i=87 99.994 */
  { 0x1.fffffffffffd1p-1, 0x1.3b6fc0b729759p-55, 0x1.065b9616170e1p-44,
    -0x1.49459f5147526p-99, -0x1.6acaa58a8be12p-42, 0x1.48fb92d0947e7p-40,
    -0x1.b7ce1a1ea8ea5p-39, 0x1.cddc552bbebebp-38, -0x1.8c751cc1a5784p-37,
    0x1.1dc79b52007bp-36, -0x1.60b3d17e7714cp-36, 0x1.7ac1d379afc28p-36,
    -0x1.641ca84798564p-36 }, /* i=88 100.787 */
  { 0x1.fffffffffffe9p-1, -0x1.5fe91226dd51p-58, 0x1.05ca50205d279p-45,
    -0x1.7a281f9edb8e6p-99, -0x1.6e18ec0d42451p-43, 0x1.4fdb051100a15p-41,
    -0x1.c66b3f3fe565ep-40, 0x1.e331281475b54p-39, -0x1.a42e6965b2b9ap-38,
    0x1.3301ef493196p-37, -0x1.804fcc1524d74p-37, 0x1.a2ef0c13a3daap-37,
    -0x1.9028a915f98d3p-37 }, /* i=89 101.591 */
  { 0x1.ffffffffffff5p-1, -0x1.238f8ed17d9b3p-55, 0x1.0330f0fd69931p-46,
    0x1.a2c00e0c6dcbap-100, -0x1.6e8334c65749dp-44, 0x1.541d561058477p-42,
    -0x1.d1ac042ada69ep-41, 0x1.f54864c5a530ep-40, -0x1.b984c73c1d301p-39,
    0x1.46ec7009c291fp-38, -0x1.9efc2df73776p-38, 0x1.cb12ac38f37cap-38,
    -0x1.bd54fcd67b8d4p-38 }, /* i=90 102.407 */
  { 0x1.ffffffffffffbp-1, -0x1.efa4d64f59f62p-55, 0x1.fd3de10d6287ap-48,
    -0x1.e1fdae91c5cfep-102, -0x1.6c073be0916e6p-45, 0x1.55a8eab9e129ap-43,
    -0x1.d94c87c1bc304p-42, 0x1.01db0818bec24p-40, -0x1.cbfbe4c0ef6eep-40,
    0x1.59179d8c519c4p-39, -0x1.bc172710440bdp-39, 0x1.f26a4f726814ep-39,
    -0x1.ead889e052555p-39 }, /* i=91 103.247 */
  { 0x1.ffffffffffffdp-1, 0x1.6be96953fe014p-55, 0x1.f05e82aae2be2p-49,
    -0x1.070a8237b4337p-103, -0x1.66b44c6d7ddb6p-46, 0x1.5474bd9d072f3p-44,
    -0x1.dd1e8c33100ccp-43, 0x1.0711486984913p-41, -0x1.db2522b66a6cep-41,
    0x1.6919a06329739p-40, -0x1.d6fe8f87926e8p-40, 0x1.0c1488010ff5cp-39,
    -0x1.0bf9fa407e9abp-39 }, /* i=92 104.083 */
  { 0x1.fffffffffffffp-1, -0x1.0fecc5ed770dep-55, 0x1.e00e9148a1d52p-50,
    0x1.f7a503c7a2ad8p-107, -0x1.5eaaa4200e355p-47, 0x1.5088b6566fcedp-45,
    -0x1.dd0b48e0f634ep-44, 0x1.0a27116d7478ep-42, -0x1.e6a3e1d5c214fp-42,
    0x1.769249755a4bcp-41, -0x1.ef16049050b69p-41, 0x1.1dbf2744f66dbp-40,
    -0x1.21c636bd8f5a9p-40 }, /* i=93 104.933 */
  { 0x1.fffffffffffffp-1, 0x1.989c6c5d51227p-55, 0x1.ccaaea71ab11p-51,
    0x1.152f323a1f3b4p-107, -0x1.541a2f15eb476p-48, 0x1.49fd53e85cdf3p-46,
    -0x1.d9144beee6b4ap-45, 0x1.0b09b02f533a1p-43, -0x1.ee312fcf48076p-43,
    0x1.812ed2f01f60ap-42, -0x1.01e6391f47ad7p-41, 0x1.2dce8f6b8c896p-41,
    -0x1.365d5011db0dfp-41 }, /* i=94 105.705 */
};

static inline void
fast_two_sum (double *hi, double *lo, double a, double b)
{
  double e;

  *hi = a + b;
  e = *hi - a; /* exact */
  *lo = b - e; /* exact */
}

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
  if (z < 0.0625) /* z < 1/16 */
    {
      static const double c0[]
	  = { 0x1.20dd750429b6dp+0,  0x1.1ae3a7862d9c4p-56, /* degree 1 */
	      -0x1.812746b0379e7p-2, 0x1.f1a64d72722a2p-57, /* degree 3 */
	      0x1.ce2f21a042b7fp-4,			    /* degree 5 */
	      -0x1.b82ce31189904p-6,			    /* degree 7 */
	      0x1.565bbf8a0fe0bp-8,			    /* degree 9 */
	      -0x1.bf9f8d2c202e4p-11 };			    /* degree 11 */
      double z2h, z2l, z4;
      a_mul (&z2h, &z2l, z, z);
      z4 = z2h * z2h;
      double c9 = fma (c0[7], z2h, c0[6]);
      double c5 = fma (c0[5], z2h, c0[4]);
      c5 = fma (c9, z4, c5);
      a_mul (&th, &tl, z2h, c5);
      fast_two_sum (h, l, c0[2], th);
      *l += tl + c0[3];
      double h_copy = *h;
      a_mul (&th, &tl, z2h, *h);
      tl += fma (z2h, *l, c0[1]);
      fast_two_sum (h, l, c0[0], th);
      *l += fma (z2l, h_copy, tl);
      a_mul (h, &tl, *h, z);
      *l = fma (*l, z, tl);
      return 0x1.78p-69;
    }
  double v = floor (16.0 * z);
  uint32_t i = 16.0 * z;
  z = (z - 0.03125) - 0.0625 * v;
  const double *c = C[i - 1];
  double z2 = z * z, z4 = z2 * z2;
  double c9 = fma (c[12], z, c[11]);
  double c7 = fma (c[10], z, c[9]);
  double c5 = fma (c[8], z, c[7]);
  double c3h, c3l;
  fast_two_sum (&c3h, &c3l, c[5], z * c[6]);
  c7 = fma (c9, z2, c7);
  fast_two_sum (&c3h, &tl, c3h, c5 * z2);
  c3l += tl;
  fast_two_sum (&c3h, &tl, c3h, c7 * z4);
  c3l += tl;
  double c2h, c2l;
  a_mul (&th, &tl, z, c3h);
  fast_two_sum (&c2h, &c2l, c[4], th);
  c2l += fma (z, c3l, tl);
  a_mul (&th, &tl, z, c2h);
  fast_two_sum (h, l, c[2], th);
  *l += tl + fma (z, c2l, c[3]);
  a_mul (&th, &tl, z, *h);
  tl = fma (z, *l, tl); /* tl += z*l */
  fast_two_sum (h, l, c[0], th);
  *l += tl + c[1];
  return 0x1.11p-69;
}

/* for |z| < 1/8, assuming z >= 2^-61, thus no underflow can occur */
static void
cr_erf_accurate_tiny (double *h, double *l, double z)
{
  /* polynomial generated by erf0_accurate.sollya */
  static const double p[] = {
    0x1.20dd750429b6dp+0,   0x1.1ae3a914fed8p-56,  /* degree 1 */
    -0x1.812746b0379e7p-2,  0x1.ee12e49ca96bap-57, /* degree 3 */
    0x1.ce2f21a042be2p-4,   -0x1.2871bc0a0a0dp-58, /* degree 5 */
    -0x1.b82ce31288b51p-6,  0x1.1003accf1355cp-61, /* degree 7 */
    0x1.565bcd0e6a53fp-8,			   /* degree 9 */
    -0x1.c02db40040cc3p-11,			   /* degree 11 */
    0x1.f9a326fa3cf5p-14,			   /* degree 13 */
    -0x1.f4d25e3c73ce9p-17,			   /* degree 15 */
    0x1.b9eb332b31646p-20,			   /* degree 17 */
    -0x1.64a4bd5eca4d7p-23,			   /* degree 19 */
    0x1.c0acc2502e94ep-25,			   /* degree 21 */
  };
  double z2 = z * z, th, tl;
  *h = p[21 / 2 + 4]; /* degree 21 */
  for (int j = 19; j > 11; j -= 2)
    *h = fma (*h, z2, p[j / 2 + 4]); /* degree j */
  *l = 0;
  for (int j = 11; j > 7; j -= 2)
    {
      /* multiply h+l by z^2 */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      a_mul (h, l, th, z);
      *l = fma (tl, z, *l);
      /* add p[j] to h + l */
      fast_two_sum (h, &tl, p[j / 2 + 4], *h);
      *l += tl;
    }
  for (int j = 7; j >= 1; j -= 2)
    {
      /* multiply h+l by z^2 */
      a_mul (&th, &tl, *h, z);
      tl = fma (*l, z, tl);
      a_mul (h, l, th, z);
      *l = fma (tl, z, *l);
      fast_two_sum (h, &tl, p[j - 1], *h);
      *l += p[j] + tl;
    }
  /* multiply by z */
  a_mul (h, &tl, *h, z);
  *l = fma (*l, z, tl);
  return;
}

/* for 1 <= i < 48, p = C2[i-1] is a degree-18 polynomial approximation of
   erf(i/8+1/16+x) for -1/16 <= x <= 1/16, where the coefficients of
   degree 0 to 7 are double-double: the coefficient of degree 0 is p[0]+p[1],
   ..., that of degree 7 is p[14]+p[15], that of degree 8 is p[16], ...,
   that of degree 18 is p[26].
   Table generated with print_table(1,48,1/16,18,7,rel=true) from erf.sage.
*/
static const double C2[47][27]
    = {
	{ 0x1.ac45e37fe2526p-3,	  0x1.48d48536c61e3p-57,
	  0x1.16e2d7093cd8cp+0,	  0x1.979a52f906b4dp-54,
	  -0x1.a254428ddb453p-3,  0x1.9c98838a77aeap-57,
	  -0x1.59b3da8e1e176p-2,  -0x1.1f650c25d97bp-59,
	  0x1.988648fe88219p-4,	  -0x1.5aecf0c7bb6c1p-58,
	  0x1.803427310d199p-4,	  -0x1.a14576e703eb2p-58,
	  -0x1.09e7bce5592c9p-5,  0x1.eb7c7f3e76998p-60,
	  -0x1.516b205318414p-6,  -0x1.941aa998b1fa4p-61,
	  0x1.038d3f3a16b57p-7,	  0x1.e19d52695ad59p-9,
	  -0x1.9542e7ed01428p-10, -0x1.1f9b6e46418dcp-11,
	  0x1.0796a08a400f4p-12,  0x1.2610d97c70323p-14,
	  -0x1.25d31d73f96d1p-15, -0x1.05e1fa9e02f11p-17,
	  0x1.1e616f979139cp-18,  0x1.9b3d54f1f222ap-21,
	  -0x1.7ad96beea439ap-22 }, /* i=1 108.83 */
	{ 0x1.5da9f415ff23fp-2,	 -0x1.a72e51e19195p-59,
	  0x1.05fd3ecbec298p+0,	 -0x1.f17d49717adf8p-54,
	  -0x1.477c8e7ee733dp-2, -0x1.92236432236b7p-56,
	  -0x1.1917b60acab73p-2, 0x1.c06e6c21b4b3bp-56,
	  0x1.322a728d4ed12p-3,	 0x1.ffa8aef32141p-60,
	  0x1.04c50a9cd2c12p-4,	 -0x1.edd0562dce396p-59,
	  -0x1.7ce764eeddd86p-5, 0x1.9afeb391c029cp-61,
	  -0x1.68aac5801171dp-7, 0x1.4f9655411fc03p-61,
	  0x1.62aa895f51cd3p-7,	 0x1.6c003c3cedb1p-10,
	  -0x1.079502dbbafffp-9, -0x1.d9c7cbb799b47p-14,
	  0x1.45a995aede3f4p-12, 0x1.0c04ea8c98fc9p-20,
	  -0x1.57edfa53128dp-15, 0x1.a96286bf3ef56p-20,
	  0x1.3c8ab12e6d24bp-18, -0x1.7454eba0cb203p-22,
	  -0x1.f02a6f6847617p-23 }, /* i=2 107.115 */
	{ 0x1.db081ce6e2a48p-2,	  -0x1.7ff0a3296d9cbp-56,
	  0x1.dd167c4cf9d2ap-1,	  0x1.44f2832f90a97p-55,
	  -0x1.a173acc35a985p-2,  -0x1.c5432c9a2274p-60,
	  -0x1.889a80f4ad955p-3,  -0x1.f6123bf467942p-57,
	  0x1.6c2eea0d17b39p-3,	  -0x1.1f4935c3cf5b1p-57,
	  0x1.b0645438e5d17p-6,	  0x1.7a5f08ebaf9dp-60,
	  -0x1.a3fd9fcbb6d6dp-5,  0x1.94a1b58b5916fp-59,
	  0x1.060b78c935b8ep-13,  0x1.9cec375875a1cp-68,
	  0x1.678b51a9c4b0ap-7,	  -0x1.1e03bfc8eebb4p-10,
	  -0x1.e653535cab33fp-10, 0x1.55f31366d2c5cp-12,
	  0x1.0dcf1445cbb88p-12,  -0x1.098913ad4dcc7p-14,
	  -0x1.f6e252329eeedp-16, 0x1.41ad0a5afe51dp-17,
	  0x1.8fd4609222f1cp-19,  -0x1.4465926de1a35p-20,
	  -0x1.407a1f42b46d4p-21 }, /* i=3 107.489 */
	{ 0x1.25b8a88b6dd7fp-1,	  0x1.9534a3b5bd215p-55,
	  0x1.a5074e215762p-1,	  0x1.fad8c0ef6fae6p-56,
	  -0x1.d9a837e5824e4p-2,  -0x1.1d19ec86adc7cp-56,
	  -0x1.9c41d1d5fae55p-4,  0x1.74c230d6afba4p-60,
	  0x1.75bebc1b18d1cp-3,	  0x1.01ece95d4dffcp-58,
	  -0x1.6410ad9332666p-7,  -0x1.16523d167a40cp-61,
	  -0x1.7df8890b11fa7p-5,  0x1.d6a99d1387564p-59,
	  0x1.4a54816d3608ap-7,	  -0x1.f810ad06699ccp-61,
	  0x1.18f36eb18f3d7p-7,	  -0x1.8d661c030e174p-9,
	  -0x1.3628ede23e249p-10, 0x1.438eb2b3c4d27p-11,
	  0x1.fd3c13e725e91p-14,  -0x1.991b866a32c87p-14,
	  -0x1.1237c600dab6fp-17, 0x1.a9c701140d4cp-17,
	  0x1.1801e61adfddap-24,  -0x1.785516863e6cep-20,
	  -0x1.3e033ef590125p-23 }, /* i=4 107.484 */
	{ 0x1.569243d2b3a9bp-1,	  0x1.8eef7012e8df4p-56,
	  0x1.681ff24b4ab04p-1,	  -0x1.dba6493354c7p-58,
	  -0x1.ef2bed2786b25p-2,  -0x1.ae3f6b6b2b679p-56,
	  -0x1.a4254557d722fp-7,  -0x1.0ff7bffd10053p-61,
	  0x1.532415c267962p-3,	  0x1.2eacc4bd2e841p-57,
	  -0x1.558b4c55a835cp-5,  0x1.c21c40815d70ap-60,
	  -0x1.1b7ad5b777f1bp-5,  -0x1.2115b2bd8d644p-59,
	  0x1.1201d3bd0e758p-6,	  -0x1.b39b84544256p-63,
	  0x1.2995e3a88a89p-8,	  -0x1.0294c3e93cdbp-8,
	  -0x1.159644a564f83p-12, 0x1.63daf9b3858efp-11,
	  -0x1.3beeb4a1255acp-15, -0x1.80c5178c36c72p-14,
	  0x1.d4f6f5bab7dfap-17,  0x1.521deb6d2f46ep-17,
	  -0x1.4ef3208231a8bp-19, -0x1.e7d2b4e06e4a2p-21,
	  0x1.21536d5b8bdf9p-22 }, /* i=5 107.637 */
	{ 0x1.7fb9bfaed8078p-1,	  0x1.66cf14bcad032p-56,
	  0x1.2a8dcede3673bp-1,	  -0x1.7378e2c70325ep-56,
	  -0x1.e5267029187cp-2,	  0x1.add23841b110ap-56,
	  0x1.fe0796bb9d05ap-5,	  -0x1.7a992e13ce574p-60,
	  0x1.0fa23021ad0acp-3,	  -0x1.17f4228359928p-59,
	  -0x1.fa21ebca76761p-5,  0x1.278ca2820f66cp-60,
	  -0x1.31546d5c4edb4p-6,  0x1.36fcf151892ap-62,
	  0x1.37e5469efb7a6p-6,	  -0x1.9553630321d4fp-60,
	  0x1.097966e2e87eap-13,  -0x1.e82ab020887a7p-9,
	  0x1.318270c11ae74p-11,  0x1.12652e433da97p-11,
	  -0x1.4dc9bd6368bb8p-13, -0x1.c441138d4ff53p-15,
	  0x1.c91d8dc5b66ecp-16,  0x1.f3ba57b86d474p-19,
	  -0x1.dd3403d11a818p-19, -0x1.31f497a106a7cp-24,
	  0x1.436dbcc93d342p-22 }, /* i=6 108.888 */
	{ 0x1.a1551a16aaeafp-1,	  0x1.a558a46df5f68p-57,
	  0x1.dfca26f5bbf88p-2,	  -0x1.ddcbaf85587b6p-57,
	  -0x1.c1cd84866038fp-2,  -0x1.200885b97f453p-56,
	  0x1.e4c9975da0987p-4,	  -0x1.f162e7576c79cp-58,
	  0x1.747e31bf47af3p-4,	  -0x1.6178f12d62ed9p-58,
	  -0x1.1d1f00109e42ap-4,  0x1.002b06e023544p-58,
	  -0x1.47654175ceb42p-9,  0x1.683389ccacfa8p-66,
	  0x1.1a817c594b8cbp-6,	  0x1.36ac477166efbp-60,
	  -0x1.cb8acd699cca6p-9,  -0x1.57b72bf874db6p-9,
	  0x1.24493dca8b6fap-10,  0x1.f556774c6aaf6p-13,
	  -0x1.b09ec5c8ba626p-13, -0x1.09bd1a09f38e8p-18,
	  0x1.d149c3e776976p-16,  -0x1.8f7c2a6575e92p-19,
	  -0x1.8391d4afaf16ap-19, 0x1.5a7552081d1d5p-21,
	  0x1.32d1bb2d1d0cap-22 }, /* i=7 109.125 */
	{ 0x1.bbef0fbde6221p-1,	  -0x1.322c1148e0d48p-55,
	  0x1.75a91a7f4d2edp-2,	  0x1.6eb826a9df85cp-58,
	  -0x1.8d03ac274201cp-2,  0x1.7a5c56eb7f6ap-58,
	  0x1.3954778d6a0dfp-3,	  -0x1.863eca74d1838p-58,
	  0x1.88e0f7b183fc6p-5,	  0x1.226527d05ce39p-59,
	  -0x1.0f7c15f75ee13p-4,  -0x1.56f74f351366p-62,
	  0x1.5e22cfa1aab51p-7,	  0x1.4b49a250c6474p-61,
	  0x1.9ad28c5557c22p-7,	  0x1.99920b730ecd5p-61,
	  -0x1.704ec5d29fc83p-8,  -0x1.23360304f19bap-10,
	  0x1.43ca3fcdf079dp-10,  -0x1.dcb97a9e04bd4p-15,
	  -0x1.735e26c43d267p-13, 0x1.360c3b06ffbb4p-15,
	  0x1.29a6b5798e781p-16,  -0x1.dbc35e4cf98f5p-18,
	  -0x1.2f6e8e81287bbp-20, 0x1.eeb2fdddad355p-21,
	  0x1.1ae65e387ac52p-23 }, /* i=8 108.742 */
	{ 0x1.d0580b2cfd249p-1,	  0x1.4fca6318dfee9p-55,
	  0x1.1a0dc51a9934dp-2,	  -0x1.ca89d2d78fba4p-57,
	  -0x1.4ef05a0f95eebp-2,  -0x1.5f7c55a00231cp-57,
	  0x1.5648b5dc47417p-3,	  -0x1.fb8fa09976e07p-57,
	  0x1.40fbaba44504cp-7,	  0x1.435c75f61f1ep-64,
	  -0x1.c0db89d0a41a4p-5,  -0x1.1dd02d9441b98p-62,
	  0x1.388c3ec056942p-6,	  0x1.8e7498172c914p-60,
	  0x1.aecb7463cf446p-8,	  -0x1.0d6701a009d7p-65,
	  -0x1.8bca53327e075p-8,  0x1.4add4a8239f4ap-12,
	  0x1.05ce4abd10484p-10,  -0x1.183f198a0b62p-12,
	  -0x1.9cd1a9b9fc69bp-14, 0x1.d30363021af83p-15,
	  0x1.da66f2161c4c6p-19,  -0x1.f41f1f238827dp-18,
	  0x1.725a07b1177b7p-21,  0x1.84c3b2483eb6ap-21,
	  0x1.e30e89d6e85cdp-25 }, /* i=9 110.374 */
	{ 0x1.df85ea8db188ep-1,	  -0x1.f71e8254d11a9p-55,
	  0x1.9cb5bd549b111p-3,	  -0x1.973e73caa1edcp-59,
	  -0x1.0ed7443f85c33p-2,  -0x1.74bf040302ad8p-58,
	  0x1.5066cda84bba9p-3,	  0x1.beb86d9e281a8p-59,
	  -0x1.419fa10b6ed7dp-6,  0x1.5157491034c58p-60,
	  -0x1.3f41761d5a941p-5,  0x1.94a1c1f7af153p-59,
	  0x1.6d1d724baaae4p-6,	  0x1.c41090a704426p-60,
	  0x1.e377f5703f7ffp-11,  -0x1.a753be0c53963p-65,
	  -0x1.4cc916ad63c27p-8,  0x1.553ef0d12719fp-10,
	  0x1.26240f55987fdp-11,  -0x1.6bbf0fffb7138p-12,
	  -0x1.320cf6663c40dp-17, 0x1.a9d4850aaa197p-15,
	  -0x1.17036c4011c91p-17, -0x1.441ea26a91a02p-18,
	  0x1.d81eb8e2ef452p-20,  0x1.17d7b798a4322p-22,
	  -0x1.b7b0dfb2559dp-27 }, /* i=10 110.102 */
	{ 0x1.ea7730ed0bbb9p-1,	  0x1.2c5bd7ce1388bp-55,
	  0x1.24a7b84d38971p-3,	  0x1.aa0c5e788ed5ep-57,
	  -0x1.a4b118ef01593p-3,  -0x1.238e3e6a99dep-60,
	  0x1.319c7a75f9187p-3,	  0x1.a8f8fff24b0acp-59,
	  -0x1.3db5bed47faf6p-5,  0x1.29cf699c8512cp-59,
	  -0x1.7019bda6c2fddp-6,  0x1.dd56b84622d88p-62,
	  0x1.59d3aa402c32ep-6,	  -0x1.8de701f1e95e8p-63,
	  -0x1.b324eab9c87a9p-9,  -0x1.c3a4329771a44p-65,
	  -0x1.b4774d37d0dd6p-9,  0x1.c01377485a844p-10,
	  0x1.a5db5f627539bp-14,  -0x1.40d9c429b8932p-12,
	  0x1.e720d935ef7dbp-15,  0x1.fc8295ac052dep-16,
	  -0x1.d1ccde95c6551p-17, -0x1.251c256ca45cbp-20,
	  0x1.e892cc5397b1bp-20,  -0x1.8f6831febdf3dp-23,
	  -0x1.aa5ef30a52421p-22 }, /* i=11 108.586 */
	{ 0x1.f21c9f12f0677p-1,	  -0x1.7efe429672268p-58,
	  0x1.92470a61b6965p-4,	  0x1.c6acd40cee352p-58,
	  -0x1.3a47801c56a57p-3,  -0x1.033705aa16f01p-57,
	  0x1.0453f90d3bd35p-3,	  -0x1.686e281ba5405p-57,
	  -0x1.8a7c6a239217bp-5,  0x1.2a988808a7222p-60,
	  -0x1.075c088031ee3p-7,  -0x1.665bd0a645f4p-62,
	  0x1.16f9c9c127b8p-6,	  -0x1.e1813af47374cp-62,
	  -0x1.74c2fc9bdfe97p-8,  0x1.5cf2dbe53783bp-62,
	  -0x1.760c522bd5becp-10, 0x1.a3cdb656adb44p-10,
	  -0x1.02c3c1ab0a7bap-12, -0x1.92892013c7e15p-13,
	  0x1.6e7b268d42034p-14,  0x1.970751eb9359fp-18,
	  -0x1.b00b549bbdf58p-17, 0x1.033f8545bcc6ap-19,
	  0x1.2d8b6f0a2204ap-20,  -0x1.c1c1335b105c5p-22,
	  0x1.61bbb2d003b8ap-25 }, /* i=12 109.215 */
	{ 0x1.f74a6d9a38383p-1,	  0x1.c33a329423946p-55,
	  0x1.0bf97e95f2a64p-4,	  -0x1.446051f6fef82p-58,
	  -0x1.c435059d09788p-4,  -0x1.b93aeb5e5cf84p-59,
	  0x1.a3687c1eaf1adp-4,	  0x1.64513fb767a13p-58,
	  -0x1.9647a30b16824p-5,  -0x1.86357831221bep-59,
	  0x1.6981061dfbb09p-9,	  -0x1.ccc83193c8742p-64,
	  0x1.7e8755da4704p-7,	  -0x1.c1eaeb337149p-65,
	  -0x1.9be731fdab95dp-8,  -0x1.ab79fedbfccd2p-63,
	  0x1.3a95ae0a75542p-13,  0x1.319f780e962d8p-10,
	  -0x1.b88dd51a4f261p-12, -0x1.037f168a8f581p-14,
	  0x1.53fc5e83e3199p-14,  -0x1.9d5bf30917222p-17,
	  -0x1.03045c999d17ap-17, 0x1.b5d376e96179fp-19,
	  0x1.c66d2e5aa2274p-23,  -0x1.aef24a52bcacap-22,
	  0x1.b20b678e8a0c6p-24 }, /* i=13 110.24 */
	{ 0x1.fab0dd89d1309p-1,	  -0x1.ae61bd9db1babp-55,
	  0x1.5a08e85af27ep-5,	  0x1.e4f9cfc8c2382p-59,
	  -0x1.399812926bc23p-4,  -0x1.b782644df6665p-58,
	  0x1.4140efb719cbp-4,	  0x1.08fa5a48311e8p-60,
	  -0x1.7535a61a4193dp-5,  0x1.59e0501c376b2p-60,
	  0x1.374c88c7e6abdp-7,	  0x1.c2578bd7e3fp-64,
	  0x1.a40709e010e77p-8,	  -0x1.18c33197d9138p-64,
	  -0x1.6dc078888efa7p-8,  0x1.2b49da4c86c7p-63,
	  0x1.2ee6d200993bp-10,	  0x1.44f175e22a161p-11,
	  -0x1.c2fb051c92f92p-12, 0x1.523035ed3964bp-15,
	  0x1.bc7b666856fc1p-15,  -0x1.574549f39ee5p-16,
	  -0x1.c57f3c47b39d9p-20, 0x1.8acc76ac31fcdp-19,
	  -0x1.f70e8b7deaa9ap-22, -0x1.e1a28a0c1a6a6p-23,
	  0x1.bfa0e5b606c5ep-25 }, /* i=14 108.959 */
	{ 0x1.fcdacca0bfb73p-1,	  -0x1.2c33d88729e43p-55,
	  0x1.b1160991ff737p-6,	  -0x1.d940a504353bcp-61,
	  -0x1.a38d59456f77dp-5,  -0x1.d625808eb9778p-62,
	  0x1.d5bd91b6b0123p-5,	  0x1.22b86f5e3e16cp-61,
	  -0x1.3b35dcbc80146p-5,  0x1.82838d776d958p-59,
	  0x1.9d76b0a0535c7p-7,	  0x1.60fda06bca0ap-61,
	  0x1.14c887a83a0e6p-9,	  0x1.55ef222558d68p-65,
	  -0x1.117f42cc6e9f4p-8,  0x1.d4213a7e14a18p-65,
	  0x1.9b477bdad8e08p-10,  0x1.1d219fb0e1bc8p-13,
	  -0x1.5bb59d3ca4fa9p-12, 0x1.8ca373c577821p-14,
	  0x1.4a9b74153a4a3p-16,  -0x1.424a8a883141p-16,
	  0x1.6ce0877965abcp-19,  0x1.c1ed3c11b1dd1p-20,
	  -0x1.86b0a731d831ap-21, -0x1.5cea3996396c5p-26,
	  0x1.640950bde5eb3p-22 }, /* i=15 108.063 */
	{ 0x1.fe307f2b503dp-1,	  -0x1.8a555000387f8p-57,
	  0x1.06ae13b0d3255p-6,	  -0x1.88abd7f4be982p-60,
	  -0x1.0ee3844e59be7p-5,  -0x1.0b0ec94b96d83p-59,
	  0x1.48b127f8ed8a5p-5,	  0x1.b6a1f18c2c162p-60,
	  -0x1.f155b4e7d8c3bp-6,  0x1.adb2d99b0c1fcp-60,
	  0x1.aa2c0753d569ap-7,	  0x1.9a37b9864b8e6p-61,
	  -0x1.bbf7e2795837bp-11, -0x1.4784a66288abfp-65,
	  -0x1.5478d784d271cp-9,  0x1.27115917a7ecp-65,
	  0x1.8eae08cdf9546p-10,  -0x1.92946556037e6p-13,
	  -0x1.90f27ae61444cp-13, 0x1.b076b78538f02p-14,
	  -0x1.b2906f1b92d5dp-18, -0x1.a2f66822d4a01p-17,
	  0x1.3031c4f7c4a97p-18,  0x1.41708ced2abdp-22,
	  -0x1.45ffd6deae2a8p-21, 0x1.e844ebdc8456ap-24,
	  0x1.c0bbf2b711595p-24 }, /* i=16 111.558 */
	{ 0x1.fefcce6813974p-1,	  -0x1.b27cf5025d1c8p-58,
	  0x1.34d7dbc76d7e5p-7,	  0x1.3780d6e7eb351p-61,
	  -0x1.51cc18621fc23p-6,  0x1.969629e4b64a6p-61,
	  0x1.b925a99886bb7p-6,	  0x1.9c8f65efdd1f4p-61,
	  -0x1.71e7d408c8c6fp-6,  -0x1.c5621deaf4cfcp-60,
	  0x1.7ea58080a81efp-7,	  -0x1.2f25b7f384ff3p-61,
	  -0x1.46eb9d203e071p-9,  0x1.ff569e38360a4p-65,
	  -0x1.403333682fa5ep-10, -0x1.36256a95953a6p-65,
	  0x1.3b37d5bd14a4p-10,	  -0x1.6be130822dbdfp-12,
	  -0x1.03d4bcdafd553p-14, 0x1.55848476c8142p-14,
	  -0x1.5492bf3c6eee6p-16, -0x1.3823d4328e9c5p-18,
	  0x1.152fefc353e5ap-18,  -0x1.5199dbf7bc4c6p-21,
	  -0x1.4dda2bebe08f2p-22, 0x1.3fb850b47210ap-23,
	  0x1.bcd1b284c4798p-25 }, /* i=17 112.32 */
	{ 0x1.ff733814af88cp-1,	  0x1.0a87238cea4fap-56,
	  0x1.5ff2750fe782p-8,	  -0x1.5f184847ca667p-62,
	  -0x1.96f0575a63ae5p-7,  0x1.95f4139297a96p-61,
	  0x1.1c5a643f04363p-6,	  -0x1.6ea87997fba3cp-62,
	  -0x1.04f5caaf2196fp-6,  0x1.19502347d3b54p-62,
	  0x1.382a146afb9d2p-7,	  -0x1.f93bde902d2dp-63,
	  -0x1.95cab93aa68d2p-9,  0x1.0f716a5fc18c4p-65,
	  -0x1.d2fd90fe62928p-13, -0x1.4e00d5fcc484ap-68,
	  0x1.9f50fb94c0b86p-11,  -0x1.7d7378074399bp-12,
	  0x1.cc0c9cb9ede1ep-16,  0x1.92a3a29471895p-15,
	  -0x1.7c127858c909ap-16, 0x1.5a72fde935a48p-20,
	  0x1.57b9d90a92106p-19,  -0x1.fdb8443754cf7p-21,
	  -0x1.6c7d633eab55ap-26, 0x1.ddcfc714a2b67p-24,
	  -0x1.9fedf738e84b4p-23 }, /* i=18 108.141 */
	{ 0x1.ffb5bdf67fe6fp-1,	  0x1.4e830346f6e8p-62,
	  0x1.84ba3004a50dp-9,	  -0x1.90b93d4632206p-64,
	  -0x1.d9c2ea85a927dp-8,  -0x1.0bcf1ea93cfdcp-62,
	  0x1.60898536e104ap-7,	  -0x1.ab6aa911c445ep-62,
	  -0x1.5eb1c899f0b7p-7,	  0x1.1bc22eed1f1fbp-61,
	  0x1.d854f73e74c87p-8,	  0x1.7a977a3364c4p-63,
	  -0x1.897719a9d257ep-9,  -0x1.ab523e3f93994p-65,
	  0x1.88cdc8b807c97p-12,  0x1.4875acc7c06ap-70,
	  0x1.b325a11c1f45ap-12,  -0x1.381548f69274p-12,
	  0x1.2b1fd05559bfap-14,  0x1.1ed31cd6feb26p-16,
	  -0x1.29cf593fdf00ap-16, 0x1.1cea99b59228cp-18,
	  0x1.ceff221e3598ap-21,  -0x1.b0ad4b899b2d9p-21,
	  0x1.3761b047e21d1p-23,  0x1.96c31c2256049p-25,
	  -0x1.0a714c57f7adfp-25 }, /* i=19 114.101 */
	{ 0x1.ffd9f78c7524ap-1,	  0x1.04ed6ff98e45dp-55,
	  0x1.a024365f771bdp-10,  0x1.3c8f5202cb405p-64,
	  -0x1.0a9732d5284ddp-8,  -0x1.1acbd0899ce7ep-62,
	  0x1.a4bf47a43042ap-8,	  0x1.e6cb2580d092p-63,
	  -0x1.c23802d8a5bb7p-8,  -0x1.9963700abfc8p-66,
	  0x1.4f40070668329p-8,	  -0x1.e1fe1c0e1182ap-62,
	  -0x1.4c9a2c9dccd04p-9,  0x1.80fb9c9cd78c1p-63,
	  0x1.4f7a50b5bc019p-11,  -0x1.40906b7a1de3ap-66,
	  0x1.18b04eb90c737p-13,  -0x1.a4c3880c0ea69p-13,
	  0x1.4b7b82a86f423p-14,  -0x1.0bc762b1c2aaap-18,
	  -0x1.589d6f8892acfp-17, 0x1.357ab63f7bdf9p-18,
	  -0x1.6675858bbff5ep-22, -0x1.ea96dcb12a15cp-22,
	  0x1.8c572fcf5610ep-23,  -0x1.700c93da86deep-28,
	  0x1.7ae9ceb75e26ep-26 }, /* i=20 110.433 */
	{ 0x1.ffed167b12ac2p-1,	  -0x1.ddc0ce3ed8fcbp-55,
	  0x1.afc85e0f82e12p-11,  0x1.438f22895e03ep-66,
	  -0x1.221a9f326bef4p-9,  -0x1.99642b37af33p-64,
	  0x1.e3c9aab90bcf4p-9,	  0x1.7dcdfdccc72ap-68,
	  -0x1.14b1b98141f21p-8,  0x1.af6edf50eba66p-62,
	  0x1.c1c19b9e63d7p-9,	  0x1.4d1e9411f1d28p-66,
	  -0x1.feac3dbeb5124p-10, 0x1.2400e6ffbc1c8p-65,
	  0x1.63e88178b0e49p-11,  0x1.3e4ae97774f91p-65,
	  -0x1.4441c86c93f39p-15, -0x1.c8ceebc5fc50bp-14,
	  0x1.125b77a79aa6cp-14,  -0x1.da7be990bc718p-17,
	  -0x1.e019960474affp-19, 0x1.d229185ef6279p-19,
	  -0x1.cea9fa10885e7p-21, -0x1.044fd6a2e447ap-23,
	  0x1.3695f88fc641dp-23,  -0x1.0c0dc0ba0d589p-25,
	  -0x1.9194748828b93p-23 }, /* i=21 108.145 */
	{ 0x1.fff6dee89352ep-1,	  0x1.b96c0ba13851dp-55,
	  0x1.b23a5a23e421p-12,	  0x1.727bce1be0014p-67,
	  -0x1.315107613c673p-10, -0x1.823f8673f5b7ap-64,
	  0x1.0c243329a9ca1p-9,	  -0x1.65e361cefe652p-64,
	  -0x1.4630116262084p-9,  -0x1.0ea6ee40daf79p-63,
	  0x1.1e84d1022e8cbp-9,	  -0x1.9b77b85eed4fp-66,
	  -0x1.6b41872716325p-10, 0x1.3e9e001100f64p-66,
	  0x1.36edde582b265p-11,  -0x1.1cb479a94e148p-65,
	  -0x1.f7870ebc38e77p-14, -0x1.51ecfdc37801dp-15,
	  0x1.711d817e0d3b6p-15,  -0x1.0ae90d500d1d8p-16,
	  0x1.a85b1bf54920cp-21,  0x1.fe73958205038p-20,
	  -0x1.d222bfef33aa4p-21, 0x1.833f8b13b1a4ep-24,
	  0x1.233b5a19285dbp-24,  -0x1.1adcf574b7db6p-25,
	  0x1.ab10bedc44532p-24 }, /* i=22 109.015 */
	{ 0x1.fffbb8f1049c6p-1,	  0x1.d2c6266b51f26p-56,
	  0x1.a740684026555p-13,  -0x1.7e24cc3ac571p-69,
	  -0x1.36d34c8f1c26ap-11, -0x1.69d73e7d1c977p-65,
	  0x1.1eb6e14974a25p-10,  -0x1.99b78600e0664p-64,
	  -0x1.714eb8cc0947fp-10, 0x1.3613f37c7410bp-64,
	  0x1.5bec08c01b1d7p-10,  -0x1.3e3a262f6c68ap-64,
	  -0x1.e4621d82dad12p-11, 0x1.302878843e2ccp-67,
	  0x1.e1b7b564b0e79p-12,  0x1.f894fc1f14d54p-67,
	  -0x1.24564b69716aap-13, 0x1.bf8e3b47f3ccdp-20,
	  0x1.8f55a9be1a264p-16,  -0x1.b3b76e6203281p-17,
	  0x1.713c795a07e0cp-19,  0x1.3bb092cfd93ep-21,
	  -0x1.473b0a8333deep-21, 0x1.645526869c143p-23,
	  0x1.1a343e004b33dp-27,  -0x1.76c7e253faad1p-26,
	  0x1.e16080963cffep-24 }, /* i=23 108.853 */
	{ 0x1.fffe0e0140857p-1,	  -0x1.6aa36f86c14dcp-57,
	  0x1.8fdc1b2dcf7b9p-14,  0x1.7050f50b8f308p-71,
	  -0x1.322484cf12daap-12, 0x1.4cc0408806d4fp-66,
	  0x1.27dc1bc6cfef5p-11,  0x1.ffbb5229f6bb7p-65,
	  -0x1.9202f465eb421p-11, 0x1.8f3f063b4066p-69,
	  0x1.93b4c9746835fp-11,  -0x1.04e2d6df2fce5p-65,
	  -0x1.30e9e6142fe9bp-11, -0x1.0396045094744p-66,
	  0x1.555b9d5fb4825p-12,  0x1.9a40d2ca5ef0bp-66,
	  -0x1.055983c4ac7a6p-13, 0x1.68e6c75a5d068p-16,
	  0x1.2d4a50d2757cep-17,  -0x1.1de08b56479aap-17,
	  0x1.9110ccc7fe6fdp-19,  -0x1.bb3184d789af8p-23,
	  -0x1.4629a164e82ap-22,  0x1.413b087ee5e4dp-23,
	  -0x1.648d7786f9fbcp-26, -0x1.293289f8c327dp-27,
	  -0x1.c283008e726f7p-25 }, /* i=24 109.772 */
	{ 0x1.ffff2436a21dcp-1,	  -0x1.3607959a29d36p-55,
	  0x1.6e2367dc27f95p-15,  0x1.d96e6f015102p-73,
	  -0x1.23c436c36fdabp-13, 0x1.f0d77fc600a5p-68,
	  0x1.26bf00867a835p-12,  -0x1.c92e1aecdc75p-66,
	  -0x1.a51fb50b15f22p-12, 0x1.248227c6d226p-69,
	  0x1.c0825378fda08p-12,  0x1.5a8a09c053451p-66,
	  -0x1.6c3dbfe0cbe4ap-12, -0x1.e65769c33f8a1p-66,
	  0x1.c1dd1438378dfp-13,  0x1.91bd161f34158p-69,
	  -0x1.94c36a9d7c0dcp-14, 0x1.bf0aab116ca41p-16,
	  0x1.6bdbd2f10393p-23,	  -0x1.2b32e8d43ef25p-18,
	  0x1.3a7403459770bp-19,  -0x1.17411873320fap-21,
	  -0x1.35bb2691c9b29p-24, 0x1.98313537ed069p-24,
	  -0x1.cb4b60e85a341p-26, 0x1.2be214cf4c9ebp-31,
	  -0x1.350a1a851865ap-23 }, /* i=25 108.474 */
	{ 0x1.ffffa1de8c582p-1,	  0x1.832540129302ap-55,
	  0x1.44f21e49054f2p-16,  0x1.f338cf4086346p-71,
	  -0x1.0d18811478659p-14, 0x1.914a7a08b6a2bp-68,
	  0x1.1b964d438f622p-13,  0x1.a52c94c56aaafp-67,
	  -0x1.a8d7851f26bfp-13,  0x1.c38dbf3ee1223p-67,
	  0x1.ddd6df9b6852dp-13,  -0x1.3b0dd7eac9b91p-67,
	  -0x1.9e52b7aac1644p-13, 0x1.904036dfb5764p-67,
	  0x1.165b2034fcab2p-13,  0x1.27beac4bf3866p-67,
	  -0x1.1b75c3332673ap-14, 0x1.91a253c42f4e7p-16,
	  -0x1.020b498095051p-18, -0x1.ade63f30809aep-20,
	  0x1.89bb0d75e59b7p-20,  -0x1.180c78d3dca28p-21,
	  0x1.cabfd39b38553p-25,  0x1.6013ffba86cfdp-25,
	  -0x1.64f2b123e1f0bp-26, 0x1.35bf3e5021105p-28,
	  -0x1.177828ffd35afp-23 }, /* i=26 108.645 */
	{ 0x1.ffffd8e1a2f22p-1,	  -0x1.c10adf6b19989p-55,
	  0x1.1783ceac2891p-17,	  -0x1.7f19d8ee58337p-71,
	  -0x1.e06a8b37e5b93p-16, 0x1.24e8db1358f2ep-71,
	  0x1.07978c7b8496bp-14,  0x1.f163b5580927cp-68,
	  -0x1.9d039884f8be5p-14, 0x1.fce53cd30b1ebp-68,
	  0x1.e8d1145e94a54p-14,  -0x1.d0f6e009a99eep-68,
	  -0x1.c1f7251172a87p-14, -0x1.3ce0f013dfe9p-71,
	  0x1.458b9e0854d68p-14,  -0x1.897cf3950b1a7p-68,
	  -0x1.6eb0557245429p-15, 0x1.33045cf65279ep-16,
	  -0x1.42c8adf18ab62p-18, 0x1.91109b80f9918p-27,
	  0x1.83a9b44249fbfp-21,  -0x1.9bcbaf0a8dfd1p-22,
	  0x1.900325b58a857p-24,  0x1.4a3cf9c161684p-28,
	  -0x1.0cbcc4d0a916ap-26, 0x1.4275e1b91f084p-28,
	  0x1.39180c75350e1p-23 }, /* i=27 108.443 */
	{ 0x1.fffff039f9e8fp-1,	  -0x1.9d1bcd6174e99p-55,
	  0x1.d21397ead99cbp-19,  -0x1.6abd9c029c47cp-75,
	  -0x1.9f19734d29cf9p-17, 0x1.20c4383da36c1p-71,
	  0x1.d982bd41d8954p-16,  0x1.d9bc9988e9666p-71,
	  -0x1.8320fc4836be5p-15, 0x1.526638b9926a8p-72,
	  0x1.e0a1cb1d071f3p-15,  0x1.d9f5d232bab9p-70,
	  -0x1.d384223047b9cp-15, -0x1.30d0b2b8a170dp-69,
	  0x1.696daf6422bd4p-15,  0x1.ba6ac732f399ep-69,
	  -0x1.bb6e2d311a93fp-16, 0x1.a4fcb0ea87efbp-17,
	  -0x1.1c940c5303dafp-18, 0x1.7469913f4e9c6p-21,
	  0x1.ef4b4f8ab67aep-23,  -0x1.e189c28e8e041p-23,
	  0x1.678b281d5bc55p-24,  -0x1.9c3bf4e9f2b5dp-27,
	  -0x1.74c9ba997ffedp-28, 0x1.b4b843f8c7068p-29,
	  0x1.c901764507862p-25 }, /* i=28 109.895 */
	{ 0x1.fffff9d446cccp-1,	  -0x1.bb06bab98bc8p-57,
	  0x1.789fb715aae95p-20,  -0x1.226d93bf89b4p-80,
	  -0x1.5b333cc7f98f1p-18, -0x1.6bd1091d2544p-72,
	  0x1.9b12fdbf90f62p-17,  0x1.d4b6b0ee9cf46p-71,
	  -0x1.5e06923144d7p-16,  0x1.c59319485786p-75,
	  0x1.c6a071925631dp-16,  -0x1.835ef595952e4p-71,
	  -0x1.d178cb0388a82p-16, -0x1.039272760f01cp-70,
	  0x1.7e29d33ac92b6p-16,  0x1.21ff8b0e9d5ebp-70,
	  -0x1.f9203429baad6p-17, 0x1.094dadeee395cp-17,
	  -0x1.a771cf3500d9fp-19, 0x1.b8fd1c29c21eap-21,
	  -0x1.cc8573d7de11p-26,  -0x1.b0362da1722cbp-24,
	  0x1.e5eae518f94e9p-25,  -0x1.07963addd99a6p-26,
	  -0x1.3f496093d0befp-29, 0x1.99078a326092dp-30,
	  0x1.42681ecfe4da1p-23 }, /* i=29 108.41 */
	{ 0x1.fffffda86faa9p-1,	  -0x1.d230252d68f26p-56,
	  0x1.26f9df8519bd7p-21,  -0x1.e339871c015b7p-75,
	  -0x1.1926290adc888p-19, -0x1.e36d23dbb2644p-73,
	  0x1.5900c02d97304p-18,  0x1.fa7d21e3ed616p-72,
	  -0x1.3166de6a8c64p-17,  0x1.b014157867958p-71,
	  0x1.9dfcc328729ep-17,	  0x1.20e9fee0b7665p-71,
	  -0x1.bcab1ed5ec38dp-17, 0x1.d9003794f0fep-73,
	  0x1.81cd74a57ce17p-17,  -0x1.809fde9c0f6f5p-71,
	  -0x1.106e95b6bf556p-17, 0x1.379625a71385fp-18,
	  -0x1.1970a5b5bd443p-19, 0x1.74761c8333ff2p-21,
	  -0x1.0864e125c9951p-23, -0x1.b83bf9019aa3bp-26,
	  0x1.0397611c35b28p-25,  -0x1.a25392adb29acp-27,
	  -0x1.7b832af40d9d4p-30, 0x1.62a02eb79577bp-32,
	  0x1.a6da58ffe94f4p-23 }, /* i=30 108.026 */
	{ 0x1.ffffff233ee1dp-1,	  0x1.db123ed17221dp-55,
	  0x1.bfd7555a3bd68p-23,  0x1.0151cf177b53ap-77,
	  -0x1.b8d7f804d2e73p-21, 0x1.82b366f0bc2dcp-75,
	  0x1.17f93e5149289p-19,  0x1.91997bfd26568p-76,
	  -0x1.013b0457d08fap-18, -0x1.0d6d5a7f06298p-73,
	  0x1.6b245d7e1d829p-18,  -0x1.9985e02c8ce3bp-72,
	  -0x1.98077548c6951p-18, 0x1.01cd3f1d12c93p-72,
	  0x1.7492048ab3cebp-18,  -0x1.0368a0dc0750ep-72,
	  -0x1.17506c7b39cbp-18,  0x1.57e94a4c5f5a6p-19,
	  -0x1.570971200d7dbp-20, 0x1.0a0f956947b21p-21,
	  -0x1.1a9b7bd5bba32p-23, 0x1.51bfc00de3146p-27,
	  0x1.95b6967f79cbep-27,  -0x1.fe3c43cb3cf84p-28,
	  0x1.2f364a7a2dc5fp-28,  -0x1.007442a10cc14p-32,
	  -0x1.ef5ab6fc5e849p-24 }, /* i=31 108.802 */
	{ 0x1.ffffffb127525p-1,	  0x1.504f382db4102p-55,
	  0x1.4980cb3c80949p-24,  0x1.7fbdd923f8057p-78,
	  -0x1.4ea6ce697296fp-22, 0x1.ea42f9c9de533p-76,
	  0x1.b771d9b6f07b8p-21,  -0x1.e9c1ca9662fe8p-78,
	  -0x1.a26c653fad5b8p-20, -0x1.146c4cee0e898p-74,
	  0x1.3302bb89379dep-19,  0x1.4c55b83ef7a68p-73,
	  -0x1.67f42e5264334p-19, -0x1.6779da26b4197p-73,
	  0x1.58b4adafb958ep-19,  0x1.8351251b45e84p-73,
	  -0x1.10f576796285ap-19, 0x1.66ca44250dd07p-20,
	  -0x1.84ee0ada37543p-21, 0x1.53b6065291e6bp-22,
	  -0x1.c09ebfd0c581cp-24, 0x1.63062625d59cp-26,
	  0x1.e259c60eb7b83p-30,  -0x1.e43802ad25514p-29,
	  0x1.51bcdabe8cda5p-28,  -0x1.930fc3df6e909p-32,
	  -0x1.81cdd770e1c81p-23 }, /* i=32 108.16 */
	{ 0x1.ffffffe4aed5ep-1,	  0x1.389c0f32ad0fp-59,
	  0x1.d5f3a8dea7357p-26,  0x1.fa07c18622dd2p-80,
	  -0x1.ebfb14c9170cp-24,  0x1.9e40632b4145dp-78,
	  0x1.4d9228525f449p-22,  0x1.d35bd7f959136p-77,
	  -0x1.48b536addac5fp-21, -0x1.61ace22b32569p-75,
	  0x1.f48ccf23a68e2p-21,  -0x1.ee1d13c79c281p-75,
	  -0x1.3183b6134cf03p-20, 0x1.e1f4d5fe2a06cp-75,
	  0x1.31efde2215f01p-20,  -0x1.64a7021e23fbap-74,
	  -0x1.fd9eeb0f18fdbp-21, 0x1.63414459ae298p-21,
	  -0x1.9dda81be20b5ap-22, 0x1.8da7d306423c5p-23,
	  -0x1.303da86a4fc28p-24, 0x1.4f5e1327706b9p-26,
	  -0x1.3efb5eefcbe53p-29, -0x1.31bc5ce1ce65dp-30,
	  -0x1.3eafe1b05c93fp-30, -0x1.47fc2d9cc851ep-32,
	  0x1.d27265006a9dfp-24 }, /* i=33 108.887 */
	{ 0x1.fffffff6d1e56p-1,	  -0x1.64d969b4be4c4p-55,
	  0x1.44d26de513197p-27,  0x1.76fc20fc4b365p-81,
	  -0x1.5e32de7af8977p-25, -0x1.888fd6ae18a1cp-80,
	  0x1.e9e05b3c8f38ap-24,  0x1.7532141b12aa7p-78,
	  -0x1.f2f6fa7db5b1dp-23, 0x1.b3bf498e3462cp-77,
	  0x1.899dcace485ebp-22,  0x1.1885a0ae9e878p-78,
	  -0x1.f34b7eef3c9b2p-22, 0x1.294a3b618b47p-76,
	  0x1.04be030272d14p-21,  -0x1.df83095e40f79p-75,
	  -0x1.c73bd22571559p-22, 0x1.4edda838439f5p-22,
	  -0x1.9fc860b504677p-23, 0x1.b0d686a26042p-24,
	  -0x1.72370c2fdbe1p-25,  0x1.ee29f0d197d25p-27,
	  -0x1.b4d88d500c5bep-29, 0x1.96014c45b0178p-35,
	  0x1.238f19dc8fd82p-31,  -0x1.8d34d46ae6567p-33,
	  -0x1.54105fe4a9cd8p-27 }, /* i=34 112.335 */
	{ 0x1.fffffffd01f89p-1,	  -0x1.35e8e39884f62p-56,
	  0x1.b334fac4b9f99p-29,  0x1.32178ed1a4971p-83,
	  -0x1.e2cec6323e50ep-27, -0x1.0e5693f9d4908p-83,
	  0x1.5c027d5bba36ap-25,  -0x1.fc46fb3cc7aep-81,
	  -0x1.6df4d024fffbep-24, -0x1.90fd7226ec57ap-79,
	  0x1.2aaf7c205b9eap-23,  0x1.dbec2005b45a8p-77,
	  -0x1.8902edfbfefddp-23, -0x1.c353aca58d08ap-77,
	  0x1.ab2ab1b338249p-23,  0x1.b498186c39105p-77,
	  -0x1.85abe0ff198d3p-23, 0x1.2d32f7c3621ebp-23,
	  -0x1.8c141c71dbc95p-24, 0x1.b9fa6fbb9b198p-25,
	  -0x1.9db5fe2c2f5b9p-26, 0x1.3b8e07840483ep-27,
	  -0x1.6d95e5070d91dp-29, 0x1.d7616168b0e49p-32,
	  0x1.f2be0744b3a5fp-30,  -0x1.737a375809985p-34,
	  -0x1.936d4936fb865p-24 }, /* i=35 109.095 */
	{ 0x1.ffffffff0dd2bp-1,	  0x1.0df73e7d2fc98p-55,
	  0x1.1a94ff571654fp-30,  0x1.fbf537b47967dp-84,
	  -0x1.4251f33f5578fp-28, 0x1.4c9cece8f41b2p-82,
	  0x1.de6bc1f75bb9bp-27,  0x1.94afb459a3p-87,
	  -0x1.036b5fd1c4158p-25, -0x1.d582afa097896p-79,
	  0x1.b58f1385def96p-25,  -0x1.8778854601996p-80,
	  -0x1.2a2347efb2133p-24, -0x1.26f9e1ef0f378p-79,
	  0x1.508db866ffep-24,	  0x1.64de561a68a21p-78,
	  -0x1.3ffea934685b9p-24, 0x1.02ff87b2e2576p-24,
	  -0x1.66e54eae5fa4bp-25, 0x1.a9ea2195c567dp-26,
	  -0x1.ae3b91fecafa1p-27, 0x1.6bb883d2e5ed1p-28,
	  -0x1.ee10e97715c11p-30, 0x1.e2873d2b77f1fp-32,
	  -0x1.af385ae29d57bp-33, -0x1.d793eecfc2513p-36,
	  0x1.20d80dcfa68d1p-27 }, /* i=36 112.58 */
	{ 0x1.ffffffffb5be5p-1,	  -0x1.729d6819c7f34p-56,
	  0x1.63ac6b4edc88ep-32,  -0x1.c45991835da24p-88,
	  -0x1.a0ce0dc06a706p-30, -0x1.1b72d11da9dabp-84,
	  0x1.3e380dd7593a5p-28,  -0x1.8ad868a7b5674p-82,
	  -0x1.638bc4fb02cbap-27, 0x1.7a84506fcda4p-87,
	  0x1.35753ad4c5875p-26,  0x1.ad190ab170366p-81,
	  -0x1.b41f33cafccbap-26, 0x1.0e3539bf61116p-80,
	  0x1.fe694e371a659p-26,  -0x1.3a84e01866ea8p-82,
	  -0x1.f8af0121aa0abp-26, 0x1.aa77274dab3d8p-26,
	  -0x1.3616fe8f6a259p-26, 0x1.84fddf4c681a1p-27,
	  -0x1.a3de05d1b8a31p-28, 0x1.822529aca9f83p-29,
	  -0x1.26c3dfba84378p-30, 0x1.64c287a84aa09p-32,
	  -0x1.107d2dac5d83bp-31, -0x1.e251d1ab1d873p-43,
	  0x1.8f37005f17b42p-26 }, /* i=37 111.111 */
	{ 0x1.ffffffffe9ebp-1,	  -0x1.ea527e0bef1e8p-58,
	  0x1.b1e5acf351d87p-34,  0x1.dc96583ba19fp-90,
	  -0x1.05042a0a5f3c3p-31, -0x1.2023f0f13867cp-85,
	  0x1.99ac8fd63c66cp-30,  -0x1.bf57c5fd0501ap-85,
	  -0x1.d72344378e114p-29, 0x1.c77758959af41p-83,
	  0x1.a6be9a123435bp-28,  0x1.dab4af8807c36p-83,
	  -0x1.33aacb4bf6deap-27, 0x1.bd241ea49ac35p-81,
	  0x1.74b732e7ceaa7p-27,  0x1.c7c89730b0264p-82,
	  -0x1.7e7eab6531ccbp-27, 0x1.50959f2daae39p-27,
	  -0x1.ffed4cef94261p-28, 0x1.51c7f99f908a2p-28,
	  -0x1.82b5fd5fbedfcp-29, 0x1.7e1c8e715c978p-30,
	  -0x1.51536822c861bp-31, 0x1.be7e4c220ca82p-33,
	  0x1.f5bb67c461296p-29,  0x1.1d7cf04529bfp-37,
	  -0x1.acc021ab828c4p-23 }, /* i=38 108.008 */
	{ 0x1.fffffffff9a1bp-1,	  -0x1.6a87270d2450cp-57,
	  0x1.0084ff125639dp-35,  -0x1.8ad61debedc86p-90,
	  -0x1.3ca42adaa26f6p-33, 0x1.c20c6583dccddp-87,
	  0x1.fe73513c67bf8p-32,  0x1.20d28c0c7e686p-86,
	  -0x1.2dd9aa5a2bee3p-30, 0x1.d76d7235461bep-85,
	  0x1.16ef6b93944a8p-29,  -0x1.f07bd785566dep-83,
	  -0x1.a2d58e9b22b26p-29, -0x1.19e6ea91dd55ep-84,
	  0x1.06389b9748f25p-28,  0x1.fbcc52565c0bep-82,
	  -0x1.16cdd9eb58ba2p-28, 0x1.fdd861b55c5p-29,
	  -0x1.9457846c943d2p-29, 0x1.178f3905f435cp-29,
	  -0x1.518cf20c53de2p-30, 0x1.6329939a34b66p-31,
	  -0x1.5ef3ad85e5d3bp-32, 0x1.f2b41494e49e9p-34,
	  0x1.bad43bc0b622dp-29,  0x1.21a45fa9dcebfp-37,
	  -0x1.790b3d88f69fep-23 }, /* i=39 108.193 */
	{ 0x1.fffffffffe38p-1,	  0x1.7ce07114e4fep-55,
	  0x1.25f9ee0b923dcp-37,  -0x1.174c43a73a4d1p-91,
	  -0x1.74105146a5162p-35, -0x1.7d0740e56625cp-91,
	  0x1.33cde4f35d941p-33,  -0x1.2a344950797c6p-88,
	  -0x1.760fe7b666392p-32, 0x1.a8b77c82ed644p-86,
	  0x1.63a70fd66d485p-31,  0x1.6b87715649d6dp-85,
	  -0x1.1324f6fb6dfa1p-30, 0x1.3fc045e39915fp-84,
	  0x1.63a31a36b815cp-30,  -0x1.02dec9bc1a7p-90,
	  -0x1.8724ca8970b91p-30, 0x1.72e290891e5dep-30,
	  -0x1.31fc03858aab1p-30, 0x1.b9e8b0e7fa253p-31,
	  -0x1.1821a002637bdp-31, 0x1.37ba5f3fba5ebp-32,
	  -0x1.3578bf23dc654p-33, 0x1.fdaf2015d7b54p-35,
	  0x1.7f6a435069067p-32,  0x1.9d14ee557ec62p-38,
	  -0x1.55f4c743ee571p-26 }, /* i=40 111.334 */
	{ 0x1.ffffffffff845p-1,	  0x1.b0edc5a89ab8ep-56,
	  0x1.46897d4b69fc6p-39,  0x1.a74852415bb49p-93,
	  -0x1.a77a4e7dcd735p-37, -0x1.34edb43ab7de6p-91,
	  0x1.67543695dcc12p-35,  -0x1.29ae577004af8p-92,
	  -0x1.c05c1e2fc710ep-34, 0x1.dbbf42d2537a8p-90,
	  0x1.b639419fedf8ep-33,  -0x1.ed72eb9e7a59ep-87,
	  -0x1.5cfd7eb9bfe87p-32, -0x1.e97db27125fcp-88,
	  0x1.d11578959ba45p-32,  -0x1.c0635ac2b5768p-87,
	  -0x1.082f9e9f7eb37p-31, 0x1.0354ceadad8b3p-31,
	  -0x1.bc2dee0154fc6p-32, 0x1.4e11efdc66eaep-32,
	  -0x1.bb357c0253f64p-33, 0x1.035f9889bc29cp-33,
	  -0x1.8e7bdb10b7441p-35, 0x1.e364571102661p-36,
	  -0x1.12cffcf49a2e8p-29, 0x1.ee9362bcfec26p-39,
	  0x1.cc5b58dd85301p-24 }, /* i=41 108.905 */
	{ 0x1.ffffffffffdf8p-1,	  -0x1.dcf8b10ff973bp-55,
	  0x1.5f8b87a31bd85p-41,  0x1.65b265455b658p-98,
	  -0x1.d2e55024a0fb5p-39, 0x1.444e1d84cea02p-93,
	  0x1.9612cc225df4bp-37,  -0x1.c784edb664ce7p-91,
	  -0x1.03ee5f38b9b4dp-35, -0x1.91ca8efa41a3p-89,
	  0x1.04f2f71e2e96bp-34,  -0x1.33f36a4e5135p-89,
	  -0x1.ab7099f99ced9p-34, -0x1.4af7a67f2110cp-90,
	  0x1.2554b8f609fd1p-33,  -0x1.29e641eb44218p-88,
	  -0x1.57c87529ca968p-33, 0x1.5cd182c967671p-33,
	  -0x1.3580a2517d57ap-33, 0x1.e3be72b1be982p-34,
	  -0x1.4e9908689ad08p-34, 0x1.9a61979d3395bp-35,
	  -0x1.7b826aadd1c89p-36, 0x1.ad3a9fc4a0d1ep-37,
	  -0x1.0e9325ed2097p-31,  0x1.0722198ff452cp-39,
	  0x1.c2ef85611aa11p-26 }, /* i=42 110.935 */
	{ 0x1.fffffffffff7bp-1,	  0x1.00fa07f7fb612p-55,
	  0x1.6ed2f2515e933p-43,  0x1.2bc1802a42b92p-98,
	  -0x1.f2a6c1669c901p-41, -0x1.7b3e174cc184p-95,
	  0x1.bc42ba38a13f8p-39,  0x1.460463d59d3dfp-93,
	  -0x1.2391e135afae4p-37, -0x1.bd08c8c5f7b18p-92,
	  0x1.2c6c24550f64fp-36,  -0x1.fdc861a48711p-92,
	  -0x1.f9a3c1b0d63ecp-36, -0x1.843dc8d9ad3d5p-90,
	  0x1.6502546ab341ap-35,  0x1.45f812e48eb98p-89,
	  -0x1.af223186006d1p-35, 0x1.c388dd1764f41p-35,
	  -0x1.9e65a242b52aap-35, 0x1.4fcd2787781ebp-35,
	  -0x1.e3cbdb20a48d6p-36, 0x1.35639e9fcd41p-36,
	  -0x1.5b8e97774b2c9p-42, 0x1.66ffe6a100bc9p-38,
	  -0x1.5706c390c113ep-30, 0x1.ff0d11cf61949p-41,
	  0x1.2054de347e3f8p-24 }, /* i=43 109.58 */
	{ 0x1.fffffffffffdfp-1,	  0x1.5669e670f914cp-56,
	  0x1.72fd93e036cdcp-45,  0x1.1c553d12fbbdp-100,
	  -0x1.01f450d1e61b2p-42, 0x1.bed807e60c078p-99,
	  0x1.d68fb81b2ed89p-41,  0x1.c7ea3c4444ccp-98,
	  -0x1.3c706aa4d2328p-39, 0x1.d6d2d51dd414dp-93,
	  0x1.4e6479565838ep-38,  0x1.50580f36c14c1p-92,
	  -0x1.20e9eb83b3dd9p-37, -0x1.04b6334a32fdp-94,
	  0x1.a35b9d2fcac8p-37,	  0x1.c07c6978bf2fp-94,
	  -0x1.04a134f6e3dcbp-36, 0x1.196579f27ddbep-36,
	  -0x1.0ab97aa74c7p-36,	  0x1.bf68355f542b1p-37,
	  -0x1.49da25a547134p-37, 0x1.bd64993a3958ep-38,
	  -0x1.1193990186399p-35, 0x1.1c0e98335ae18p-39,
	  0x1.e08edb685494ap-29,  0x1.cb9fcc058465bp-42,
	  -0x1.94cacccfb8964p-23 }, /* i=44 108.091 */
	{ 0x1.ffffffffffff8p-1,	  0x1.0160ef15c497ep-56,
	  0x1.6ba91ac734786p-47,  -0x1.f81d6fa69b5b2p-101,
	  -0x1.028a39099f4dbp-44, -0x1.83ed68de15404p-99,
	  0x1.e292863e1795ep-43,  -0x1.b292e812abb68p-98,
	  -0x1.4c4e690fbdd14p-41, -0x1.80991e1d4ef25p-95,
	  0x1.67e6e5ac60fd1p-40,  0x1.1d2ca68dcf0e8p-95,
	  -0x1.3f00d80afa00cp-39, -0x1.3e174dc7225acp-93,
	  0x1.db88ee63eb28ap-39,  0x1.8abd97527892fp-93,
	  -0x1.2fe58a1f19368p-38, 0x1.51dbeae22a5c8p-38,
	  -0x1.4a4d54823e0fcp-38, 0x1.1e432d674cfbap-38,
	  -0x1.b001e26c6e764p-39, 0x1.328ce695259fep-39,
	  -0x1.4e492cf7d4f4cp-36, 0x1.aaa77d339dcp-41,
	  0x1.366538db382e5p-29,  0x1.826ad7d581503p-43,
	  -0x1.056e0810b14dap-23 }, /* i=45 108.721 */
	{ 0x1.ffffffffffffep-1,	  0x1.59ab24e589a3p-56,
	  0x1.5982008db1304p-49,  -0x1.1cf9bda64b38ap-103,
	  -0x1.f610e8cde57acp-47, -0x1.884dcd86f98c8p-102,
	  0x1.df2dac2f2d47fp-45,  -0x1.7f27bf279d988p-102,
	  -0x1.51b17f95fc0b4p-43, -0x1.063e04485c3e7p-97,
	  0x1.76996ddc975d7p-42,  -0x1.21489d6648428p-97,
	  -0x1.546155a972b18p-41, -0x1.9d3fb518aa7cp-100,
	  0x1.0456ed89c4f24p-40,  0x1.eee772fc32c5ep-94,
	  -0x1.55d6295aa388ap-40, 0x1.86ead99977388p-40,
	  -0x1.89b3d387efa6ep-40, 0x1.6011e175e64f8p-40,
	  -0x1.0cd70515af47bp-40, 0x1.9402199dfdde7p-41,
	  -0x1.806743bc32b08p-37, 0x1.30e561550364ap-42,
	  0x1.70093985e2c1bp-30,  0x1.31999a27ace63p-44,
	  -0x1.35f54db0f4dbcp-24 }, /* i=46 109.476 */
	{ 0x1p+0,
	  -0x1.a6d7d18831888p-55,
	  0x1.3e296303b2297p-51,
	  0x1.68cf648cfed1cp-105,
	  -0x1.d8456ef97c744p-49,
	  0x1.fcded170055p-103,
	  0x1.ccb92e6c24c8dp-47,
	  -0x1.a704dc202cff2p-101,
	  -0x1.4c1aa8cf1229bp-45,
	  0x1.652efa61e4ec2p-99,
	  0x1.7918b6b83c0fbp-44,
	  0x1.2fb01fb8836dcp-100,
	  -0x1.5f073659de44dp-43,
	  -0x1.9ceb48a2d1931p-97,
	  0x1.134d070b5921ep-42,
	  0x1.9af3038fcc184p-98,
	  -0x1.730a2938c09ddp-42,
	  0x1.b4091041f5905p-42,
	  -0x1.c3c44ab8c8421p-42,
	  0x1.a06b4f4c3044dp-42,
	  -0x1.704f511555fe7p-42,
	  0x1.fe51fcfc1acbap-43,
	  0x1.5e7229a07e7cdp-38,
	  0x1.9f8121f6c3146p-44,
	  -0x1.692b2f9b3f445p-31,
	  0x1.c8c34f73d3823p-46,
	  0x1.301e540260d52p-25 }, /* i=47 110.503 */
      };

/* Assuming 0 <= z <= 0x1.7afb48dc96626p+2, put in h+l an accurate
   approximation of erf(z).
   Assumes z >= 2^-61, thus no underflow can occur. */
__attribute__ ((cold)) static void
cr_erf_accurate (double *h, double *l, double z)
{
  double th, tl;
  if (z < 0.125) /* z < 1/8 */
    return cr_erf_accurate_tiny (h, l, z);
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

// Multiply a double with a double double : a * (bh + bl)
static inline void
s_mul (double *hi, double *lo, double a, double bh, double bl)
{
  a_mul (hi, lo, a, bh); /* exact */
  *lo = fma (a, bl, *lo);
}

// Returns (ah + al) * (bh + bl) - (al * bl)
static inline void
d_mul (double *hi, double *lo, double ah, double al, double bh, double bl)
{
  a_mul (hi, lo, ah, bh);
  *lo = fma (ah, bl, *lo);
  *lo = fma (al, bh, *lo);
}

// Add a + (bh + bl), assuming |a| >= |bh|
static inline void
fast_sum (double *hi, double *lo, double a, double bh, double bl)
{
  fast_two_sum (hi, lo, a, bh);
  /* |(a+bh)-(hi+lo)| <= 2^-105 |hi| and |lo| < ulp(hi) */
  *lo += bl;
  /* |(a+bh+bl)-(hi+lo)| <= 2^-105 |hi| + ulp(lo),
     where |lo| <= ulp(hi) + |bl|. */
}

/* For 0 <= i < 64, T1[i] = (h,l) such that h+l is the best double-double
   approximation of 2^(i/64). The approximation error is bounded as follows:
   |h + l - 2^(i/64)| < 2^-107. */
static const double T1[][2] = {
  { 0x1p+0, 0x0p+0 },
  { 0x1.02c9a3e778061p+0, -0x1.19083535b085dp-56 },
  { 0x1.059b0d3158574p+0, 0x1.d73e2a475b465p-55 },
  { 0x1.0874518759bc8p+0, 0x1.186be4bb284ffp-57 },
  { 0x1.0b5586cf9890fp+0, 0x1.8a62e4adc610bp-54 },
  { 0x1.0e3ec32d3d1a2p+0, 0x1.03a1727c57b53p-59 },
  { 0x1.11301d0125b51p+0, -0x1.6c51039449b3ap-54 },
  { 0x1.1429aaea92dep+0, -0x1.32fbf9af1369ep-54 },
  { 0x1.172b83c7d517bp+0, -0x1.19041b9d78a76p-55 },
  { 0x1.1a35beb6fcb75p+0, 0x1.e5b4c7b4968e4p-55 },
  { 0x1.1d4873168b9aap+0, 0x1.e016e00a2643cp-54 },
  { 0x1.2063b88628cd6p+0, 0x1.dc775814a8495p-55 },
  { 0x1.2387a6e756238p+0, 0x1.9b07eb6c70573p-54 },
  { 0x1.26b4565e27cddp+0, 0x1.2bd339940e9d9p-55 },
  { 0x1.29e9df51fdee1p+0, 0x1.612e8afad1255p-55 },
  { 0x1.2d285a6e4030bp+0, 0x1.0024754db41d5p-54 },
  { 0x1.306fe0a31b715p+0, 0x1.6f46ad23182e4p-55 },
  { 0x1.33c08b26416ffp+0, 0x1.32721843659a6p-54 },
  { 0x1.371a7373aa9cbp+0, -0x1.63aeabf42eae2p-54 },
  { 0x1.3a7db34e59ff7p+0, -0x1.5e436d661f5e3p-56 },
  { 0x1.3dea64c123422p+0, 0x1.ada0911f09ebcp-55 },
  { 0x1.4160a21f72e2ap+0, -0x1.ef3691c309278p-58 },
  { 0x1.44e086061892dp+0, 0x1.89b7a04ef80dp-59 },
  { 0x1.486a2b5c13cdp+0, 0x1.3c1a3b69062fp-56 },
  { 0x1.4bfdad5362a27p+0, 0x1.d4397afec42e2p-56 },
  { 0x1.4f9b2769d2ca7p+0, -0x1.4b309d25957e3p-54 },
  { 0x1.5342b569d4f82p+0, -0x1.07abe1db13cadp-55 },
  { 0x1.56f4736b527dap+0, 0x1.9bb2c011d93adp-54 },
  { 0x1.5ab07dd485429p+0, 0x1.6324c054647adp-54 },
  { 0x1.5e76f15ad2148p+0, 0x1.ba6f93080e65ep-54 },
  { 0x1.6247eb03a5585p+0, -0x1.383c17e40b497p-54 },
  { 0x1.6623882552225p+0, -0x1.bb60987591c34p-54 },
  { 0x1.6a09e667f3bcdp+0, -0x1.bdd3413b26456p-54 },
  { 0x1.6dfb23c651a2fp+0, -0x1.bbe3a683c88abp-57 },
  { 0x1.71f75e8ec5f74p+0, -0x1.16e4786887a99p-55 },
  { 0x1.75feb564267c9p+0, -0x1.0245957316dd3p-54 },
  { 0x1.7a11473eb0187p+0, -0x1.41577ee04992fp-55 },
  { 0x1.7e2f336cf4e62p+0, 0x1.05d02ba15797ep-56 },
  { 0x1.82589994cce13p+0, -0x1.d4c1dd41532d8p-54 },
  { 0x1.868d99b4492edp+0, -0x1.fc6f89bd4f6bap-54 },
  { 0x1.8ace5422aa0dbp+0, 0x1.6e9f156864b27p-54 },
  { 0x1.8f1ae99157736p+0, 0x1.5cc13a2e3976cp-55 },
  { 0x1.93737b0cdc5e5p+0, -0x1.75fc781b57ebcp-57 },
  { 0x1.97d829fde4e5p+0, -0x1.d185b7c1b85d1p-54 },
  { 0x1.9c49182a3f09p+0, 0x1.c7c46b071f2bep-56 },
  { 0x1.a0c667b5de565p+0, -0x1.359495d1cd533p-54 },
  { 0x1.a5503b23e255dp+0, -0x1.d2f6edb8d41e1p-54 },
  { 0x1.a9e6b5579fdbfp+0, 0x1.0fac90ef7fd31p-54 },
  { 0x1.ae89f995ad3adp+0, 0x1.7a1cd345dcc81p-54 },
  { 0x1.b33a2b84f15fbp+0, -0x1.2805e3084d708p-57 },
  { 0x1.b7f76f2fb5e47p+0, -0x1.5584f7e54ac3bp-56 },
  { 0x1.bcc1e904bc1d2p+0, 0x1.23dd07a2d9e84p-55 },
  { 0x1.c199bdd85529cp+0, 0x1.11065895048ddp-55 },
  { 0x1.c67f12e57d14bp+0, 0x1.2884dff483cadp-54 },
  { 0x1.cb720dcef9069p+0, 0x1.503cbd1e949dbp-56 },
  { 0x1.d072d4a07897cp+0, -0x1.cbc3743797a9cp-54 },
  { 0x1.d5818dcfba487p+0, 0x1.2ed02d75b3707p-55 },
  { 0x1.da9e603db3285p+0, 0x1.c2300696db532p-54 },
  { 0x1.dfc97337b9b5fp+0, -0x1.1a5cd4f184b5cp-54 },
  { 0x1.e502ee78b3ff6p+0, 0x1.39e8980a9cc8fp-55 },
  { 0x1.ea4afa2a490dap+0, -0x1.e9c23179c2893p-54 },
  { 0x1.efa1bee615a27p+0, 0x1.dc7f486a4b6bp-54 },
  { 0x1.f50765b6e454p+0, 0x1.9d3e12dd8a18bp-54 },
  { 0x1.fa7c1819e90d8p+0, 0x1.74853f3a5931ep-55 },
};

/* For 0 <= i < 64, T2[i] = (h,l) such that h+l is the best double-double
   approximation of 2^(i/2^12). The approximation error is bounded as follows:
   |h + l - 2^(i/2^12)| < 2^-107. */
static const double T2[][2] = {
  { 0x1p+0, 0x0p+0 },
  { 0x1.000b175effdc7p+0, 0x1.ae8e38c59c72ap-54 },
  { 0x1.00162f3904052p+0, -0x1.7b5d0d58ea8f4p-58 },
  { 0x1.0021478e11ce6p+0, 0x1.4115cb6b16a8ep-54 },
  { 0x1.002c605e2e8cfp+0, -0x1.d7c96f201bb2fp-55 },
  { 0x1.003779a95f959p+0, 0x1.84711d4c35e9fp-54 },
  { 0x1.0042936faa3d8p+0, -0x1.0484245243777p-55 },
  { 0x1.004dadb113dap+0, -0x1.4b237da2025f9p-54 },
  { 0x1.0058c86da1c0ap+0, -0x1.5e00e62d6b30dp-56 },
  { 0x1.0063e3a559473p+0, 0x1.a1d6cedbb9481p-54 },
  { 0x1.006eff583fc3dp+0, -0x1.4acf197a00142p-54 },
  { 0x1.007a1b865a8cap+0, -0x1.eaf2ea42391a5p-57 },
  { 0x1.0085382faef83p+0, 0x1.da93f90835f75p-56 },
  { 0x1.00905554425d4p+0, -0x1.6a79084ab093cp-55 },
  { 0x1.009b72f41a12bp+0, 0x1.86364f8fbe8f8p-54 },
  { 0x1.00a6910f3b6fdp+0, -0x1.82e8e14e3110ep-55 },
  { 0x1.00b1afa5abcbfp+0, -0x1.4f6b2a7609f71p-55 },
  { 0x1.00bcceb7707ecp+0, -0x1.e1a258ea8f71bp-56 },
  { 0x1.00c7ee448ee02p+0, 0x1.4362ca5bc26f1p-56 },
  { 0x1.00d30e4d0c483p+0, 0x1.095a56c919d02p-54 },
  { 0x1.00de2ed0ee0f5p+0, -0x1.406ac4e81a645p-57 },
  { 0x1.00e94fd0398ep+0, 0x1.b5a6902767e09p-54 },
  { 0x1.00f4714af41d3p+0, -0x1.91b2060859321p-54 },
  { 0x1.00ff93412315cp+0, 0x1.427068ab22306p-55 },
  { 0x1.010ab5b2cbd11p+0, 0x1.c1d0660524e08p-54 },
  { 0x1.0115d89ff3a8bp+0, -0x1.e7bdfb3204be8p-54 },
  { 0x1.0120fc089ff63p+0, 0x1.843aa8b9cbbc6p-55 },
  { 0x1.012c1fecd613bp+0, -0x1.34104ee7edae9p-56 },
  { 0x1.0137444c9b5b5p+0, -0x1.2b6aeb6176892p-56 },
  { 0x1.01426927f5278p+0, 0x1.a8cd33b8a1bb3p-56 },
  { 0x1.014d8e7ee8d2fp+0, 0x1.2edc08e5da99ap-56 },
  { 0x1.0158b4517bb88p+0, 0x1.57ba2dc7e0c73p-55 },
  { 0x1.0163da9fb3335p+0, 0x1.b61299ab8cdb7p-54 },
  { 0x1.016f0169949edp+0, -0x1.90565902c5f44p-54 },
  { 0x1.017a28af25567p+0, 0x1.70fc41c5c2d53p-55 },
  { 0x1.018550706ab62p+0, 0x1.4b9a6e145d76cp-54 },
  { 0x1.019078ad6a19fp+0, -0x1.008eff5142bf9p-56 },
  { 0x1.019ba16628de2p+0, -0x1.77669f033c7dep-54 },
  { 0x1.01a6ca9aac5f3p+0, -0x1.09bb78eeead0ap-54 },
  { 0x1.01b1f44af9f9ep+0, 0x1.371231477ece5p-54 },
  { 0x1.01bd1e77170b4p+0, 0x1.5e7626621eb5bp-56 },
  { 0x1.01c8491f08f08p+0, -0x1.bc72b100828a5p-54 },
  { 0x1.01d37442d507p+0, -0x1.ce39cbbab8bbep-57 },
  { 0x1.01de9fe280ac8p+0, 0x1.16996709da2e2p-55 },
  { 0x1.01e9cbfe113efp+0, -0x1.c11f5239bf535p-55 },
  { 0x1.01f4f8958c1c6p+0, 0x1.e1d4eb5edc6b3p-55 },
  { 0x1.020025a8f6a35p+0, -0x1.afb99946ee3fp-54 },
  { 0x1.020b533856324p+0, -0x1.8f06d8a148a32p-54 },
  { 0x1.02168143b0281p+0, -0x1.2bf310fc54eb6p-55 },
  { 0x1.0221afcb09e3ep+0, -0x1.c95a035eb4175p-54 },
  { 0x1.022cdece68c4fp+0, -0x1.491793e46834dp-54 },
  { 0x1.02380e4dd22adp+0, -0x1.3e8d0d9c49091p-56 },
  { 0x1.02433e494b755p+0, -0x1.314aa16278aa3p-54 },
  { 0x1.024e6ec0da046p+0, 0x1.48daf888e9651p-55 },
  { 0x1.02599fb483385p+0, 0x1.56dc8046821f4p-55 },
  { 0x1.0264d1244c719p+0, 0x1.45b42356b9d47p-54 },
  { 0x1.027003103b10ep+0, -0x1.082ef51b61d7ep-56 },
  { 0x1.027b357854772p+0, 0x1.2106ed0920a34p-56 },
  { 0x1.0286685c9e059p+0, -0x1.fd4cf26ea5d0fp-54 },
  { 0x1.02919bbd1d1d8p+0, -0x1.09f8775e78084p-54 },
  { 0x1.029ccf99d720ap+0, 0x1.64cbba902ca27p-58 },
  { 0x1.02a803f2d170dp+0, 0x1.4383ef231d207p-54 },
  { 0x1.02b338c811703p+0, 0x1.4a47a505b3a47p-54 },
  { 0x1.02be6e199c811p+0, 0x1.e47120223467fp-54 },
};

/* The following is a degree-4 polynomial generated by Sollya for exp(x)
   over [-0.000130273,0.000130273] with absolute error < 2^-74.346. */
static const double Q_1[] = {
  0x1p0,		/* degree 0 */
  0x1p0,		/* degree 1 */
  0x1p-1,		/* degree 2 */
  0x1.5555555995d37p-3, /* degree 3 */
  0x1.55555558489dcp-5	/* degree 4 */
};

// Approximation for the fast path of exp(z) for z=zh+zl,
// with |z| < 0.000130273 < 2^-12.88 and |zl| < 2^-42.6
// (assuming x^y does not overflow or underflow)
static inline void
q_1 (double *hi, double *lo, double zh, double zl)
{
  double z = zh + zl;
  double q = fma (Q_1[4], zh, Q_1[3]);

  q = fma (q, z, Q_1[2]);

  fast_two_sum (hi, lo, Q_1[1], q * z);

  d_mul (hi, lo, zh, zl, *hi, *lo);

  fast_sum (hi, lo, Q_1[0], *hi, *lo);
}

/*
  Approximation of exp(x), where x = xh + xl

  exp(x) is approximated by hi + lo.

  For the error analysis, we only consider the case where x^y does not
  overflow or underflow. We get:

  (hi + lo) / exp(xh + xl) = 1 + eps with |eps| < 2^-74.139

  Assumes |xl/xh| < 2^-23.89 and |xl| < 2^-14.3486.

  At output, we also have 0.99985 < hi+lo < 1.99995 and |lo/hi| < 2^-41.4.
*/

static inline void
exp_1 (double *hi, double *lo, double xh, double xl)
{

#define INVLOG2 0x1.71547652b82fep+12 /* |INVLOG2-2^12/log(2)| < 2^-43.4 */
  double k = roundeven_finite (xh * INVLOG2);

  double kh, kl;
#define LOG2H 0x1.62e42fefa39efp-13
#define LOG2L 0x1.abc9e3b39803fp-68
  s_mul (&kh, &kl, k, LOG2H, LOG2L);

  double yh, yl;
  fast_two_sum (&yh, &yl, xh - kh, xl);
  yl -= kl;

  int64_t K = k; /* Note: k is an integer, this is just a conversion. */
  int64_t M = (K >> 12) + 0x3ff;
  int64_t i2 = (K >> 6) & 0x3f;
  int64_t i1 = K & 0x3f;

  double t1h = T1[i2][0], t1l = T1[i2][1], t2h = T2[i1][0], t2l = T2[i1][1];
  d_mul (hi, lo, t2h, t2l, t1h, t1l);

  double qh, ql;
  q_1 (&qh, &ql, yh, yl);

  d_mul (hi, lo, *hi, *lo, qh, ql);

  double df = asdouble (M << 52);
  *hi *= df;
  *lo *= df;
}

/* the following contains polynomials generated by Sollya for
   erfc(1/x)*exp(1/x^2), over various ranges.
   Polynomials are of degree 23, with only odd coefficients, of type double
   except the degree-1 one which is a double-double.
   With p=T[i], the degree-1 coefficient is p[0] + p[1],
   the degree-3 coefficient is p[2], ...,
   the degree-23 coefficient is p[12]. In each line, the value is comment is
   the relative error bound given by Sollya. */
static const double T[6][13] = {
  { 0x1.20dd750429b6dp-1, 0x1.1a1feb75a48a8p-57, -0x1.20dd750429b6cp-2,
    0x1.b14c2f863e403p-2, -0x1.0ecf9db3af35dp+0, 0x1.d9eb53ca6eeedp+1,
    -0x1.0a945830d95c8p+4, 0x1.6e8a963e2f1f5p+6, -0x1.29b7ccc8f396fp+9,
    0x1.15e716e83c27ep+12, -0x1.1cfdcfbcaf22ap+15, 0x1.1986cc7a7e8fep+18,
    -0x1.71f7540590a91p+20 }, /* asympt0.sollya,
				 [0x1.2ce37fb080c7dp-5,0x1.d5p-4], 2^-74.788,
				 |p'/p| < 27.2 */
  { 0x1.20dd750429ae7p-1, 0x1.63da89e801fd4p-55, -0x1.20dd750400795p-2,
    0x1.b14c2f57c490cp-2, -0x1.0ecf95c8c9014p+0, 0x1.d9e981f2321efp+1,
    -0x1.0a81482de1506p+4, 0x1.6d662420a604bp+6, -0x1.233c96fff7772p+9,
    0x1.f5d62018d3e37p+11, -0x1.9ae55e955445p+14, 0x1.052901e10d139p+17,
    -0x1.66465df1385fp+18 }, /* asympt1.sollya,
				[0x1.d5p-4,0x1.59da6ca291ba6p-3], 2^-72.938,
				|p'/p| < 8.63 */
  { 0x1.20dd75041e3fcp-1, -0x1.c9b491c4920fcp-56, -0x1.20dd74e5f1526p-2,
    0x1.b14c1d35a40ep-2, -0x1.0ecdecd30e86bp+0, 0x1.d9b4e7f725263p+1,
    -0x1.0958b5ca8fb39p+4, 0x1.63e3179bf609cp+6, -0x1.06bbd1cd2d0fdp+9,
    0x1.7b66eb6d1d2f2p+11, -0x1.ce5a4b1afab75p+13, 0x1.8b5c6ae6f773cp+15,
    -0x1.5475860326f86p+16 }, /* asympt2.sollya,
				 [0x1.59da6ca291ba6p-3,0x1.bcp-3], 2^-72.15,
				 |p'/p| < 5.77 */
  { 0x1.20dd75025cfe9p-1, 0x1.5a92eef32fb2p-58, -0x1.20dd71eb9d4e7p-2,
    0x1.b14af4c25db28p-2, -0x1.0ebc78a22b3d8p+0, 0x1.d85287a0b3399p+1,
    -0x1.045f751e5ca1dp+4, 0x1.4a0d87ddea589p+6, -0x1.ac6a0981d1eeep+8,
    0x1.f44822f567956p+10, -0x1.cba372de71349p+12, 0x1.1a4a19f550ca4p+14,
    -0x1.52a580455ed79p+14 }, /* asympt3.sollya, [0x1.bcp-3,0x1.0cp-2],
				 2^-71.804, |p'/p| < 4.42 */
  { 0x1.20dd74eb31d84p-1, -0x1.39c4054b7c09p-59, -0x1.20dd561af98c4p-2,
    0x1.b1435165d9df1p-2, -0x1.0e6b60308e94p+0, 0x1.d3ce30c140882p+1,
    -0x1.f2083e404c299p+3, 0x1.20f113d89b42ap+6, -0x1.41433ebd89f19p+8,
    0x1.2f35b6a3154f6p+10, -0x1.b020a4313cf3bp+11, 0x1.90f07e92da7eep+12,
    -0x1.6565e1d7665c3p+12 }, /* asympt4.sollya, [0x1.0cp-2,0x1.38p-2],
				 2^-71.811, |p'/p| < 3.60 */
  { 0x1.20dd744b3517bp-1, -0x1.f77ab25e01ab4p-57, -0x1.20dcc62ec4024p-2,
    0x1.b125bfa4f66c1p-2, -0x1.0d80e6538197p+0, 0x1.ca11fbcfa65b2p+1,
    -0x1.cd9eaffb88315p+3, 0x1.e010db42e0da7p+5, -0x1.c5c85250ef6a3p+7,
    0x1.5e118d9c1eeafp+9, -0x1.8d74be13d3d3p+10, 0x1.211b1b2b5ac83p+11,
    -0x1.900be759fc663p+10 }, /* asympt5.sollya, [0x1.38p-2,0x1.63p-2],
				 2^-71.841, |p'/p| < 3.04 */
};

/* the following is a degree-19 polynomial approximating exp(x) for
   1/sqrt(2) <= x <= sqrt(2) with relative error bounded by 2^-104.311
   (see file exp_accurate.sollya).
   Coefficients of degree 0-7 are double-double and stored first,
   coefficients of degree 8-19 are double and follow. */
static const double E2[] = {
  0x1p+0,
  -0x1p-105, // degree 0
  0x1p+0,
  -0x1.e2p-100, // degree 1
  0x1p-1,
  0x1.3cp-95, // degree 2
  0x1.5555555555555p-3,
  0x1.55555555c78d9p-57, // degree 3
  0x1.5555555555555p-5,
  0x1.55555545616e2p-59, // degree 4
  0x1.1111111111111p-7,
  0x1.11110121fc314p-63, // degree 5
  0x1.6c16c16c16c17p-10,
  -0x1.f49e06ee3a56ep-65, // degree 6
  0x1.a01a01a01a01ap-13,
  0x1.b053e1eeab9cp-73,	 // degree 7
  0x1.a01a01a01a01ap-16, // degree 8
  0x1.71de3a556c733p-19, // degree 9
  0x1.27e4fb7789f66p-22, // degree 10
  0x1.ae64567f54abep-26, // degree 11
  0x1.1eed8eff8958bp-29, // degree 12
  0x1.6124613837216p-33, // degree 13
  0x1.93974aaf26a57p-37, // degree 14
  0x1.ae7f4fd6d0bd9p-41, // degree 15
  0x1.ae7e982620b25p-45, // degree 16
  0x1.94e4ca59460d8p-49, // degree 17
  0x1.69a2a4b7ef36dp-53, // degree 18
  0x1.abfe1602308c9p-57, // degree 19
};

/* put in 2^e*(h+l) an approximation of exp(xh+xl) for -742 <= xh+xl <= -2.92,
   with target accuracy 104 bits */
static void
exp_accurate (double *h, double *l, int *e, double xh, double xl)
{
  double th, tl, yh, yl;
  /* first reduce argument: xh + xl ~ k*log(2) + yh + yl */
#define INVLOG2acc 0x1.71547652b82fep+0 // approximates 1/log(2)
  int k = roundeven_finite (xh * INVLOG2acc);
  /* since |xh| <= 742, |k| <= round(742/log(2)) = 1070 */
  /* subtract k*log(2), where LOG2H+LOG2L approximates log(2) */
#define LOG2Hacc 0x1.62e42fefa39efp-1
  /* we approximate LOG2Lacc ~ log(2) - LOG2H with 38 bits, so that
     k*LOG2Lacc is exact (k has at most 11 bits) */
#define LOG2Lacc 0x1.abc9e3b398p-56
#define LOG2tiny 0x1.f97b57a079a19p-103
  yh = fma (-k, LOG2Hacc, xh);
  /* since |xh+xl| >= 2.92 we have |k| >= 4;
  (|k|-1/2)*log(2) <= |x| <= (|k|+1/2)*log(2) thus
  1-1/(2|k|) <= |x/(k*log(2))| <= 1+1/(2|k|) thus by Sterbenz theorem
  yh is exact too */
  two_sum (&th, &tl, -(double) k * LOG2Lacc, xl);
  fast_two_sum (&yh, &yl, yh, th);
  yl = fma (-k, LOG2tiny, yl + tl);
  /* now yh+yl approximates xh + xl - k*log(2), and we approximate p(yh+yl)
     in h + l */
  /* Since |xh| <= 742, we assume |xl| <= ulp(742) = 2^-43. Then since
     |k| <= round(742/log(2)) = 1070, |yl| <= 1070*LOG2L + 2^-42 < 2^-42.7.
     Since |yh| <= log(2)/2, the contribution of yl is negligible as long
     as |i*p[i]*yh^(i-1)*yl| < 2^-104, which holds for i >= 16.
     Thus for coefficients of degree 16 or more, we don't take yl into account.
  */
  *h = E2[19 + 8]; // degree 19
  for (int i = 18; i >= 16; i--)
    *h = fma (*h, yh, E2[i + 8]); // degree i
  /* degree 15: h*(yh+yl)+E2[15 + 8] */
  a_mul (&th, &tl, *h, yh);
  tl = fma (*h, yl, tl);
  fast_two_sum (h, l, E2[15 + 8], th);
  *l += tl;
  for (int i = 14; i >= 8; i--)
    {
      /* degree i: (h+l)*(yh+yl)+E2[i+8] */
      a_mul (&th, &tl, *h, yh);
      tl = fma (*h, yl, tl);
      tl = fma (*l, yh, tl);
      fast_two_sum (h, l, E2[i + 8], th);
      *l += tl;
    }
  for (int i = 7; i >= 0; i--)
    {
      /* degree i: (h+l)*(yh+yl)+E2[2i]+E2[2i+1] */
      a_mul (&th, &tl, *h, yh);
      tl = fma (*h, yl, tl);
      tl = fma (*l, yh, tl);
      fast_two_sum (h, l, E2[2 * i], th);
      *l += tl + E2[2 * i + 1];
    }
  /* we have to multiply h,l by 2^k */
  *e = k;
}

/* Fast path for 0x1.713786d9c7c09p+1 < x < 0x1.b39dc41e48bfdp+4,
   using the asymptotic formula erfc(x) = exp(-x^2) * p(1/x), where
   p(x) is computed with Sollya (files asympt[0-5].sollya).
   Return a bound on the absolute error. */
static double
erfc_asympt_fast (double *h, double *l, double x)
{
  /* for x >= 0x1.9db1bb14e15cap+4, erfc(x) < 2^-970, and we might encounter
     underflow issues in the computation of l, thus we delegate this case
     to the accurate path */
  if (x >= 0x1.9db1bb14e15cap+4)
    {
      *h = 0;
      *l = 0;
      return 1.0;
    }

  /* first approximate exp(-x^2): */
  double eh, el, uh, ul;
  a_mul (&uh, &ul, x, x);
  exp_1 (&eh, &el, -uh, -ul);
  /* the assumptions from exp_1 are satisfied:
     * a_mul ensures |ul| <= ulp(uh), thus |ul/uh| <= 2^-52
     * since |x| < 0x1.9db1bb14e15cap+4 we have
       |ul| < ulp(0x1.9db1bb14e15cap+4^2) = 2^-43 */
  /* eh+el approximates exp(-x^2) with maximal relative error 2^-74.139 */

  /* compute 1/x as double-double */
  double yh, yl;
  yh = 1.0 / x;
  /* Assume 1 <= x < 2, then 0.5 <= yh <= 1,
     and yh = 1/x + eps with |eps| <= 2^-53. */
  /* Newton's iteration for 1/x is y -> y + y*(1-x*y) */
  yl = yh * fma (-x, yh, 1.0);
  /* x*yh-1 = x*(1/x+eps)-1 = x*eps
     with |x*eps| <= 2^-52, thus the error on the FMA is bounded by
     ulp(2^-52.1) = 2^-105.
     Now |yl| <= |yh| * 2^-52 <= 2^-52, thus the rounding error on
     yh * fma (-x, yh, 1.0) is bounded also by ulp(2^-52.1) = 2^-105.
     From [6], Lemma 3.7, if yl was computed exactly, then yh+yl would differ
     from 1/x by at most yh^2/theta^3*(1/x-yh)^2 for some theta in [yh,1/x]
     or [1/x,yh].
     Since yh, 1/x <= 1, this gives eps^2 <= 2^-106.
     Adding the rounding errors, we have:
     |yh + yl - 1/x| <= 2^-105 + 2^-105 + 2^-106 < 2^-103.67.
     For the relative error, since |yh| >= 1/2, this gives:
     |yh + yl - 1/x| < 2^-102.67 * |yh+yl|
  */

  /* look for the right interval for yh */
  static const double threshold[]
      = { 0x1.d5p-4, 0x1.59da6ca291ba6p-3, 0x1.bcp-3, 0x1.0cp-2, 0x1.38p-2,
	  0x1.63p-2 };
  int i;
  for (i = 0; yh > threshold[i]; i++)
    ;

  const double *p = T[i];
  a_mul (&uh, &ul, yh, yh); // exact
  /* Since |yh| <= 1, we have |uh| <= 1 and |ul| <= 2^-53. */
  ul = fma (2.0 * yh, yl, ul);
  /* uh+ul approximates (yh+yl)^2, with absolute error bounded by
     ulp(ul) + yl^2, where ulp(ul) is the maximal rounding error in
     the FMA, and yl^2 is the neglected term.
     Since |ul| <= 2^-53, ulp(ul) <= 2^-105, and since |yl| <= 2^-52,
     this yields |uh + ul - yh^2| <= 2^-105 + 2^-104 < 2^-103.41.
     For the relative error, since |(yh+yl)^2| >= 1/4:
     |uh + ul - yh^2| < 2^-101.41 * |uh+ul|.
     And relatively to 1/x^2:
     yh + yl = 1/x * (1 + eps1)       with |eps1| < 2^-102.67
     uh + ul = (yh+yl)^2 * (1 + eps2) with |eps2| < 2^-101.41
     This yields:
     |uh + ul - 1/x| < 2^-100.90 * |uh+ul|.
  */

  /* evaluate p(uh+ul) */
  double zh, zl;
  zh = p[12];		    // degree 23
  zh = fma (zh, uh, p[11]); // degree 21
  zh = fma (zh, uh, p[10]); // degree 19
  /* degree 17: zh*(uh+ul)+p[i] */
  s_mul (h, l, zh, uh, ul);
  fast_two_sum (&zh, &zl, p[9], *h);
  zl += *l;

  for (int j = 15; j >= 3; j -= 2)
    {
      d_mul (h, l, zh, zl, uh, ul);
      fast_two_sum (&zh, &zl, p[(j + 1) / 2], *h);
      zl += *l;
    }
  /* degree 1: (zh+zl)*(uh+ul)+p[0]+p[1] */
  d_mul (h, l, zh, zl, uh, ul);
  fast_two_sum (&zh, &zl, p[0], *h);
  zl += *l + p[1];
  /* multiply by yh+yl */
  d_mul (&uh, &ul, zh, zl, yh, yl);
  /* now uh+ul approximates p(1/x) */
  /* now multiply (uh+ul)*(eh+el) */
  d_mul (h, l, uh, ul, eh, el);
  /* Write y = 1/x.  We have the following errors:
     * the maximal mathematical error is:
       |erfc(x)*exp(x^2) - p(y)| < 2^-71.804 * |p(y)| (for i=3) thus
       |erfc(x) - exp(-x^2)*p(y)| < 2^-71.804 * |exp(-x^2)*p(y)|
     * the error in approximating exp(-x^2) by eh+el:
       |eh + el - exp(-x^2)| < 2^-74.139 * |eh + el|
     * the fact that we evaluate p on yh+yl instead of 1/x
       this error is bounded by |p'| * |yh+yl - 1/x|, where
       |yh+yl - 1/x| < 2^-102.67 * |yh+yl|, and the relative
       error is bounded by |p'/p| * |yh+yl - 1/x|.
       Since the maximal value of |p'/p| is bounded by 27.2 (for i=0),
       this yields 27.2 * 2^-102.67 < 2^-97.9
     * the rounding errors when evaluating p on yh+yl: this error is bounded
       (relatively) by 2^-67.184 (for i=5), see analyze_erfc_asympt_fast()
       in erfc.sage
     * the rounding error in (uh+ul)*(eh+el): we assume this error is bounded
       by 2^-80 (relatively)
     This yields a global relative bound of:
     (1+2^-71.804)*(1+2^-74.139)*(1+2^-97.9)*(1+2^-67.184)*(1+2^-80)-1
     < 2^-67.115
  */
  // avoid a spurious underflow in 0x1.d9p-68 * h
  if (*h >= 0x1.151b9a3fdd5c9p-955)
    return 0x1.d9p-68 * *h; /* 2^-67.115 < 0x1.d9p-68 */
  else
    return 0x1p-1022; // this overestimates 0x1.d9p-68 * h
}

/* given -0x1.7744f8f74e94bp2 < x < 0x1.b39dc41e48bfdp+4,
   put in h+l a double-double approximation of erfc(x),
   with *absolute* error bounded by err (the returned value) */
static double
cr_erfc_fast (double *h, double *l, double x)
{
  /* on a i7-8700 with gcc 12.2.0, for x in [-5,0], the average reciprocal
     throughput is about 44 cycles */
  if (x < 0) // erfc(x) = 1 - erf(x) = 1 + erf(-x)
    {
      double err = cr_erf_fast (h, l, -x);
      /* h+l approximates erf(-x), with relative error bounded by err,
	 where err <= 0x1.78p-69 */
      err = err * *h; /* convert into absolute error */
      double t;
      fast_two_sum (h, &t, 1.0, *h);
      // since h <= 2, the fast_two_sum() error is bounded by 2^-105*h <=
      // 2^-104
      *l = t + *l;
      /* After the fast_two_sum() call, we have |t| <= ulp(h) <= ulp(2) = 2^-51
	 thus assuming |l| <= 2^-51 after the cr_erf_fast() call,
	 we have |t| <= 2^-50 here, thus the rounding
	 error on t -= *l is bounded by ulp(2^-50) = 2^-102.
	 The absolute error is thus bounded by err + 2^-104 + 2^-102
	 = err + 0x1.4p-102.
	 The maximal value of err here is for |x| < 0.0625, where cr_erf_fast()
	 returns 0x1.78p-69, and h=1/2, yielding err = 0x1.78p-70 here.
	 Adding 0x1.4p-102 is thus exact. */
      return err + 0x1.4p-102;
    }
    // now 0 <= x < 0x1.b39dc41e48bfdp+4
#define THRESHOLD1 0x1.713786d9c7c09p+1
  /* on a i7-8700 with gcc 12.2.0, for x in [0,THRESHOLD1],
     the average reciprocal throughput is about 59 cycles */
  else if (x <= THRESHOLD1)
    {
      double err = cr_erf_fast (h, l, x);
      /* h+l approximates erf(x), with relative error bounded by err,
	 where err <= 0x1.78p-69 */
      err = err * *h; /* convert into absolute error */
      double t;
      fast_two_sum (h, &t, 1.0, -*h);
      *l = t - *l;
      /* for x >= 0x1.e861fbb24c00ap-2, erf(x) >= 1/2, thus 1-h is exact
	 by Sterbenz theorem, thus t = 0 in fast_two_sum(), and we have t = -l
	 here, thus the absolute error is err */
      if (x >= 0x1.e861fbb24c00ap-2)
	return err;
      /* for x < 0x1.e861fbb24c00ap-2, the error in fast_two_sum() is bounded
	 by 2^-105*h, and since h <= 1/2, this yields 2^-106.
	 After the fast_two_sum() call, we have |t| <= ulp(h) <= ulp(1/2) =
	 2^-53 thus assuming |l| <= 2^-53 after the cr_erf_fast() call, we have
	 |t| <= 2^-52 here, thus the rounding error on t -= *l is bounded by
	 ulp(2^-52) = 2^-104. The absolute error is thus bounded by err +
	 2^-106 + 2^-104 = err + 0x1.4p-104. The maximal value of err here is
	 for x < 0.0625, where cr_erf_fast() returns 0x1.78p-69, and h=1/2,
	 yielding err = 0x1.78p-70 here. Adding 0x1.4p-104 is thus exact. */
      return err + 0x1.4p-104;
    }
  /* Now THRESHOLD1 < x < 0x1.b39dc41e48bfdp+4 thus erfc(x) < 0.000046. */
  /* on a i7-8700 with gcc 12.2.0, for x in [THRESHOLD1,+5.0],
     the average reciprocal throughput is about 111 cycles
     (among which 20 cycles for exp_1) */
  return erfc_asympt_fast (h, l, x);
}

/* The following are polynomial approximations for erfc(1/x)*exp(1/x^2)
   over various intervals covering [0x1.2ce37fb080c7dp-5,0x1.2b81f34bfce36p-1].
   Polynomials have only odd coefficients, with the first six coefficients
   (degree 1 to 11) being double-double, the remaining coefficients double.
*/
static const double Tacc[10][30]
    = {
	{ 0x1.20dd750429b6dp-1,	 0x1.1ae3a912b08fp-57,
	  -0x1.20dd750429b6dp-2, -0x1.1ae34c0606d68p-58,
	  0x1.b14c2f863e924p-2,	 -0x1.96c0f4c848fc8p-56,
	  -0x1.0ecf9db3e71b6p+0, 0x1.45d756bd288bp-57,
	  0x1.d9eb53fad4672p+1,	 -0x1.c61629de9adf2p-53,
	  -0x1.0a945f3d147eap+4, 0x1.8fec5ad7ece2p-52,
	  0x1.6e8c02f27ca6dp+6,	 -0x1.29d1c21c363ep+9,
	  0x1.17349b70be627p+12, -0x1.28a6bb4686182p+15,
	  0x1.602d1662523cap+18, -0x1.ccae7625c4111p+21,
	  0x1.4237d064f6e0dp+25, -0x1.b1e5466ca3a2fp+28,
	  0x1.90ae06a0f6cc1p+31 }, /* asympt_acc0.sollya:
				      [0x1.2ce37fb080c7dp-5,0x1.45p-4], degree
				      29, relerr <= 2^-107.71 */
	{ 0x1.20dd750429b6dp-1,	 0x1.1adaa62435c1p-57,
	  -0x1.20dd750429b6dp-2, -0x1.41516126827c8p-59,
	  0x1.b14c2f863e90bp-2,	 0x1.a535780ba5ed4p-56,
	  -0x1.0ecf9db3e65d6p+0, -0x1.089edde27ad07p-54,
	  0x1.d9eb53fa52f2p+1,	 -0x1.bc9737e9464acp-53,
	  -0x1.0a945f2cd7621p+4, -0x1.589f28b700332p-51,
	  0x1.6e8bffd7e194ep+6,	 -0x1.29d18716876e2p+9,
	  0x1.17312abe1825p+12,	 -0x1.287e73592805cp+15,
	  0x1.5ebf7394a39c1p+18, -0x1.c2f14d46d0cf9p+21,
	  0x1.2af3d256f955ep+25, -0x1.7041659ebd7aap+28,
	  0x1.6039c232e2f71p+31, -0x1.70ca15c5a07cbp+33 }, /* asympt_acc1.sollya:
							      [0x1.45p-4,0x1.e0p-4],
							      degree 31, relerr
							      <= 2^-106.203 */
	{ 0x1.20dd750429b6dp-1,	 0x1.d3c35b5d3741p-58,
	  -0x1.20dd750429b56p-2, -0x1.c028415f6f81bp-56,
	  0x1.b14c2f863c1cfp-2,	 0x1.1bb0de6470dbcp-58,
	  -0x1.0ecf9db33c363p+0, 0x1.0f8068459eb16p-55,
	  0x1.d9eb53b9ce57bp+1,	 0x1.20cce33e7d84ap-53,
	  -0x1.0a945aa2ec4fap+4, -0x1.f6e0fcd7c603p-50,
	  0x1.6e8b824d2bfaap+6,	 -0x1.29cc372a6d0bp+9,
	  0x1.1703a99ddd429p+12, -0x1.2749f9a267cc6p+15,
	  0x1.5856a17271849p+18, -0x1.a8bcb4ba9753fp+21,
	  0x1.035dcce88294p+25,	 -0x1.1e5d8c5e6e043p+28,
	  0x1.fe3b4f365386ep+30, -0x1.398fdef2b98fep+33,
	  0x1.84234d4f4ea12p+34 }, /* asympt_acc2.sollya:
				      [0x1.e0p-4,0x1.3fp-3], degree 33, relerr
				      <= 2^-105.859 */
	{ 0x1.20dd750429b6ap-1,	 0x1.ae622b765e9fdp-55,
	  -0x1.20dd750428f0ep-2, 0x1.03c6c67d69513p-56,
	  0x1.b14c2f8563e8ep-2,	 0x1.766a6bd7aa89cp-57,
	  -0x1.0ecf9d8dedd48p+0, 0x1.0af52e90336e3p-54,
	  0x1.d9eb4aad086fep+1,	 0x1.640d371d54a19p-53,
	  -0x1.0a93f1d01cfep+4,	 -0x1.68dbd8d9c522cp-51,
	  0x1.6e842e9fd5898p+6,	 -0x1.299886ef1fb8p+9,
	  0x1.15e0f0162c9ap+12,	 -0x1.222dbc6b04cd8p+15,
	  0x1.460268db1ebdfp+18, -0x1.74f53ce065fb3p+21,
	  0x1.961ca8553f87p+24,	 -0x1.8788395d13798p+27,
	  0x1.35e37b25d0e81p+30, -0x1.707b7457c8f5ep+32,
	  0x1.1ff852df1c023p+34, -0x1.b75d0ec56e2cdp+34 }, /* asympt_acc3.sollya:
							      [0x1.3fp-3,0x1.95p-3],
							      degree 35, relerr
							      <= 2^-105.557 */
	{ 0x1.20dd750429a8fp-1,	 -0x1.66d8dda59bceap-56,
	  -0x1.20dd7503fdbabp-2, 0x1.707bdffc2b3fep-57,
	  0x1.b14c2f6526025p-2,	 -0x1.7fa4bb954114p-61,
	  -0x1.0ecf99c417d45p+0, -0x1.748645ef7af94p-54,
	  0x1.d9eaa9c712a7dp+1,	 0x1.79e478994ebb4p-53,
	  -0x1.0a8ef11fbf141p+4, 0x1.b5c72d69f8954p-52,
	  0x1.6e4653e0455b1p+6,	 -0x1.286909448e6cfp+9,
	  0x1.113424ce76821p+12, -0x1.1346d859e76dep+15,
	  0x1.1f9f6cf2293bfp+18, -0x1.258e6e3b337dbp+21,
	  0x1.14029ecd465fbp+24, -0x1.c530df5337a6fp+26,
	  0x1.34bc4bbccd336p+29, -0x1.4a37c52641688p+31,
	  0x1.019707cec2974p+33, -0x1.031fe736ea169p+34,
	  0x1.f6b3003de3ddfp+33 }, /* asympt_acc4.sollya:
				      [0x1.95p-3,0x1.f5p-3], degree 37, relerr
				      <= 2^-105.539 */
	{ 0x1.20dd75042756bp-1,	 0x1.4ad9178b5691p-55,
	  -0x1.20dd74feda9e8p-2, -0x1.8141c70bbc8d6p-56,
	  0x1.b14c2cb128467p-2,	 -0x1.09aebaa106821p-56,
	  -0x1.0ecf603921a0bp+0, 0x1.7d3cb5bceaf0bp-54,
	  0x1.d9e3e1751ca59p+1,	 0x1.6622ae564267p-56,
	  -0x1.0a686af57f547p+4, 0x1.083b320aff6b6p-51,
	  0x1.6cf0b6c027326p+6,	 -0x1.23afcb69443d3p+9,
	  0x1.03ab450d9f1b9p+12, -0x1.e74cdb76bcab4p+14,
	  0x1.c671b60e607f1p+17, -0x1.8f1376d324ce4p+20,
	  0x1.3b64276234676p+23, -0x1.aff0ce13c5a8ep+25,
	  0x1.ef20247251e87p+27, -0x1.c9f5662f721f6p+29,
	  0x1.4687858e185e1p+31, -0x1.4fa507be073c2p+32,
	  0x1.b99ac35ee4accp+32, -0x1.16cb585ee3fa9p+32 }, /* asympt_acc5.sollya:
							      [0x1.f5p-3,0x1.31p-2],
							      degree 39, relerr
							      <= 2^-105.722 */
	{ 0x1.20dd7503e730dp-1,	 0x1.4e524a098a467p-55,
	  -0x1.20dd7498fa6b2p-2, 0x1.60a4e27751c8p-61,
	  0x1.b14c061bd2a0cp-2,	 0x1.95a8f847d2fc2p-57,
	  -0x1.0ecd0f11b8c7dp+0, -0x1.4126deea76061p-54,
	  0x1.d9b1344463548p+1,	 0x1.fe09a4eca9b0ep-53,
	  -0x1.0996ea52a87edp+4, -0x1.924f920db26cp-53,
	  0x1.67a2264b556bp+6,	 -0x1.15dfc2c86b6b5p+9,
	  0x1.ccc291b62efe4p+11, -0x1.81375a78e746ap+14,
	  0x1.33a6f15546329p+17, -0x1.c1e9dc121601p+19,
	  0x1.2397ea3d43fdap+22, -0x1.4661e5b2ea512p+24,
	  0x1.3412367ca5d45p+26, -0x1.de56b9d7f37c4p+27,
	  0x1.2851d9722146dp+29, -0x1.19027baf0c3fep+30,
	  0x1.7e7b8b6ab58acp+30, -0x1.4c446d56aaf22p+30,
	  0x1.1492190400505p+29 }, /* asympt_acc6.sollya:
				      [0x1.31p-2,0x1.71p-2], degree 41, relerr
				      <= 2^-105.433 */
	{ 0x1.20dd74ff10852p-1,	 0x1.a32f26deff875p-55,
	  -0x1.20dd6f06c491cp-2, 0x1.70c16e1793358p-56,
	  0x1.b14a7d5e7fd4ap-2,	 0x1.479998b54db5bp-56,
	  -0x1.0ebbdb3889c5fp+0, -0x1.59b853e11369cp-56,
	  0x1.d89dd249d7ef8p+1,	 -0x1.4b5edf0c8c314p-55,
	  -0x1.06526fb386114p+4, -0x1.40d04eed7c7ep-55,
	  0x1.57ff657e429cep+6,	 -0x1.ef63e90d3863p+8,
	  0x1.6d4f34c4ea3dap+11, -0x1.04542b9e36a54p+14,
	  0x1.577bf19097738p+16, -0x1.9702fe47c736dp+18,
	  0x1.a7ae12b54fdc6p+20, -0x1.7ca3f0f7c4fa9p+22,
	  0x1.225d983963cbfp+24, -0x1.71a6eac612f9ep+25,
	  0x1.8086324225e1ep+26, -0x1.3de68670a7716p+27,
	  0x1.91674de4dcbe9p+27, -0x1.6b44cc15b76c2p+27,
	  0x1.a36dae0f30d8p+26,	 -0x1.cffc1747ea3dcp+24 }, /* asympt_acc7.sollya:
							      [0x1.71p-2,0x1.bcp-2],
							      degree 43, relerr
							      <= 2^-105.629 */
	{ 0x1.20dd74ba8f3p-1,	 -0x1.9dd256871d21p-58,
	  -0x1.20dd3593675bcp-2, 0x1.ec0e7ffa91ad9p-56,
	  0x1.b13eef86a077ap-2,	 -0x1.4fb5d78d411b8p-56,
	  -0x1.0e5cf52a11f3ap+0, -0x1.51f36c779dc8cp-55,
	  0x1.d4417a08b39d5p+1,	 0x1.1be9fb5956638p-54,
	  -0x1.f91b9f6ce80c3p+3, -0x1.c9c99dd42829cp-51,
	  0x1.356439f45bb43p+6,	 -0x1.8c0ca12819b48p+8,
	  0x1.efcad2ecd6671p+10, -0x1.21b0af6fc1039p+13,
	  0x1.327d215ee30c9p+15, -0x1.1fabda96167bp+17,
	  0x1.d82e4373b315dp+18, -0x1.4ed9e2ff591e9p+20,
	  0x1.95c85dcd8eab5p+21, -0x1.9f016f0a3d62ap+22,
	  0x1.60e89d918b96fp+23, -0x1.e97be202cba64p+23,
	  0x1.0d8a081619793p+24, -0x1.c5422b4fcfc65p+23,
	  0x1.1131a9dc6aed1p+23, -0x1.a457d9dced257p+21,
	  0x1.3605e980e8b86p+19 }, /* asympt_acc8.sollya:
				      [0x1.bcp-2,0x1.0bp-1], degree 45, relerr
				      <= 2^-105.064 */
	{ 0x1.20dd7319d4d25p-1,	 0x1.2b02992c3b7abp-55,
	  -0x1.20dc29c13ab1bp-2, -0x1.d78d79b4ad767p-56,
	  0x1.b115a57b5ab13p-2,	 -0x1.aa8c45be0aa2ep-57,
	  -0x1.0d58ec437efd7p+0, -0x1.994f00a15e85p-58,
	  0x1.cb1742e229f23p+1,	 -0x1.8000471d54399p-53,
	  -0x1.d99a5edf7b946p+3, -0x1.af76ed7e35cdep-52,
	  0x1.0a8b71058eb28p+6,	 -0x1.2d88289da5bfcp+8,
	  0x1.43ddf24168edbp+10, -0x1.3e9dfc38b6d1ap+12,
	  0x1.18d4df97ab3dfp+14, -0x1.b550fc62dcab5p+15,
	  0x1.29cb71f116ed1p+17, -0x1.5fc9cc4e854e3p+18,
	  0x1.65915fd0567b1p+19, -0x1.35eb5fca0e46dp+20,
	  0x1.c5261ecc0d789p+20, -0x1.138932dc4eafcp+21,
	  0x1.117d4eb18facdp+21, -0x1.af96163e35ecap+20,
	  0x1.0454a3a63c766p+20, -0x1.c2ebc1d39b44ap+18,
	  0x1.f3327698e0e6bp+16, -0x1.094febc3dff35p+14 }, /* asympt_acc9.sollya:
							      [0x1.0bp-1,0x1.2b81f34bfce36p-1],
							      degree 47, relerr
							      <= 2^-117.075 */
      };

/* assume 0x1.b59ffb450828cp+0 < x < 0x1.b39dc41e48bfdp+4
   thus 1.70 < x < 27.3 */
static double
erfc_asympt_accurate (double x)
{
  // the following table is sorted by ascending values of the 1st entry
  static const double exceptions[22][3] = {
    {0x1.b8940788b825dp+0, 0x1.e97eaf1080bffp-7, 0x1.38836346525eap-107},
    {0x1.bb1ef58eda44p+0, 0x1.d6d259cb81801p-7, 0x1.387cbdb500a5ep-114},
    {0x1.d1e19af184c6ep+0, 0x1.49bf1937979edp-7, 0x1.c1b55587c1381p-112},
    {0x1.0ca37ce17afa6p+1, 0x1.88cc1284157f5p-9, 0x1.6fd53d489a2c1p-120},
    {0x1.1cb6b0f91a4ccp+1, 0x1.b2639c7a46899p-10, 0x1.f8111f4b0ab0fp-114},
    {0x1.37994c710a4d2p+1, 0x1.2df150139d8eap-11, 0x1.d46afcc14563p-114},
    {0x1.76957728f1f31p+1, 0x1.251dea1afe1adp-15, -0x1.155995aa5bae6p-119},
    {0x1.7fa5439251a75p+1, 0x1.7923ec231992dp-16, -0x1.74f633ea13fecp-121},
    {0x1.8966f65d9a1a1p+1, 0x1.d01a8497b522dp-17, 0x1.c935c97d5bfe4p-121},
    {0x1.16ffd71e2d8c6p+2, 0x1.834e2899c5506p-31, -0x1.b7516e4549528p-135},
    {0x1.651c78cec84f6p+2, 0x1.af9d1df8b5e8p-49, -0x1.fe39e86292256p-153},
    {0x1.9f4a466c51bb5p+2, 0x1.a3d059f3770b9p-65, 0x1.3f16dde099b74p-167},
    {0x1.a8fb2b7ef1f8p+2, 0x1.c1b9f4cda246bp-68, 0x1.6fce2e1cae1a5p-171},
    {0x1.ef72633933d36p+2, 0x1.aee3861d8657ep-91, -0x1.00ca47f3ced2ap-194},
    {0x1.164f857d1749cp+3, 0x1.e35102e39e989p-114, 0x1.32b4c82bbb9e3p-219},
    {0x1.4a42b163f7a7dp+3, 0x1.183d60a1f7e3cp-158, -0x1.fffffffffffffp-212},
    {0x1.77d0f07e113dcp+3, 0x1.8b93f18b3cdc6p-204, 0x1.6ffd370fba52ap-307},
    {0x1.a631d4bc7f56bp+3, 0x1.3f07281bb43aep-256, -0x1p-309},
    {0x1.1b2588f5d972ep+4, 0x1.2923609150ffp-457, -0x1.ffffffffffffdp-511},
    {0x1.1d41cb671cad3p+4, 0x1.5c4d8d179be8cp-464, -0x1.fffffffffffffp-518},
    {0x1.391f434b53d18p+4, 0x1.44e8c50fa25e9p-558, -0x1.15aa7b49bc597p-662},
    {0x1.48de452fb1a15p+4, 0x1.3c2a1264045adp-615, 0x1.fffffffffffffp-669},
  };
  for (int i = 0; i < 22; i++)
    if (x == exceptions[i][0])
      return exceptions[i][1] + exceptions[i][2];

  /* subnormal exceptions */
  if (x == 0x1.a8f7bfbd15495p+4)
    return fma (0x1p-1074, -0.25, 0x1.99ef5883f656cp-1024);

  double h, l;
  /* first approximate exp(-x^2) */
  double eh, el, uh, ul;
  a_mul (&uh, &ul, x, x);
  int e;
  exp_accurate (&eh, &el, &e, -uh, -ul);
  /* eh+el approximates exp(-x^2), where 2.92 < x^2 < 742 */

  /* compute 1/x as double-double */
  double yh, yl;
  yh = 1.0 / x;
  /* Newton's iteration for 1/x is y -> y + y*(1-x*y) */
  yl = yh * fma (-x, yh, 1.0);
  // yh+yl approximates 1/x
  static const double threshold[]
      = { 0x1.45p-4, 0x1.e0p-4, 0x1.3fp-3, 0x1.95p-3, 0x1.f5p-3,
	  0x1.31p-2, 0x1.71p-2, 0x1.bcp-2, 0x1.0bp-1, 0x1.3p-1 };
  int i;
  for (i = 0; yh > threshold[i]; i++)
    ;
  // 0 <= i <= 9
  const double *p = Tacc[i];
  /* now evaluate p(yh + yl) */
  a_mul (&uh, &ul, yh, yh);
  ul = fma (2.0 * yh, yl, ul);
  /* uh+ul approximates 1/x^2 */
  double zh, zl;
  /* the polynomial p has degree 29+2i, and its coefficient of largest
     degree is p[14+6+i] */
  zh = p[14 + 6 + i];
  zl = 0;
  for (int j = 27 + 2 * i; j >= 13; j -= 2)
    {
      /* degree j: (zh+zl)*(uh+ul)+p[(j-1)/2+6]] */
      a_mul (&h, &l, zh, uh);
      l = fma (zh, ul, l);
      l = fma (zl, uh, l);
      two_sum (&zh, &zl, p[(j - 1) / 2 + 6], h);
      zl += l;
    }
  for (int j = 11; j >= 1; j -= 2)
    /* degree j: (zh+zl)*(uh+ul)+p[j-1]+p[j] */
    {
      a_mul (&h, &l, zh, uh);
      l = fma (zh, ul, l);
      l = fma (zl, uh, l);
      two_sum (&zh, &zl, p[j - 1], h);
      zl += l + p[j];
    }
  /* multiply by yh+yl */
  a_mul (&uh, &ul, zh, yh);
  ul = fma (zh, yl, ul);
  ul = fma (zl, yh, ul);
  /* now uh+ul approximates p(1/x), i.e., erfc(x)*exp(x^2) */
  /* now multiply (uh+ul)*(eh+el), after normalizing uh+ul to reduce the
     number of exceptional cases */
  fast_two_sum (&uh, &ul, uh, ul);
  a_mul (&h, &l, uh, eh);
  l = fma (uh, el, l);
  l = fma (ul, eh, l);
  /* multiply by 2^e */
  double res = __ldexp (h + l, e);
  if (res < 0x1p-1022)
    {
      /* for erfc(x) in the subnormal range, we have to perform a special
	 rounding */
      double corr = h - __ldexp (res, -e);
      corr += l;
      /* add corr*2^e */
      res += __ldexp (corr, e);
    }
  return res;
}

static double
cr_erfc_accurate (double x)
{
  double h, l, t;
  if (x < 0)
    {
      static const double exceptions[][3] = {
	{ -0x1.c5bf891b4ef6bp-54, 0x1.0000000000001p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.fe777a3eb8d58p-51, 0x1.0000000000005p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.d3ed8564296e6p-49, 0x1.0000000000011p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.29c5b1f9ebd1ep-49, 0x1.000000000000bp+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.e9327fd17121fp-48, 0x1.0000000000023p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.941e961c5253bp-48, 0x1.000000000001dp+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.3f0aac6733857p-48, 0x1.0000000000017p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.dc87cb831c792p-37, 0x1.0000000010cdbp+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.b71f3a10691d9p-37, 0x1.000000000f7bfp+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.cea935ba73f4cp-32, 0x1.000000020a0e9p+0,
	  0x1.fffffffffffffp-54 },
	{ -0x1.abaf3b436049ep-25, 0x1.000000f14b8e3p+0,
	  0x1.fffffffffffffp-54 },
	{ -0x1.d4c18d3599cbep-19, 0x1.0000421de806fp+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.607b67e03b83bp-15, 0x1.00031b7780fe5p+0,
	  -0x1.fffffffffffffp-54 },
	{ -0x1.4a943c917ed26p-12, 0x1.0017504bc67c1p+0,
	  0x1.fffffffffffffp-54 },
	{ -0x1.106c2cd5ea6c3p-9, 0x1.0099b29fab8cfp+0, 0x1.fffffffffffffp-54 },
	{ -0x1.d4af8adb90116p-4, 0x1.20e8e5d1e73ffp+0, 0x1.ffffffffffffep-54 },
	{ -0x1.f9a4a209ca0e4p+0, 0x1.feaa166e384c9p+0, 0x1.ffffffffffffep-54 },
      };
      for (int i = 0; i < 17; i++)
	if (x == exceptions[i][0])
	  return exceptions[i][1] + exceptions[i][2];
      cr_erf_accurate (&h, &l, -x);
      fast_two_sum (&h, &t, 1.0, h);
      l = t + l;
      return h + l;
    }
  else if (x <= 0x1.b59ffb450828cp+0) // erfc(x) >= 2^-6
    {
      static const double exceptions[][3] = {
	{ 0x1.c5bf891b4ef6bp-55, 0x1.fffffffffffffp-1, 0x1.ffffffffffffep-55 },
	{ 0x1.fe777a3eb8d58p-52, 0x1.ffffffffffffbp-1, 0x1.ffffffffffffep-55 },
	{ 0x1.29c5b1f9ebd1ep-50, 0x1.ffffffffffff5p-1, 0x1.ffffffffffffep-55 },
	{ 0x1.d3ed8564296e6p-50, 0x1.fffffffffffefp-1, 0x1.ffffffffffffep-55 },
	{ 0x1.3f0aac6733857p-49, 0x1.fffffffffffe9p-1, 0x1.fffffffffffffp-55 },
	{ 0x1.941e961c5253bp-49, 0x1.fffffffffffe3p-1, 0x1.fffffffffffffp-55 },
	{ 0x1.e9327fd17121fp-49, 0x1.fffffffffffddp-1, 0x1.fffffffffffffp-55 },
	{ 0x1.b71f3a10691d9p-38, 0x1.fffffffff0841p-1,
	  -0x1.ffffffffffffep-55 },
	{ 0x1.dc87cb831c792p-38, 0x1.ffffffffef325p-1, 0x1.fffffffffffffp-55 },
	{ 0x1.52b18fe8fbad1p-36, 0x1.ffffffffd03a7p-1,
	  -0x1.ffffffffffffep-55 },
	{ 0x1.cf0ed5959b276p-28, 0x1.ffffffbeafd79p-1,
	  -0x1.fffffffffffffp-55 },
	{ 0x1.6af79a30bbf3cp-27, 0x1.ffffff999bdd3p-1, 0x1.fffffffffffffp-55 },
	{ 0x1.dd48360dd7824p-26, 0x1.fffffef2b8f0dp-1, 0x1.fffffffffffffp-55 },
	{ 0x1.40757fba82477p-24, 0x1.fffffd2ccd30dp-1,
	  -0x1.fffffffffffffp-55 },
	{ 0x1.8bc46e0d85524p-9, 0x1.fe416cffa803dp-1, -0x1.fffffffffffffp-55 },
	{ 0x1.d4af8adb90116p-4, 0x1.be2e345c31801p-1, 0x1.ea7fa18173ab8p-106 },
	{ 0x1.3229110bf9d66p-2, 0x1.5847806aef94bp-1, 0x1.fffffffffffffp-55 },
	{ 0x1.a93059aabe4bcp-1, 0x1.ebfa1229d0dcep-3, 0x1.99b4b80fd9415p-107 },
	{ 0x1.bc466342a2296p-1, 0x1.c2153a8529156p-3,
	  -0x1.088fb70829f71p-107 },
	{ 0x1.eef77c6caae18p-1, 0x1.5f61e96f60783p-3, -0x1.ffffffffffffdp-57 },
	{ 0x1.fd5d9d8c9ef66p-1, 0x1.468ba052c2fep-3, -0x1.03fa708a553b3p-105 },
	{ 0x1.23c67ff8b8a0ap+0, 0x1.b640be5efa2e3p-4, -0x1.b9a66b42d916p-107 },
	{ 0x1.3a013a3c71e92p+0, 0x1.5329743c2c15cp-4, 0x1.7766eb5263404p-109 },
	{ 0x1.589bbd3ae5489p+0, 0x1.d2847b1409248p-5, 0x1.57b4a906bc605p-107 },
	{ 0x1.757fe152a87c9p+0, 0x1.402acce35d833p-5, 0x1.d38ccfa0c1d6bp-108 },
	{ 0x1.7749479e598d3p+0, 0x1.3891521afe3e3p-5, 0x1.8b8423c82ad72p-108 },
	{ 0x1.9a3a5f5f28f3p+0, 0x1.80040c25bab9p-6, -0x1.a8686704bdb14p-108 },
	{ 0x1.a44869b34ea46p+0, 0x1.4bb609c105c2dp-6,
	  -0x1.9182dae766fc4p-109 },
	{ 0x1.af08ae30c9364p+0, 0x1.1ac47852096cbp-6, -0x1.fffffffffffdap-60 },
      };
      for (int i = 0; i < 29; i++)
	if (x == exceptions[i][0])
	  return exceptions[i][1] + exceptions[i][2];
      cr_erf_accurate (&h, &l, x);
      fast_two_sum (&h, &t, 1.0, -h);
      l = t - l;
      return h + l;
    }
  // now 0x1.b59ffb450828cp+0 < x < 0x1.b39dc41e48bfdp+4
  return erfc_asympt_accurate (x);
}

double
__erfc (double x)
{
  uint64_t t = asuint64 (x);
  uint64_t at = t & UINT64_C(0x7fffffffffffffff);

  if (t >= UINT64_C(0x8000000000000000)) // x = -NaN or x <= 0 (excluding +0)
    {
      // for x <= -0x1.7744f8f74e94bp2, erfc(x) rounds to 2 (to nearest)
      // x = NaN or x <= -0x1.7744f8f74e94bp2
      if (t >= UINT64_C(0xc017744f8f74e94b))
	{
	  if (t >= UINT64_C(0xfff0000000000000))
	    { // -Inf or NaN
	      if (t == UINT64_C(0xfff0000000000000))
		return 2.0; // -Inf
	      return x + x; // NaN
	    }
	  return 2.0 - 0x1p-54; // rounds to 2 or below(2)
	}

      // for -0x1.c5bf891b4ef6ap-54 <= x <= 0, erfc(x) rounds to 1 (to nearest)
      if (-0x1.c5bf891b4ef6ap-54 <= x)
	return fma (-x, 0x1p-54, 1.0);
    }

  else // x = +NaN or x >= 0 (excluding -0)
    {
      // for x >= 0x1.b39dc41e48bfdp+4, erfc(x) < 2^-1075: rounds to 0 or
      // 2^-1074
      // x = NaN or x >= 0x1.b39dc41e48bfdp+4
      if (at >= UINT64_C(0x403b39dc41e48bfd))
	{
	  if (at >= UINT64_C(0x7ff0000000000000))
	    { // +Inf or NaN
	      if (at == UINT64_C(0x7ff0000000000000))
		return 0.0; // +Inf
	      return x + x; // NaN
	    }
	  return __math_erange (0x1p-1074 * 0.25); // 0 or 2^-1074 wrt rounding
	}

      // for 0 <= x <= 0x1.c5bf891b4ef6ap-55, erfc(x) rounds to 1 (to nearest)
      if (x <= 0x1.c5bf891b4ef6ap-55)
	return fma (-x, 0x1p-54, 1.0);
    }

  /* now -0x1.7744f8f74e94bp+2 < x < -0x1.c5bf891b4ef6ap-54
     or 0x1.c5bf891b4ef6ap-55 < x < 0x1.b39dc41e48bfdp+4 */
  double h, l, err;
  err = cr_erfc_fast (&h, &l, x);

  double left = h + (l - err);
  double right = h + (l + err);
  if (left == right)
    return left;

  return cr_erfc_accurate (x);
}
libm_alias_double (__erfc, erfc)
