/* Constants used in polynomial approximations for vectorized sin, cos,
   and sincos functions.
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

#ifndef D_TRIG_DATA_H
#define D_TRIG_DATA_H

#include <altivec.h>

/* PI/2.  */
static const vector double __d_half_pi  = {0x1.921fb54442d18p+0, 0x1.921fb54442d18p+0};

/* Inverse PI.  */
static const vector double __d_inv_pi   = {0x1.45f306dc9c883p-2, 0x1.45f306dc9c883p-2};

/* Right-shifter constant.  */
static const vector double __d_rshifter = {0x1.8p+52, 0x1.8p+52};

/* Working range threshold.  */
static const vector double __d_rangeval = {0x1p+23, 0x1p+23};

/* One-half.  */
static const vector double __d_one_half = {0x1p-1, 0x1p-1};

/* Range reduction PI-based constants if FMA available:
   PI high part (FMA available).  */
static const vector double __d_pi1_fma = {0x1.921fb54442d18p+1, 0x1.921fb54442d18p+1};

/* PI mid part  (FMA available).  */
static const vector double __d_pi2_fma = {0x1.1a62633145c06p-53, 0x1.1a62633145c06p-53};

/* PI low part  (FMA available).  */
static const vector double __d_pi3_fma
= {0x1.c1cd129024e09p-106,0x1.c1cd129024e09p-106};

/* Polynomial coefficients for cosine (relative error 2^(-52.115)).  */
static const vector double __d_coeff7 = {-0x1.9f0d60811aac8p-41,-0x1.9f0d60811aac8p-41};
static const vector double __d_coeff6 = {0x1.60e6857a2f22p-33,0x1.60e6857a2f22p-33};
static const vector double __d_coeff5 = {-0x1.ae63546002231p-26,-0x1.ae63546002231p-26};
static const vector double __d_coeff4 = {0x1.71de38030feap-19,0x1.71de38030feap-19};
static const vector double __d_coeff3 = {-0x1.a01a019a5b86dp-13,-0x1.a01a019a5b86dp-13};
static const vector double __d_coeff2 = {0x1.111111110a4a8p-7,0x1.111111110a4a8p-7};
static const vector double __d_coeff1 = {-0x1.55555555554a7p-3,-0x1.55555555554a7p-3};

/* Absolute value mask.  */
static const vector bool long long __d_abs_mask = { 0x7fffffffffffffff, 0x7fffffffffffffff };

/* Polynomial coefficients for sine (relative error 2^(-52.115)).  */
static const vector double __d_coeff7_sin = { -0x1.9f1517e9f65fp-41, -0x1.9f1517e9f65fp-41 };
static const vector double __d_coeff6_sin = { 0x1.60e6bee01d83ep-33, 0x1.60e6bee01d83ep-33 };
static const vector double __d_coeff5_sin = { -0x1.ae6355aaa4a53p-26, -0x1.ae6355aaa4a53p-26 };
static const vector double __d_coeff4_sin = { 0x1.71de3806add1ap-19, 0x1.71de3806add1ap-19 };
static const vector double __d_coeff3_sin = { -0x1.a01a019a659ddp-13, -0x1.a01a019a659ddp-13 };
static const vector double __d_coeff2_sin = { 0x1.111111110a573p-7, 0x1.111111110a573p-7 };
static const vector double __d_coeff1_sin = { -0x1.55555555554a8p-3, -0x1.55555555554a8p-3 };

#endif /* D_TRIG_DATA_H.  */
