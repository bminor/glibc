/* Constants used in polynomial approximations for vectorized sinf, cosf,
   and sincosf functions.
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

#ifndef S_TRIG_DATA_H
#define S_TRIG_DATA_H

#include <altivec.h>

/* PI/2.  */
static const vector float __s_half_pi =
{ 0x1.921fb6p+0, 0x1.921fb6p+0, 0x1.921fb6p+0, 0x1.921fb6p+0 };

/* Inverse PI.  */
static const vector float __s_inv_pi =
{ 0x1.45f306p-2, 0x1.45f306p-2, 0x1.45f306p-2, 0x1.45f306p-2 };

/* Right-shifter constant.  */
static const vector float __s_rshifter =
{ 0x1.8p+23, 0x1.8p+23, 0x1.8p+23, 0x1.8p+23 };

/* One-half.  */
static const vector float __s_one_half =
{ 0x1p-1, 0x1p-1, 0x1p-1, 0x1p-1 };

/* Threshold for out-of-range values.  */
static const vector float __s_rangeval =
{ 0x1.388p+13, 0x1.388p+13, 0x1.388p+13, 0x1.388p+13 };

/* PI1, PI2, and PI3 when FMA is available
   PI high part (when FMA available).  */
static const vector float __s_pi1_fma =
{ 0x1.921fb6p+1, 0x1.921fb6p+1, 0x1.921fb6p+1, 0x1.921fb6p+1 };

/* PI mid part  (when FMA available).  */
static const vector float __s_pi2_fma =
{ -0x1.777a5cp-24, -0x1.777a5cp-24, -0x1.777a5cp-24, -0x1.777a5cp-24 };

/* PI low part  (when FMA available).  */
static const vector float __s_pi3_fma =
{ -0x1.ee59dap-49, -0x1.ee59dap-49, -0x1.ee59dap-49, -0x1.ee59dap-49 };

/* Polynomial constants for work w/o FMA, relative error ~ 2^(-26.625).  */
static const vector float __s_a3 =
{ -0x1.55554cp-3, -0x1.55554cp-3, -0x1.55554cp-3, -0x1.55554cp-3 };

/* Polynomial constants, work with FMA, relative error ~ 2^(-26.417).  */
static const vector float __s_a5_fma =
{ 0x1.110edp-7, 0x1.110edp-7, 0x1.110edp-7, 0x1.110edp-7 };

static const vector float __s_a7_fma =
{ -0x1.9f6d9ep-13, -0x1.9f6d9ep-13, -0x1.9f6d9ep-13, -0x1.9f6d9ep-13 };

static const vector float __s_a9_fma =
{ 0x1.5d866ap-19, 0x1.5d866ap-19, 0x1.5d866ap-19, 0x1.5d866ap-19 };

/* Absolute value mask.  */
static const vector bool int __s_abs_mask =
{ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };

#endif /* S_TRIG_DATA_H.  */
