/* Single-precision vector pow function.
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

#include <math.h>
#include <math-barriers.h>
#include <stdint.h>
#include "math_config_flt.h"

typedef vector long long unsigned v64u;
/*
POWF_LOG2_POLY_ORDER = 5
EXP2F_TABLE_BITS = 5

ULP error: 0.82 (~ 0.5 + relerr*2^24)
relerr: 1.27 * 2^-26 (Relative error ~= 128*Ln2*relerr_log2 + relerr_exp2)
relerr_log2: 1.83 * 2^-33 (Relative error of logx.)
relerr_exp2: 1.69 * 2^-34 (Relative error of exp2(ylogx).)
*/

#define N (1 << POWF_LOG2_TABLE_BITS)
#define T __powf_log2_data.tab
#define A __powf_log2_data.poly
#define OFF 0x3f330000


/* Subnormal input is normalized so ix has negative biased exponent.
   Output is multiplied by N (POWF_SCALE) if TOINT_INTRINICS is set.  */
struct two_v_doubles {
  vector double l;
  vector double r;
};

static inline struct two_v_doubles
log2_inline (vector unsigned ix)
{
  vector float z;
  vector double rl, rr, r2l, r2r, r4l, r4r, pl, pr, ql, qr, yl, yr, y0l, y0r;
  vector unsigned iz, top, tmp;
  vector signed k, i;
  struct two_v_doubles ret;

  /* x = 2^k z; where z is in range [OFF,2*OFF] and exact.
     The range is split into N subintervals.
     The ith subinterval contains z and c is near its center.  */
  tmp = ix - OFF;
  i = (vector signed)(tmp >> (23 - POWF_LOG2_TABLE_BITS)) % N;
  top = tmp & 0xff800000;
  iz = ix - top;
  k = (vector signed)top >> (23 - POWF_SCALE_BITS); /* arithmetic shift */
  vector double invcl = {T[i[0]].invc, T[i[1]].invc};
  vector double invcr = {T[i[2]].invc, T[i[3]].invc};
  vector double logcl = {T[i[0]].logc, T[i[1]].logc};
  vector double logcr = {T[i[2]].logc, T[i[3]].logc};
  z = vasfloat(iz);
  vector double zl = {z[0], z[1]};
  vector double zr = {z[2], z[3]};

  /* log2(x) = log1p(z/c-1)/ln2 + log2(c) + k */
  rl = zl * invcl  - 1;
  rr = zr * invcr - 1;
  vector double kl = {(double)k[0], (double)k[1]};
  vector double kr = {(double)k[2], (double)k[3]};
  y0l = logcl + kl;
  y0r = logcr + kr;

  /* Pipelined polynomial evaluation to approximate log1p(r)/ln2.  */
  r2l = rl * rl;
  r2r = rr * rr;
  yl = A[0] * rl + A[1];
  yr = A[0] * rr + A[1];
  pl = A[2] * rl + A[3];
  pr = A[2] * rr + A[3];
  r4l = r2l * r2l;
  r4r = r2r * r2r;
  ql = A[4] * rl + y0l;
  qr = A[4] * rr + y0r;
  ql = pl * r2l + ql;
  qr = pr * r2r + qr;
  yl = yl * r4l + ql;
  yr = yr * r4r + qr;
  ret.l = yl;
  ret.r = yr;
  return ret;
}

#undef N
#undef T
#define N (1 << EXP2F_TABLE_BITS)
#define T __exp2f_data.tab
#define SIGN_BIAS (1 << (EXP2F_TABLE_BITS + 11))

/* The output of log2 and thus the input of exp2 is either scaled by N
   (in case of fast toint intrinsics) or not.  The unscaled xd must be
   in [-1021,1023], sign_bias sets the sign of the result.  */
static inline vector float
exp2_inline (vector double xdl, vector double xdr, vector unsigned sign_bias)
{
  v64u kil, kir, skil, skir, sign_biasl, sign_biasr;
  vector double kdl, kdr, zl, zr, rl, rr, r2l, r2r, yl, yr, sl, sr;

  vector unsigned zero = {0, 0, 0, 0};
#ifdef __LITTLE_ENDIAN__
  sign_biasl = (v64u) vec_mergeh (sign_bias, zero);
  sign_biasr = (v64u) vec_mergel (sign_bias, zero);
#else
  sign_biasl = (v64u) vec_mergel (zero, sign_bias);
  sign_biasr = (v64u) vec_mergeh (zero, sign_bias);
#endif
#define C __exp2f_data.poly
#define SHIFT __exp2f_data.shift_scaled
  /* x = k/N + r with r in [-1/(2N), 1/(2N)] */
  kdl = xdl + SHIFT;
  kdr = xdr + SHIFT;
  kil = vasuint64 (kdl);
  kir = vasuint64 (kdr);
  kdl -= SHIFT;
  kdr -= SHIFT; /* k/N */
  rl = xdl - kdl;
  rr = xdr - kdr;

  /* exp2(x) = 2^(k/N) * 2^r ~= s * (C0*r^3 + C1*r^2 + C2*r + 1) */
  v64u tl = {T[kil[0] % N], T[kil[1] % N]};
  v64u tr = {T[kir[0] % N], T[kir[1] % N]};
  skil = kil + sign_biasl;
  skir = kir + sign_biasr;
  tl += skil << (52 - EXP2F_TABLE_BITS);
  tr += skir << (52 - EXP2F_TABLE_BITS);
  sl = vasdouble(tl);
  sr = vasdouble(tr);
  zl = C[0] * rl + C[1];
  zr = C[0] * rr + C[1];
  r2l = rl * rl;
  r2r = rr * rr;
  yl = C[2] * rl + 1;
  yr = C[2] * rr + 1;
  yl = zl * r2l + yl;
  yr = zr * r2r + yr;
  yl = yl * sl;
  yr = yr * sr;
  /* There is no vector pack/unpack for 64<->32 */
  vector float res = {(float)yl[0], (float)yl[1], (float)yr[0], (float)yr[1]};
  return res;
}

/* Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
   the bit representation of a non-zero finite floating-point value.  */
static inline vector unsigned
checkint (vector unsigned iy)
{
  vector unsigned e = iy >> 23 & 0xff;
  vector unsigned zero = {0, 0, 0, 0};
  vector unsigned not_matched = ~zero;
  vector unsigned res;
  vector unsigned is_first = (vector unsigned) vec_cmplt (e, zero + 0x7f);
  not_matched &= ~is_first;
  res = zero;
  vector unsigned is_second = (vector unsigned) vec_cmpgt (e, zero + 0x7f + 23);
  not_matched &= ~is_second;
  res = vec_sel (res, zero + 2, is_second);
  vector unsigned is_third =
    (vector unsigned) vec_cmpne (iy & ((1 << (0x7f + 23 - e)) - 1), zero);
  res = vec_sel (res, zero, is_third & not_matched);
  not_matched &= ~is_third;
  vector unsigned is_four =
    (vector unsigned) vec_cmpne (iy & (1 << (0x7f + 23 - e)), zero);
  res = vec_sel (res, zero + 1, is_four & not_matched);
  not_matched &= ~is_four;
  res = vec_sel (res, zero + 2, not_matched);
  return res;
}

static vector unsigned
zeroinfnan (vector unsigned ix)
{
  vector unsigned zero = {0, 0, 0, 0};
  return (vector unsigned) vec_cmpge (2 * ix - 1, zero + (2u * 0x7f800000 - 1));
}

vector float
_ZGVbN4vv_powf (vector float x, vector float y)
{
  vector unsigned zero = {0, 0, 0, 0};
  vector unsigned sign_bias = zero;
  vector unsigned ix, iy, res = zero, res_mask = zero;

  ix = vasuint (x);
  iy = vasuint (y);
  vector unsigned special_cst = {0x7f800000 - 0x00800000,
				 0x7f800000 - 0x00800000,
				 0x7f800000 - 0x00800000,
				 0x7f800000 - 0x00800000};
  vector unsigned is_special = (vector unsigned) vec_cmpge (ix - 0x00800000,
							    special_cst);
  vector unsigned is_zeroinfnanx = zeroinfnan(iy);
  if (__glibc_unlikely (!vec_all_eq (is_special | is_zeroinfnanx, zero)))
    {
      if (!vec_all_eq(is_zeroinfnanx, zero))
	{
	  vector unsigned not_covered = is_zeroinfnanx;
	  res_mask = is_zeroinfnanx;
	  vector unsigned is_one = (vector unsigned) vec_cmpeq (2 * iy, zero);
	  vector float one = {1.0f, 1.0f, 1.0f, 1.0f};
	  vector unsigned is_two =
	    (vector unsigned) vec_cmpeq (ix, zero + 0x3f800000);
	  res = vec_sel (res, vasuint (one), (is_one | is_two) & not_covered);
	  not_covered &= ~(is_one | is_two);
	  vector unsigned is_threea =
	    (vector unsigned) vec_cmpgt (2 * ix, zero + 2u * 0x7f800000);
	  vector unsigned is_threeb =
	    (vector unsigned) vec_cmpgt (2 * iy, zero + 2u * 0x7f800000);
	  vector float xy = x + y;
	  res = vec_sel (res, vasuint (xy), (is_threea | is_threeb)
			 & not_covered);
	  not_covered &= ~(is_threea | is_threeb);
	  vector unsigned is_four =
	    (vector unsigned) vec_cmpeq (2 * ix,  zero + 2 * 0x3f800000);
	  res = vec_sel (res, vasuint (one), is_four & not_covered);
	  not_covered &= ~is_four;
	  vector unsigned is_fivea =
	    (vector unsigned) vec_cmplt (2 * ix, zero + 2 * 0x3f800000);
	  vector unsigned is_fiveb =
	    (vector unsigned) vec_cmplt (iy, zero + 0x80000000);
	  vector unsigned is_five =
	    (vector unsigned) vec_cmpeq (is_fivea, is_fiveb);
	  res = vec_sel (res, zero, is_five & not_covered);
	  not_covered &= ~is_five;
	  vector float yy = y * y;
	  res = vec_sel (res, vasuint (yy), not_covered);
	}
      vector unsigned is_ix =
	(vector unsigned) vec_cmpge (ix, zero + 0x80000000);
      vector unsigned is_xinfnan = zeroinfnan(ix);
      if (!vec_all_eq (is_xinfnan & ~res_mask, zero))
	{
	  vector float x2 = x * x;
	  vector unsigned is_checkinty =
	    (vector unsigned) vec_cmpeq (checkint(iy), zero + 1);
	  if (!vec_all_eq (is_ix & is_checkinty, zero))
	    x2 = vec_sel (x2, -x2, is_ix & is_checkinty);
	  vector unsigned is_iy =
	    (vector unsigned) vec_cmpge (iy, zero + 0x80000000);
	  if (!vec_all_eq (is_iy, zero))
	    {
	      math_force_eval (1 / x2);
	      x2 = vec_sel (x2, 1 / x2, is_iy);
	    }
	  res = vec_sel (res, vasuint(x2), is_xinfnan);
	  res_mask |= is_xinfnan;
	}
      vector unsigned is_xneg =
	(vector unsigned) vec_cmpgt (ix, zero + 0x80000000);
      if (!vec_all_eq (is_xneg, zero))
	{
	  vector unsigned yint = checkint (iy);
	  vector unsigned is_invalid = (vector unsigned) vec_cmpeq (yint, zero);
	  if (!vec_all_eq(is_invalid & ~res_mask, zero))
	    {
	      for (int m=0;m<4;m++)
		{
		  if ((is_invalid & ~res_mask)[m] == 0)
		      continue;
		  res[m] = asuint (__math_invalidf (x[m]));
		  res_mask[m] = 0xffffffff;
		}
	    }
	  vector unsigned is_one = (vector unsigned) vec_cmpeq (yint, zero + 1);
	  sign_bias = vec_sel (sign_bias, zero + SIGN_BIAS, is_one);
	  ix = vec_sel (ix, ix & 0x7fffffff, is_xneg);
	}
      vector unsigned is_subnormal =
	(vector unsigned) vec_cmplt (ix, zero + 0x00800000);
      if (!vec_all_eq (is_subnormal & ~res_mask, zero))
	{
	  vector unsigned subnormals = ix;
	  subnormals = vasuint (vasfloat(ix) * 0x1p23f);
	  subnormals = subnormals & (unsigned)0x7ffffffff;
	  subnormals -= 23 << 23;
	  ix = vec_sel (ix, subnormals, is_subnormal & ~res_mask);
	}
      /* If we already filled in all the results, we can return early.  */
      if (vec_all_eq (res_mask, ~zero))
	  return vasfloat (res);
    }
  struct two_v_doubles logx = log2_inline (ix);

  vector double yl = {y[0], y[1]};
  vector double yr = {y[2], y[3]};
  vector double ylogxl = yl * logx.l;
  vector double ylogxr = yr * logx.r;
  v64u overunderflow_const = {asuint64(126.0 * POWF_SCALE) >> 47,
			      asuint64(126.0 * POWF_SCALE) >> 47};
  v64u is_overunderflowl = (v64u) vec_cmpge
      (((vasuint64 (ylogxl) >> 47) & 0xffff), overunderflow_const);
  v64u is_overunderflowr = (v64u) vec_cmpge
      (((vasuint64(ylogxr) >> 47) & 0xffff), overunderflow_const);
  vector unsigned is_overunderflow = vec_pack (is_overunderflowl,
					       is_overunderflowr);
  if (__glibc_unlikely (!vec_all_eq (is_overunderflow, zero)))
    {
      vector double ylogx = ylogxl;
      for (int m=0;m<4;m++)
	{
	  if (is_overunderflow[m] == 0)
	      continue;
	  if (m == 2 || m == 3)
	      ylogx = ylogxr;
	  if (ylogx[m % 2] > (0x1.fffffffd1d571p+6 * POWF_SCALE))
	    {
	      res[m] = asuint (__math_oflowf (sign_bias[m]));
	      res_mask[m] = 0xffffffff;
	    }
	  else if (ylogx[m % 2] <= (-150.0 * POWF_SCALE))
	    {
	      res[m] = asuint (__math_uflowf (sign_bias[m]));
	      res_mask[m] = 0xffffffff;
	    }
	}
    }
  vector unsigned exp2 = vasuint (exp2_inline (ylogxl, ylogxr, sign_bias));
  return vasfloat (vec_sel (exp2, res, res_mask));
}
