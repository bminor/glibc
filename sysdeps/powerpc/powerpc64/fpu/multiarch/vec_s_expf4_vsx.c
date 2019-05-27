/* Single-precision vector expf function.
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
/* Based off ./sysdeps/ieee754/flt-32/e_expf.c
   which was contributed by Szabolcs Nagy of ARM Ltd.  */
#include <altivec.h>
#include <math.h>

#include "math_config_flt.h"

typedef vector long long unsigned v64u;
typedef union {
  vector unsigned u;
  vector float f;
  v64u l;
  vector double d;
} u;
typedef union {
  double d;
  int64_t l;
  unsigned u;
  float f;
} us;

#define N (1 << EXP2F_TABLE_BITS)
#define InvLn2N __exp2f_data.invln2_scaled
#define T __exp2f_data.tab
#define C __exp2f_data.poly_scaled

vector float
_ZGVbN4v_expf (vector float x)
{
  u res;
  u xu;
  xu.f = x;
  us c88;
  c88.f = 88.0f;
  us inf;
  inf.f = INFINITY;
  us ninf;
  ninf.f = -INFINITY;
  vector unsigned constants = {(c88.u & 0xfff00000) << 1,
				ninf.u, inf.u, 0};
  vector float constants2 = {0x1.62e42ep6f, -0x1.9fe368p6f, 0, 0};
  vector unsigned zero = {0, 0, 0, 0};
  vector unsigned v88 = vec_splat (constants, 0);
  vector unsigned is_special_case
	= (vector unsigned) vec_cmpge (xu.u << 1, v88);
  vector unsigned is_special_not_covered = is_special_case;
  if (__glibc_unlikely (!vec_all_eq (is_special_case, zero)))
    {
      vector unsigned inf = vec_splat (constants, 2);
      vector unsigned is_inf_or_ninf_or_nan
	= (vector unsigned) vec_cmpge (xu.u, inf);
      u xpx;
      xpx.f = x + x;
      res.u = xpx.u;
      is_special_not_covered &= ~is_inf_or_ninf_or_nan;
      vector unsigned ninf = vec_splat (constants, 1);
      vector unsigned is_ninf
	= (vector unsigned) vec_cmpeq (xu.u, ninf);
      res.u = vec_sel (res.u, zero, is_ninf);

      vector float overflow_v = vec_splat (constants2, 0);
      vector unsigned is_overflow
	= (vector unsigned) vec_cmpgt (xu.f, overflow_v);
      if (__glibc_unlikely (!vec_all_eq (is_overflow, zero)))
	  {
	  // This branch is because we are generating an
	  // overflow fp flag
	  us ofu;
	  ofu.f = __math_oflowf (0);
	  vector unsigned of = {ofu.u, ofu.u, ofu.u, ofu.u};
	  res.u = vec_sel (res.u, of, is_overflow);
	  is_special_not_covered &= ~is_overflow;
	}
      vector float underflow_v = vec_splat (constants2, 1);
      vector unsigned is_underflow
	= (vector unsigned) vec_cmplt (xu.f, underflow_v);
      if (__glibc_unlikely (!vec_all_eq (is_underflow, zero)))
	{
	  // This branch is because we are generating an
	  // underflow fp flag
	  us ufu;
	  ufu.f = __math_uflowf (0);
	  vector unsigned uf = {ufu.u, ufu.u, ufu.u, ufu.u};
	  res.u = vec_sel (res.u, uf, is_underflow);
	  is_special_not_covered &= ~is_underflow;
	}
     }

#if __GNUC__ >= 8
  vector double xl = vec_unpackh (x);
  vector double xr = vec_unpackl (x);
#else
  vector double xl = {x[0], x[1]};
  vector double xr = {x[2], x[3]};
#endif
  vector double zl = InvLn2N * xl;
  vector double zr = InvLn2N * xr;
#if TOINT_INTRINSICS
  vector double kdl = roundtoint (zl);
  vector double kdl = roundtoint (zl);
  v64u kil = converttoint (zl);
  v64u kir = converttoint (zr);
#else
#define SHIFT __exp2f_data.shift
  vector double kdl = zl + SHIFT;
  vector double kdr = zr + SHIFT;
  u kilu;
  kilu.d = kdl;
  u kiru;
  kiru.d = kdr;
  v64u kil = kilu.l;
  v64u kir = kiru.l;
  kdl -= SHIFT;
  kdr -= SHIFT;
#endif
  vector double rl = zl - kdl;
  vector double rr = zr - kdr;

  v64u tl = {T[kil[0] % N], T[kil[1] % N]};
  v64u tr = {T[kir[0] % N], T[kir[1] % N]};
  tl += (kil << (52 - EXP2F_TABLE_BITS));
  tr += (kir << (52 - EXP2F_TABLE_BITS));
  u slu;
  slu.l = tl;
  u sru;
  sru.l = tr;
  // This cast is obnoxious, but there is no vec_ld for double
  zl = C[0] * rl + C[1];
  zr = C[0] * rr + C[1];
  vector double r2l = rl * rl;
  vector double r2r = rr * rr;
  vector double yl = C[2] * rl + 1;
  vector double yr = C[2] * rr + 1;
  yl = zl * r2l + yl;
  yr = zr * r2r + yr;
  yl = yl * slu.d;
  yr = yr * sru.d;
  vector float restmp = {(float) yl[0], (float) yl[1],
	(float) yr[0], (float) yr[1]};
  return vec_sel (restmp, res.f,
	is_special_case & ~is_special_not_covered);
}
