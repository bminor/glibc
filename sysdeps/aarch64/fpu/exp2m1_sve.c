/* Double-precision (SVE) exp2m1 function

   Copyright (C) 2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include "sv_math.h"

/* Value of |x| above which scale overflows without special treatment.  */
#define SpecialBound 1022.0

/* Value of n above which scale overflows even with special treatment.  */
#define ScaleBound 1280.0

/* 87/256, value of x under which table lookup is used for 2^x-1.  */
#define TableBound 0x1.5bfffffffffffp-2

/* Number of bits for each value in the table.  */
#define N (1 << V_EXP_TABLE_BITS)
#define IndexMask (N - 1)

static const struct data
{
  double shift, rnd2zero;
  double c1, c3, c5;
  double c0, c2, c4;
  uint64_t sm1_tbl_mask, sm1_tbl_off;
  uint64_t scalem1[88];
} data = {
  /* Generated using fpminimax.  */
  .c0 = 0x1.62e42fefa39efp-1,
  .c1 = 0x1.ebfbdff82c58ep-3,
  .c2 = 0x1.c6b08d707e662p-5,
  .c3 = 0x1.3b2ab6fc45f33p-7,
  .c4 = 0x1.5d86c0ff8618dp-10,
  .c5 = 0x1.4301374d5d2f5p-13,
  .shift = 0x1.8p52 / N,
  .sm1_tbl_mask = 0x3f,
  .sm1_tbl_off = 24,
  .rnd2zero = -0x1p-8,
  /* Table containing 2^x - 1, for 2^x values close to 1.
     The table holds values of 2^(i/128) - 1, computed in
     arbitrary precision.
     The 1st half contains values associated to i=0..43.
     The 2nd half contains values associated to i=-44..-1.  */
  .scalem1= {
    0x0000000000000000, 0x3f763da9fb33356e, 0x3f864d1f3bc03077,
    0x3f90c57a1b9fe12f, 0x3f966c34c5615d0f, 0x3f9c1aca777db772,
    0x3fa0e8a30eb37901, 0x3fa3c7d958de7069, 0x3fa6ab0d9f3121ec,
    0x3fa992456e48fee8, 0x3fac7d865a7a3440, 0x3faf6cd5ffda635e,
    0x3fb1301d0125b50a, 0x3fb2abdc06c31cc0, 0x3fb429aaea92ddfb,
    0x3fb5a98c8a58e512, 0x3fb72b83c7d517ae, 0x3fb8af9388c8de9c,
    0x3fba35beb6fcb754, 0x3fbbbe084045cd3a, 0x3fbd4873168b9aa8,
    0x3fbed5022fcd91cc, 0x3fc031dc431466b2, 0x3fc0fa4c8beee4b1,
    0x3fc1c3d373ab11c3, 0x3fc28e727d9531fa, 0x3fc35a2b2f13e6e9,
    0x3fc426ff0fab1c05, 0x3fc4f4efa8fef709, 0x3fc5c3fe86d6cc80,
    0x3fc6942d3720185a, 0x3fc7657d49f17ab1, 0x3fc837f0518db8a9,
    0x3fc90b87e266c18a, 0x3fc9e0459320b7fa, 0x3fcab62afc94ff86,
    0x3fcb8d39b9d54e55, 0x3fcc6573682ec32c, 0x3fcd3ed9a72cffb7,
    0x3fce196e189d4724, 0x3fcef5326091a112, 0x3fcfd228256400dd,
    0x3fd0582887dcb8a8, 0x3fd0c7d76542a25b, 0xbfcb23213cc8e86c,
    0xbfca96ecd0deb7c4, 0xbfca09f58086c6c2, 0xbfc97c3a3cd7e119,
    0xbfc8edb9f5703dc0, 0xbfc85e7398737374, 0xbfc7ce6612886a6d,
    0xbfc73d904ed74b33, 0xbfc6abf137076a8e, 0xbfc61987b33d329e,
    0xbfc58652aa180903, 0xbfc4f25100b03219, 0xbfc45d819a94b14b,
    0xbfc3c7e359c9266a, 0xbfc331751ec3a814, 0xbfc29a35c86a9b1a,
    0xbfc20224341286e4, 0xbfc1693f3d7be6da, 0xbfc0cf85bed0f8b7,
    0xbfc034f690a387de, 0xbfbf332113d56b1f, 0xbfbdfaa500017c2d,
    0xbfbcc0768d4175a6, 0xbfbb84935fc8c257, 0xbfba46f918837cb7,
    0xbfb907a55511e032, 0xbfb7c695afc3b424, 0xbfb683c7bf93b074,
    0xbfb53f391822dbc7, 0xbfb3f8e749b3e342, 0xbfb2b0cfe1266bd4,
    0xbfb166f067f25cfe, 0xbfb01b466423250a, 0xbfad9b9eb0a5ed76,
    0xbfaafd11874c009e, 0xbfa85ae0438b37cb, 0xbfa5b505d5b6f268,
    0xbfa30b7d271980f7, 0xbfa05e4119ea5d89, 0xbf9b5a991288ad16,
    0xbf95f134923757f3, 0xbf90804a4c683d8f, 0xbf860f9f985bc9f4,
    0xbf761eea3847077b,
  }
};

static inline svuint64_t
lookup_sbits (svbool_t pg, svuint64_t indices)
{
  /* Mask indices to valid range.  */
  svuint64_t masked = svand_z (pg, indices, svdup_n_u64 (IndexMask));
  return svld1_gather_index (pg, __v_exp_data, masked);
}

static inline svfloat64_t
lookup_sm1bits (svbool_t pg, svfloat64_t x, svuint64_t u, const struct data *d)
{
  /* Extract sign bit and use as offset into table.  */
  svbool_t signBit = svcmplt_f64 (pg, x, sv_f64 (d->rnd2zero));
  svuint64_t base_idx = svand_x (pg, u, d->sm1_tbl_mask);
  svuint64_t idx = svadd_m (signBit, base_idx, sv_u64 (d->sm1_tbl_off));

  /* Fall back to table lookup for 2^x - 1, when x is close to zero to
     avoid large errors.  */
  svuint64_t sm1 = svld1_gather_index (svptrue_b64 (), d->scalem1, idx);
  return svreinterpret_f64 (sm1);
}

#define SpecialOffset 0x6000000000000000 /* 0x1p513.  */
/* SpecialBias1 + SpecialBias1 = asuint(1.0).  */
#define SpecialBias1 0x7000000000000000 /* 0x1p769.  */
#define SpecialBias2 0x3010000000000000 /* 0x1p-254.  */

static inline svfloat64_t
special_case (svfloat64_t s, svfloat64_t y, svfloat64_t n,
	      const struct data *d)
{
  /* s=2^n may overflow, break it up into s=s1*s2,
     such that exp = s + s*y can be computed as s1*(s2+s2*y)
     and s1*s1 overflows only if n>0.  */
  svbool_t p_sign = svcmple (svptrue_b64 (), n, 0.0); /* n <= 0.  */
  svuint64_t b = svdup_u64_z (p_sign, SpecialOffset);

  /* Set s1 to generate overflow depending on sign of exponent n.  */
  svfloat64_t s1
      = svreinterpret_f64 (svsubr_x (svptrue_b64 (), b, SpecialBias1));
  /* Offset s to avoid overflow in final result if n is below threshold.  */
  svfloat64_t s2 = svreinterpret_f64 (svadd_x (
      svptrue_b64 (),
      svsub_x (svptrue_b64 (), svreinterpret_u64 (s), SpecialBias2), b));

  /* |n| > 1280 => 2^(n) overflows.  */
  svbool_t p_cmp = svacle (svptrue_b64 (), n, ScaleBound);

  svfloat64_t r1 = svmul_x (svptrue_b64 (), s1, s1);
  svfloat64_t r2 = svmla_x (svptrue_b64 (), s2, s2, y);
  svfloat64_t r0 = svmul_x (svptrue_b64 (), r2, s1);

  return svsub_x (svptrue_b64 (), svsel (p_cmp, r0, r1), 1.0);
}

/* Double-precision SVE exp2(x) - 1.
   Maximum error is 2.58 + 0.5 ULP.
   _ZGVsMxv_exp2m1(0x1.0284a345c99bfp-8) got 0x1.66df630cd2965p-9
					want 0x1.66df630cd2962p-9.  */
svfloat64_t SV_NAME_D1 (exp2m1) (svfloat64_t x, svbool_t pg)
{
  /* exp2(x) = 2^n (1 + poly(r))
     x = n + r, with r in [-1/2N, 1/2N].
     n is a floating point number, multiple of 1/N.  */
  const struct data *d = ptr_barrier (&data);
  svbool_t special = svacge (pg, x, SpecialBound);

  svfloat64_t z = svadd_x (pg, x, d->shift);
  svuint64_t u = svreinterpret_u64 (z);
  svfloat64_t n = svsub_x (pg, z, d->shift);

  svfloat64_t r = svsub_x (svptrue_b64 (), x, n);
  svfloat64_t r2 = svmul_x (svptrue_b64 (), r, r);

  /* Look up table to calculate 2^n.  */
  svuint64_t e = svlsl_x (pg, u, 52 - V_EXP_TABLE_BITS);
  svuint64_t scale_bits = lookup_sbits (pg, u);
  svfloat64_t scale = svreinterpret_f64_u64 (svadd_x (pg, scale_bits, e));

  /* Pairwise Horner scheme.  */
  svfloat64_t c35 = svld1rq (svptrue_b64 (), &d->c3);

  svfloat64_t p01 = svmla_x (pg, sv_f64 (d->c0), r, d->c1);
  svfloat64_t p23 = svmla_lane (sv_f64 (d->c2), r, c35, 0);
  svfloat64_t p45 = svmla_lane (sv_f64 (d->c4), r, c35, 1);

  svfloat64_t p25 = svmla_x (pg, p23, r2, p45);
  svfloat64_t p05 = svmla_x (pg, p01, r2, p25);
  svfloat64_t poly = svmul_x (pg, p05, r);

  /* Use table to gather scalem1 for small values of x.  */
  svbool_t is_small = svaclt (pg, x, TableBound);
  svfloat64_t scalem1 = svsub_x (pg, scale, sv_f64 (1.0));
  if (svptest_any (pg, is_small))
    scalem1 = svsel_f64 (is_small, lookup_sm1bits (pg, x, u, d), scalem1);

  /* Construct exp2m1 = (scale - 1) + scale * poly.  */
  svfloat64_t y = svmla_x (pg, scalem1, scale, poly);

  /* Fallback to special case for lanes with overflow.  */
  if (__glibc_unlikely (svptest_any (pg, special)))
    return svsel_f64 (special, special_case (scale, poly, n, d), y);

  return y;
}
