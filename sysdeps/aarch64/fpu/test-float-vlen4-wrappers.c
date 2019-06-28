/* Wrapper part of tests for float aarch64 vector math functions.
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

#include <arm_neon.h>
#include "test-float-vlen4.h"

#define VEC_TYPE float32x4_t

/* Hack: VECTOR_WRAPPER declares the vector function without the pcs attribute,
   placing it here happens to work, should be fixed in test-math-vector.h.  */
__attribute__ ((aarch64_vector_pcs))

VECTOR_WRAPPER (WRAPPER_NAME (cosf), _ZGVnN4v_cosf)

__attribute__ ((aarch64_vector_pcs))
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVnN4v_expf)

__attribute__ ((aarch64_vector_pcs))
VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVnN4v_logf)

__attribute__ ((aarch64_vector_pcs))
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVnN4vv_powf)

__attribute__ ((aarch64_vector_pcs))
VECTOR_WRAPPER (WRAPPER_NAME (sinf), _ZGVnN4v_sinf)
