/* Definitions to simplify code by allowing reuse of sine and cosine
   function implementations.
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
#include "vec_s_trig_data.h"

static inline vector float
__s_cos_poly_eval (vector float x)
{

  /*
   ALGORITHM DESCRIPTION:

   1) Range reduction to [-Pi/2; +Pi/2] interval
     a) We remove sign using absolute value operation
     b) Add Pi/2 value to argument X for Cos to Sin transformation
     c) Getting octant Y by 1/Pi multiplication
     d) Add "Right Shifter" value
     e) Treat obtained value as integer for destination sign setting.
        Shift first bit of this value to the last (sign) position
     f) Subtract "Right Shifter"  value
     g) Subtract 0.5 from result for octant correction
     h) Subtract Y*PI from X argument, where PI divided to 4 parts:
          X = X - Y*PI1 - Y*PI2 - Y*PI3 - Y*PI4;
   2) Polynomial (minimax for sin within [-Pi/2; +Pi/2] interval)
     a) Calculate X^2 = X * X
     b) Calculate polynomial:
         R = X + X * X^2 * (A3 + x^2 * (A5 + .....
   3) Destination sign setting
     a) Set shifted destination sign using XOR operation:
          R = XOR( R, S ).  */

  /*
   ARGUMENT RANGE REDUCTION:
   Add Pi/2 to argument: X' = X+Pi/2. Transforms cos to sin.  */
  vector float x_prime = __s_half_pi + x;

  /* Y = X'*InvPi + RS : right shifter add.  */
  vector float y = (x_prime * __s_inv_pi) + __s_rshifter;

  /* N = Y - RS : right shifter sub.  */
  vector float n = y - __s_rshifter;

  /* SignRes = Y<<31 : shift LSB to MSB place for result sign.  */
  vector float sign_res = (vector float)
      vec_sl ((vector signed int) y, (vector unsigned int) vec_splats (31));

  /* N = N - 0.5.  */
  n = n - __s_one_half;

  /* Get absolute argument value: X = |X|.  */
  vector float abs_x = vec_abs (x);

  /* Check for large arguments path.  */
  vector bool int large_in = vec_cmpgt (abs_x, __s_rangeval);

  /* R = X - N*Pi1. */
  vector float r = x - (n * __s_pi1_fma);

  /* R = R - N*Pi2.  */
  r = r - (n * __s_pi2_fma);

  /* R = R - N*Pi3.  */
  r = r - (n * __s_pi3_fma);

  /* R2 = R*R.  */
  vector float r2 = r * r;

  /* RECONSTRUCTION:
     Final sign setting: Res = Poly^SignRes.  */
  vector float res = (vector float)
      ((vector signed int) r ^ (vector signed int) sign_res);

  /* Poly = R + R * R2*(A3+R2*(A5+R2*(A7+R2*A9))). */
  vector float poly = r2 * __s_a9_fma + __s_a7_fma;
  poly = poly * r2 + __s_a5_fma;
  poly = poly * r2 + __s_a3;
  poly = poly * r2 * res + res;

  if (large_in[0])
    poly[0] = cosf (x[0]);

  if (large_in[1])
    poly[1] = cosf (x[1]);

  if (large_in[2])
    poly[2] = cosf (x[2]);

  if (large_in[3])
    poly[3] = cosf (x[3]);

  return poly;

} /* Function __s_cos_poly_eval.  */

static inline vector float
__s_sin_poly_eval (vector float x)
{

  /*
    ALGORITHM DESCRIPTION:

    1) Range reduction to [-Pi/2; +Pi/2] interval
       a) Grab sign from source argument and save it.
       b) Remove sign using AND operation
       c) Getting octant Y by 1/Pi multiplication
       d) Add "Right Shifter" value
       e) Treat obtained value as integer for destination sign setting.
          Shift first bit of this value to the last (sign) position
       f) Change destination sign if source sign is negative
          using XOR operation.
       g) Subtract "Right Shifter" value
       h) Subtract Y*PI from X argument, where PI divided to 4 parts:
          X = X - Y*PI1 - Y*PI2 - Y*PI3 - Y*PI4;
    2) Polynomial (minimax for sin within [-Pi/2; +Pi/2] interval)
       a) Calculate X^2 = X * X
       b) Calculate polynomial:
          R = X + X * X^2 * (A3 + x^2 * (A5 + ......
    3) Destination sign setting
       a) Set shifted destination sign using XOR operation:
          R = XOR( R, S ).  */


  /* Remove sign of input argument: X'=|X|.  */
  vector float abs_x = vec_abs (x);

  /* Getting octant Y by 1/Pi multiplication. Add "Right Shifter" value.
     Y = X'*InvPi + RS.  */
  vector float y = (abs_x * __s_inv_pi) + __s_rshifter;

  /* N = Y - RS : right shifter sub.  */
  vector float n = y - __s_rshifter;

  /* SignRes = Y<<31 : shift LSB to MSB place for result sign.  */
  vector float sign_res = (vector float)
      vec_sl ((vector signed int) y, (vector unsigned int) vec_splats (31));

  /* Subtract N*PI from X argument, where PI divided into 3 parts.  */
  /* R = X - N*PI1 - N*PI2 - N*PI3.  */
  vector float r = abs_x - (n * __s_pi1_fma);

  /* R = R - N*Pi2.  */
  r = r - (n * __s_pi2_fma);

  /* R = R - N*Pi3.  */
  r = r - (n * __s_pi3_fma);

  /* Check for large arguments path.  */
  vector bool int large_in = vec_cmpgt (abs_x, __s_rangeval);

  /* Polynomial (minimax for sin within [-Pi/2; +Pi/2] interval).  */

  /* R2 = R*R.  */
  vector float r2 = r * r;

  /* Change destination sign if source sign is -ve using XOR operation.  */
  vector float neg_sign = vec_andc (x, __s_abs_mask);

  vector float res = (vector float)
      ((vector signed int) r ^ (vector signed int) sign_res);

  /* Poly = R + R * R2*(A3+R2*(A5+R2*(A7+R2*A9))). */
  vector float poly = r2 * __s_a9_fma + __s_a7_fma;
  poly = poly * r2 + __s_a5_fma;
  poly = poly * r2 + __s_a3;
  poly = poly * r2 * res + res;

/* Destination sign setting.
   Set shifted destination sign using XOR operation: R = XOR( R, S ).  */
  vector float out
      = (vector float) ((vector int) poly ^ (vector int) neg_sign);

  if (large_in[0])
    out[0] = sinf (x[0]);

  if (large_in[1])
    out[1] = sinf (x[1]);

  if (large_in[2])
    out[2] = sinf (x[2]);

  if (large_in[3])
    out[3] = sinf (x[3]);

  return out;

} /* Function __s_sin_poly_eval.  */
