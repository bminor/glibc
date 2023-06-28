/* Scalar wrappers for double-precision SVE vector math functions.

   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <arm_sve.h>

#include "test-double-sve.h"

/* Wrapper from scalar to SVE function. Cannot just use VECTOR_WRAPPER due to
   predication.  */
#define SVE_VECTOR_WRAPPER(scalar_func, vector_func)                          \
  extern VEC_TYPE vector_func (VEC_TYPE, svbool_t);                           \
  FLOAT scalar_func (FLOAT x)                                                 \
  {                                                                           \
    VEC_TYPE mx = svdup_n_f64 (x);                                            \
    VEC_TYPE mr = vector_func (mx, svptrue_b64 ());                           \
    return svlastb_f64 (svptrue_b64 (), mr);                                  \
  }

SVE_VECTOR_WRAPPER (cos_sve, _ZGVsMxv_cos)
SVE_VECTOR_WRAPPER (exp_sve, _ZGVsMxv_exp)
SVE_VECTOR_WRAPPER (log_sve, _ZGVsMxv_log)
SVE_VECTOR_WRAPPER (sin_sve, _ZGVsMxv_sin)
