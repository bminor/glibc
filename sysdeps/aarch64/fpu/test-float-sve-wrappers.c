/* Scalar wrappers for single-precision SVE vector math functions.

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

#include "test-float-sve.h"

/* Wrapper from scalar to SVE function. Cannot just use VECTOR_WRAPPER due to
   predication.  */
#define SVE_VECTOR_WRAPPER(scalar_func, vector_func)                          \
  extern VEC_TYPE vector_func (VEC_TYPE, svbool_t);                           \
  FLOAT scalar_func (FLOAT x)                                                 \
  {                                                                           \
    VEC_TYPE mx = svdup_n_f32 (x);                                            \
    VEC_TYPE mr = vector_func (mx, svptrue_b32 ());                           \
    return svlastb_f32 (svptrue_b32 (), mr);                                  \
  }

SVE_VECTOR_WRAPPER (cosf_sve, _ZGVsMxv_cosf)
SVE_VECTOR_WRAPPER (expf_sve, _ZGVsMxv_expf)
SVE_VECTOR_WRAPPER (logf_sve, _ZGVsMxv_logf)
SVE_VECTOR_WRAPPER (sinf_sve, _ZGVsMxv_sinf)
