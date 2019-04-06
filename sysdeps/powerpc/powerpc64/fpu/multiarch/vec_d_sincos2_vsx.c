/* Function sincos vectorized with VSX.
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
#include "vec_d_sincos2_vsx.h"

void
_ZGVbN2vvv_sincos (vector double x, vector double * sines_x, vector double * cosines_x)
{

  /* Call vector sine evaluator.  */
  *sines_x = __d_sin_poly_eval(x);

  /* Call vector cosine evaluator.  */
  *cosines_x = __d_cos_poly_eval(x);

} /* Function _ZGVbN2_vvv_sincos.  */
