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
#include "vec_d_trig_data.h"

static inline vector double
__d_cos_poly_eval (vector double x)
{

  /*
   ARGUMENT RANGE REDUCTION:
   Add Pi/2 to argument: X' = X+Pi/2.  */
  vector double x_prime = (vector double) __d_half_pi + x;

  /* Get absolute argument value: X' = |X'|.  */
  vector double abs_x_prime = vec_abs (x_prime);

  /* Y = X'*InvPi + RS : right shifter add.  */
  vector double y = (x_prime * __d_inv_pi) + __d_rshifter;

  /* Check for large arguments path.  */
  vector bool long long large_in = vec_cmpgt (abs_x_prime, __d_rangeval);

  /* N = Y - RS : right shifter sub.  */
  vector double n = y - __d_rshifter;

  /* SignRes = Y<<63 : shift LSB to MSB place for result sign.  */
  vector double sign_res = (vector double) vec_sl ((vector long long) y,
						   (vector unsigned long long)
						   vec_splats (63));

  /* N = N - 0.5.  */
  n = n - __d_one_half;

  /* R = X - N*Pi1.  */
  vector double r = x - (n * __d_pi1_fma);

  /* R = R - N*Pi2.  */
  r = r - (n * __d_pi2_fma);

  /* R = R - N*Pi3.  */
  r = r - (n * __d_pi3_fma);

  /* R2 = R*R.  */
  vector double r2 = r * r;

  /* Poly = C3+R2*(C4+R2*(C5+R2*(C6+R2*C7))).  */
  vector double poly = r2 * __d_coeff7 + __d_coeff6;
  poly = poly * r2 + __d_coeff5;
  poly = poly * r2 + __d_coeff4;
  poly = poly * r2 + __d_coeff3;

  /* Poly = R+R*(R2*(C1+R2*(C2+R2*Poly))).  */
  poly = poly * r2 + __d_coeff2;
  poly = poly * r2 + __d_coeff1;
  poly = poly * r2 * r + r;

  /*
     RECONSTRUCTION:
     Final sign setting: Res = Poly^SignRes.  */
  vector double out
    = (vector double) ((vector long long) poly ^ (vector long long) sign_res);

  if (large_in[0] != 0)
    out[0] = cos (x[0]);

  if (large_in[1] != 0)
    out[1] = cos (x[1]);

  return out;

} /* Function __d_cos_poly_eval.  */

static inline vector double
__d_sin_poly_eval (vector double x)
{
  /* ALGORITHM DESCRIPTION:

      ( low accuracy ( < 4ulp ) or enhanced performance
      ( half of correct mantissa ) implementation )

     Argument representation:
     arg = N*Pi + R

     Result calculation:
     sin(arg) = sin(N*Pi + R) = (-1)^N * sin(R)
     sin(R) is approximated by corresponding polynomial.  */

  /* ARGUMENT RANGE REDUCTION: X' = |X|.  */
  vector double abs_x_prime = vec_abs (x);

  /* Y = X'*InvPi + RS : right shifter add.  */
  vector double y = (abs_x_prime * __d_inv_pi) + __d_rshifter;

  /* N = Y - RS : right shifter sub.  */
  vector double n = y - __d_rshifter;

  /* SignRes = Y<<63 : shift LSB to MSB place for result sign.  */
  vector double sign_res = (vector double) vec_sl ((vector long long) y,
                                                   (vector unsigned long long)
                                                   vec_splats (63));

  /* Check for large arguments path.  */
  vector bool long long large_in = vec_cmpgt (abs_x_prime, __d_rangeval);

  /* R = X' - N*Pi1.  */
  vector double r = abs_x_prime - (n * __d_pi1_fma);

  /* R = R - N*Pi2.  */
  r = r - (n * __d_pi2_fma);

  /* R = R - N*Pi3.  */
  r = r - (n * __d_pi3_fma);

  /* POLYNOMIAL APPROXIMATION: R2 = R*R.  */
  vector double r2 = r * r;

  /* R = R^SignRes : update sign of reduced argument.  */
  vector double r_sign
      = (vector double) ((vector long long) r ^ (vector long long) sign_res);

  /* Poly = C3+R2*(C4+R2*(C5+R2*(C6+R2*C7))).  */
  vector double poly = r2 * __d_coeff7_sin + __d_coeff6_sin;
  poly = poly * r2 + __d_coeff5_sin;
  poly = poly * r2 + __d_coeff4_sin;
  poly = poly * r2 + __d_coeff3_sin;

  /* Poly = R2*(C1+R2*(C2+R2*Poly)).  */
  poly = poly * r2 + __d_coeff2_sin;
  poly = poly * r2 + __d_coeff1_sin;
  poly = poly * r2;

  /* Poly = Poly*R + R.  */
  poly = poly * r_sign + r_sign;

  /* SignX: -ve sign bit of X.  */
  vector double neg_sign
      = (vector double) vec_andc ((vector bool long long) x, __d_abs_mask);

  /* RECONSTRUCTION: Final sign setting: Res = Poly^SignX.  */
  vector double out
      = (vector double) ((vector long long) poly ^ (vector long long) neg_sign);

  if (large_in[0] != 0)
    out[0] = sin (x[0]);

  if (large_in[1] != 0)
    out[1] = sin (x[1]);

  return out;

} /* Function __d_sin_poly_eval.  */
