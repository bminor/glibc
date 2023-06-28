/* Scalar wrappers for single-precision Advanced SIMD vector math functions.

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

#include <arm_neon.h>

#include "test-float-advsimd.h"

#define VEC_TYPE float32x4_t

VPCS_VECTOR_WRAPPER (cosf_advsimd, _ZGVnN4v_cosf)
VPCS_VECTOR_WRAPPER (expf_advsimd, _ZGVnN4v_expf)
VPCS_VECTOR_WRAPPER (logf_advsimd, _ZGVnN4v_logf)
VPCS_VECTOR_WRAPPER (sinf_advsimd, _ZGVnN4v_sinf)
