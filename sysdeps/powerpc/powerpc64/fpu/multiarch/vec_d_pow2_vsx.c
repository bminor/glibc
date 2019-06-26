/* Double-precision vector pow function.
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
/* Based on sysdeps/ieee754/dbl-64/e_pow.c which came from
   Szabolcs Nagy at ARM Ltd. */
#include <altivec.h>
#include <math.h>
#include <stdbool.h>
#include <math-barriers.h>

#include "math_config_dbl.h"

typedef vector long long unsigned v64u;

/*
Worst-case error: 0.54 ULP (~= ulperr_exp + 1024*Ln2*relerr_log*2^53)
relerr_log: 1.3 * 2^-68 (Relative error of log, 1.5 * 2^-68 without fma)
ulperr_exp: 0.509 ULP (ULP error of exp, 0.511 ULP without fma)
*/

#define T __pow_log_data.tab
#define A __pow_log_data.poly
#define Ln2hi __pow_log_data.ln2hi
#define Ln2lo __pow_log_data.ln2lo
#define N (1 << POW_LOG_TABLE_BITS)
#define OFF 0x3fe6955500000000

/* Compute y+TAIL = log(x) where the rounded result is y and TAIL has about
   additional 15 bits precision.  IX is the bit representation of x, but
   normalized in the subnormal range using the sign bit for the exponent.  */
struct two_v_doubles {
  vector double y;
  vector double tail;
};
static inline struct two_v_doubles
log_inline(v64u ix)
{
  /* double_t for better performance on targets with FLT_EVAL_METHOD==2.  */
  vector double z, r, y, kd, hi, t1, t2, lo, lo1, lo2, p;
  v64u iz, tmp, i;
  vector signed long long k;

  /* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - OFF;
  i = (tmp >> (52 - POW_LOG_TABLE_BITS)) % N;
  k = ((vector signed long long)tmp >> 52); /* arithmetic shift */
  iz = ix - (tmp & 0xfffULL << 52);
  z = vasdouble(iz);
  // we don't use __builtin_vectorconvert as we want to support old gcc
  kd[0] = (double)k[0];
  kd[1] = (double)k[1];

  /* log(x) = k*Ln2 + log(c) + log1p(z/c-1).  */
  vector double invc = {T[i[0]].invc, T[i[1]].invc};
  vector double logc = {T[i[0]].logc, T[i[1]].logc};
  vector double logctail = {T[i[0]].logctail, T[i[1]].logctail};

  /* Note: 1/c is j/N or j/N/2 where j is an integer in [N,2N) and
     |z/c - 1| < 1/N, so r = z/c - 1 is exactly representible.  */
  // This is so that the results are identical to the non-SIMD version
#if __FP_FAST_FMA
  vector double negone = {-1.0, -1.0};
  r = vec_madd(z, invc, negone);
#else
  vector double zhi = vasdouble((iz + (1ULL << 31)) & (-1ULL << 32));
  vector double zlo = z - zhi;
  vector double rhi = zhi * invc - 1.0;
  vector double rlo = zlo * invc;
  r = rhi + rlo;
#endif
  /* k*Ln2 + log(c) + r.  */
  t1 = kd * Ln2hi + logc;
  t2 = t1 + r;
  lo1 = kd * Ln2lo + logctail;
  lo2 = t1 - t2 + r;

  /* Evaluation is optimized assuming superscalar pipelined execution.  */
  vector double ar, ar2, ar3, lo3, lo4;
  ar = A[0] * r; /* A[0] = -0.5.  */
  ar2 = r * ar;
  ar3 = r * ar2;
  /* k*Ln2 + log(c) + r + A[0]*r*r.  */
#if __FP_FAST_FMA
  hi = t2 + ar2;
  lo3 = vec_madd(ar, r, -ar2);
  lo4 = t2 - hi + ar2;
#else
  vector double arhi = A[0] * rhi;
  vector double arhi2 = rhi * arhi;
  hi = t2 + arhi2;
  lo3 = rlo * (ar + arhi);
  lo4 = t2 - hi + arhi2;
#endif
  /* p = log1p(r) - r - A[0]*r*r.  */
  p = (ar3 * (A[1] + r * A[2] +
    ar2 * (A[3] + r * A[4] + ar2 * (A[5] + r * A[6]))));
  lo = lo1 + lo2 + lo3 + lo4 + p;
  y = hi + lo;
  struct two_v_doubles ret;
  ret.tail = hi - y + lo;
  ret.y = y;
  return ret;
}


/* Handle cases that may overflow or underflow when computing the result that
   is scale*(1+TMP) without intermediate rounding.  The bit representation of
   scale is in SBITS, however it has a computed exponent that may have
   overflown into the sign bit so that needs to be adjusted before using it as
   a double.  (int32_t)KI is the k used in the argument reduction and exponent
   adjustment of scale, positive k here means the result may overflow and
   negative k means the result may underflow.  */
static inline double
specialcase (double_t tmp, uint64_t sbits, uint64_t ki)
{
  double_t scale, y;

  if ((ki & 0x80000000) == 0)
    {
      /* k > 0, the exponent of scale might have overflowed by <= 460.  */
      sbits -= 1009ull << 52;
      scale = asdouble (sbits);
      y = 0x1p1009 * (scale + scale * tmp);
      return y;
    }
  /* k < 0, need special care in the subnormal range.  */
  sbits += 1022ull << 52;
  /* Note: sbits is signed scale.  */
  scale = asdouble (sbits);
  y = scale + scale * tmp;
  if (fabs (y) < 1.0)
    {
      /* Round y to the right precision before scaling it into the subnormal
	 range to avoid double rounding that can cause 0.5+E/2 ulp error where
	 E is the worst-case ulp error outside the subnormal range.  So this
	 is only useful if the goal is better than 1 ulp worst-case error.  */
      double_t hi, lo, one = 1.0;
      if (y < 0.0)
	one = -1.0;
      lo = scale - y + scale * tmp;
      hi = one + y;
      lo = one - hi + y + lo;
      y = math_narrow_eval (hi + lo) - one;
      /* Fix the sign of 0.  */
      if (y == 0.0)
	y = asdouble (sbits & 0x8000000000000000);
      /* The underflow exception needs to be signaled explicitly.  */
      math_force_eval (math_opt_barrier (0x1p-1022) * 0x1p-1022);
    }
  y = 0x1p-1022 * y;
  return y;
}

#undef N
#undef T
#define N (1 << EXP_TABLE_BITS)
#define InvLn2N __exp_data.invln2N
#define NegLn2hiN __exp_data.negln2hiN
#define NegLn2loN __exp_data.negln2loN
#define Shift __exp_data.shift
#define T __exp_data.tab
#define C2 __exp_data.poly[5 - EXP_POLY_ORDER]
#define C3 __exp_data.poly[6 - EXP_POLY_ORDER]
#define C4 __exp_data.poly[7 - EXP_POLY_ORDER]
#define C5 __exp_data.poly[8 - EXP_POLY_ORDER]
#define C6 __exp_data.poly[9 - EXP_POLY_ORDER]

#define SIGN_BIAS (0x800 << EXP_TABLE_BITS)

/* Computes sign*exp(x+xtail) where |xtail| < 2^-8/N and |xtail| <= |x|.
   The sign_bias argument is SIGN_BIAS or 0 and sets the sign to -1 or 1.  */
static inline vector double
exp_inline(vector double x, vector double xtail, v64u sign_bias)
{
  v64u zero = {0, 0};
  v64u ki, idx, top, sbits, res, res_mask = zero;
  /* double_t for better performance on targets with FLT_EVAL_METHOD==2.  */
  vector double kd, z, r, r2, scale, tmp;

  v64u abstop = vasuint64 (x) & 0x7ff0000000000000;
  v64u comp_one = (v64u) vec_cmpge ((abstop - asuint64 (0x1p-54))
				    & 0xfff0000000000000,
    zero + ((asuint64 (512.0) & 0xfff0000000000000) - (asuint64 (0x1p-54)
						       & 0xfff0000000000000)));
  if (!vec_all_eq (comp_one, zero))
    {
      v64u comp_tiny = (v64u)vec_cmpge (zero + asuint64(0x1p-54), abstop);
      if (!vec_all_eq (comp_tiny, zero))
	{
	  /* Avoid spurious underflow for tiny x.  */
	  /* Note: 0 is common input.  */
	  vector double one = WANT_ROUNDING ? 1.0 + x + vasdouble(zero)
	    : 1.0 + vasdouble(zero);
	  res = vasuint64 (vec_sel (-one, one,
				    (v64u) (vec_cmpgt (sign_bias, zero + 1))));
	}
      v64u comp_xflow =
	(v64u) vec_cmpge (abstop, zero + (asuint64(1024.0)
					  & 0xfff0000000000000));
      comp_xflow &= ~res_mask;
      if (!vec_all_eq (comp_xflow, zero))
	{
	  vector double inf = {INFINITY, INFINITY};
	  /* Note: inf and nan are already handled.  */
	  v64u is_uflow = (v64u) vec_cmpne (vasuint64 (x) >> 63, zero);
	  if (!vec_all_eq (is_uflow, zero))
	      (void)__math_uflow (0/* this only determines output */);
	  if (!vec_all_eq (comp_xflow & ~is_uflow, zero))
	      (void)__math_oflow (0/* this only determines output */);
	  res = vasuint64(inf);
	  res = vec_sel (res, vasuint64 (-vasdouble(res)),
			 ~vec_cmpeq(sign_bias, zero));
	  res = vec_sel (res, zero, is_uflow);
	  res = vec_sel (res, asuint64 (-0.0) + zero,
			 is_uflow & ~vec_cmpeq(sign_bias, zero));
	  res_mask |= comp_xflow;
	}
      if (vec_all_eq (res_mask, ~zero))
	  return vasdouble(res);
      /* Large x is special cased below.  */
      abstop = zero;
    }

  /* exp(x) = 2^(k/N) * exp(r), with exp(r) in [2^(-1/2N),2^(1/2N)].  */
  /* x = ln2/N*k + r, with int k and r in [-ln2/2N, ln2/2N].  */
  z = InvLn2N * x;
  /* z - kd is in [-0.5-2^-16, 0.5] in all rounding modes.  */
  kd = (vector double)(z + Shift);
  ki = vasuint64(kd);
  kd -= Shift;
  r = x + kd * NegLn2hiN + kd * NegLn2loN;
  /* The code assumes 2^-200 < |xtail| < 2^-8/N.  */
  r += xtail;
  /* 2^(k/N) ~= scale * (1 + tail).  */
  idx = 2 * (ki % N);
  top = (ki + sign_bias) << (52 - EXP_TABLE_BITS);
  // we can't use __builtin_vectorconvert as we want to support old gcc
  vector double tail = {asdouble(T[idx[0]]), asdouble(T[idx[1]])};
  /* This is only a valid scale when -1023*N < k < 1024*N.  */
  v64u Tadd = {T[idx[0] + 1], T[idx[1] + 1]};
  sbits = Tadd + top;
  /* exp(x) = 2^(k/N) * exp(r) ~= scale + scale * (tail + exp(r) - 1).  */
  /* Evaluation is optimized assuming superscalar pipelined execution.  */
  r2 = r * r;
  /* Without fma the worst case error is 0.25/N ulp larger.  */
  /* Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.  */
  tmp = tail + r + r2 * (C2 + r * C3) + r2 * r2 * (C4 + r * C5);
  v64u is_abstop_zero = (v64u)vec_cmpeq (abstop, zero);
  is_abstop_zero &= ~res_mask;
  if (!vec_all_eq(is_abstop_zero, zero))
    {
      for (int i=0;i<2;i++)
	{
	  if (is_abstop_zero[i] == 0)
	      continue;
	  res[i] = asuint64 (specialcase (tmp[i], sbits[i], ki[i]));
	  res_mask |= is_abstop_zero;
	}
      if (vec_all_eq (res_mask, ~zero))
	  return vasdouble(res);
    }
  scale = vasdouble(sbits);
  /* Note: tmp == 0 or |tmp| > 2^-200 and scale > 2^-739, so there
     is no spurious underflow here even without fma.  */
  return vec_sel(scale + scale * tmp, vasdouble(res), res_mask);
}

/* Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
   the bit representation of a non-zero finite floating-point value.  */
static inline int checkint(uint64_t iy)
{
  int e = iy >> 52 & 0x7ff;
  if (e < 0x3ff)
      return 0;
  if (e > 0x3ff + 52)
      return 2;
  if (iy & ((1ULL << (0x3ff + 52 - e)) - 1))
      return 0;
  if (iy & (1ULL << (0x3ff + 52 - e)))
      return 1;
  return 2;
}

/* Returns 1 if input is the bit representation of 0, infinity or nan.  */
static inline int
zeroinfnan (uint64_t i)
{
  return 2 * i - 1 >= 2 * asuint64 (INFINITY) - 1;
}

/* Top 12 bits of a double (sign and exponent bits).  */
static inline uint32_t
top12 (double x)
{
  return asuint64 (x) >> 52;
}

static double
mainspecialcase (double x, double y, bool *is_subnormal, uint64_t *sign_bias)
{
  uint64_t iy = asuint64 (y);
  uint64_t ix = asuint64 (x);
  uint32_t topx = top12 (x);
  uint32_t topy = top12 (y);
      /* Note: if |y| > 1075 * ln2 * 2^53 ~= 0x1.749p62 then pow(x,y) = inf/0
	 and if |y| < 2^-54 / 1075 ~= 0x1.e7b6p-65 then pow(x,y) = +-1.  */
      /* Special cases: (x < 0x1p-126 or inf or nan) or
	 (|y| < 0x1p-65 or |y| >= 0x1p63 or nan).  */
  if (__glibc_unlikely (zeroinfnan (iy)))
    {
      if (2 * iy == 0)
        return issignaling_inline (x) ? x + y : 1.0;
      if (ix == asuint64 (1.0))
        return issignaling_inline (y) ? x + y : 1.0;
      if (2 * ix > 2 * asuint64 (INFINITY)
          || 2 * iy > 2 * asuint64 (INFINITY))
        return x + y;
      if (2 * ix == 2 * asuint64 (1.0))
        return 1.0;
      if ((2 * ix < 2 * asuint64 (1.0)) == !(iy >> 63))
        return 0.0; /* |x|<1 && y==inf or |x|>1 && y==-inf.  */
      return y * y;
    }
  if (__glibc_unlikely (zeroinfnan (ix)))
    {
      double_t x2 = x * x;
      if (ix >> 63 && checkint (iy) == 1)
        {
          x2 = -x2;
          *sign_bias = 1;
        }
      if (WANT_ERRNO && 2 * ix == 0 && iy >> 63)
        return __math_divzero (*sign_bias);
      /* Without the barrier some versions of clang hoist the 1/x2 and
         thus division by zero exception can be signaled spuriously.  */
      return iy >> 63 ? math_opt_barrier (1 / x2) : x2;
    }
      /* Here x and y are non-zero finite.  */
  if (ix >> 63)
    {
      /* Finite x < 0.  */
      int yint = checkint (iy);
      if (yint == 0)
        return __math_invalid (x);
      if (yint == 1)
        *sign_bias = SIGN_BIAS;
      ix &= 0x7fffffffffffffff;
      topx &= 0x7ff;
    }
  if ((topy & 0x7ff) - 0x3be >= 0x43e - 0x3be)
    {
      /* Note: sign_bias == 0 here because y is not odd.  */
      if (ix == asuint64 (1.0))
        return 1.0;
      if ((topy & 0x7ff) < 0x3be)
        {
          /* |y| < 2^-65, x^y ~= 1 + y*log(x).  */
          if (WANT_ROUNDING)
	      return ix > asuint64 (1.0) ? 1.0 + y : 1.0 - y;
          else
	      return 1.0;
        }
      return (ix > asuint64 (1.0)) == (topy < 0x800) ? __math_oflow (0)
						     : __math_uflow (0);
    }
  if (topx == 0)
    {
      /* Normalize subnormal x so exponent becomes negative.  */
      ix = asuint64 (x * 0x1p52);
      ix &= 0x7fffffffffffffff;
      ix -= 52ULL << 52;
    }
  *is_subnormal = true;
  return asdouble(ix);
}

vector double
_ZGVbN2vv_pow (vector double x, vector double y)
{
  v64u zero = {0, 0};
  v64u sign_bias = zero;
  v64u res, res_mask = zero;

  v64u is_special1 =
    (v64u) vec_cmpge (vasuint64(x) - 0x0010000000000000,
		      zero + 0x7ff0000000000000 - 0x0010000000000000);
  v64u is_special2 =
    (v64u) vec_cmpge ((vasuint64(y) & 0x7ff0000000000000) - 0x3be0000000000000,
		      zero + 0x43e0000000000000 - 0x3be0000000000000);
  v64u is_special = is_special1 | is_special2;
  if (!vec_all_eq (is_special, zero))
    {
      for (int i=0;i<2;i++)
	{
	  if (is_special[i] == 0)
	      continue;
	  bool is_subnormal = false;
	  double r = mainspecialcase(x[i], y[i], &is_subnormal, &sign_bias[i]);
	  if (!is_subnormal)
	    {
	      res[i] = asuint64(r);
	      res_mask[i] = 0xffffffffffffffff;
	    }
	      else
	    {
	      x[i] = r;
	    }
	}
      if (!vec_any_eq (res_mask, zero))
	  return vasdouble(res);
  }

  struct two_v_doubles logres = log_inline (vasuint64 (x));
  vector double ehi, elo;
#if __FP_FAST_FMA
  ehi = y * logres.y;
  elo = y * logres.tail + vec_madd (y, logres.y, -ehi);
#else
  vector double yhi = vasdouble(vasuint64(y) & -1ULL << 27);
  vector double ylo = y - yhi;
  vector double lhi = vasdouble(vasuint64(logres.y) & -1ULL << 27);
  vector double llo = logres.y - lhi + logres.tail;
  ehi = yhi * lhi;
  elo = ylo * lhi + y * llo; /* |elo| < |ehi| * 2^-25.  */
#endif
  return vec_sel (exp_inline (ehi, elo, sign_bias), vasdouble (res), res_mask);
}
