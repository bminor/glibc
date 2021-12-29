/* Wrapper part of tests for AVX-512 ISA versions of vector math functions.
   Copyright (C) 2014-2021 Free Software Foundation, Inc.
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

#include "test-float-vlen16.h"
#include "test-math-vector-sincos.h"
#include <immintrin.h>

#define VEC_TYPE __m512

VECTOR_WRAPPER (WRAPPER_NAME (cosf), _ZGVeN16v_cosf)
VECTOR_WRAPPER (WRAPPER_NAME (sinf), _ZGVeN16v_sinf)
VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVeN16v_logf)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVeN16v_expf)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVeN16vv_powf)
VECTOR_WRAPPER (WRAPPER_NAME (acosf), _ZGVeN16v_acosf)
VECTOR_WRAPPER (WRAPPER_NAME (atanf), _ZGVeN16v_atanf)
VECTOR_WRAPPER (WRAPPER_NAME (asinf), _ZGVeN16v_asinf)
VECTOR_WRAPPER_ff (WRAPPER_NAME (hypotf), _ZGVeN16vv_hypotf)
VECTOR_WRAPPER (WRAPPER_NAME (exp2f), _ZGVeN16v_exp2f)
VECTOR_WRAPPER (WRAPPER_NAME (exp10f), _ZGVeN16v_exp10f)

#define VEC_INT_TYPE __m512i

#ifndef __ILP32__
VECTOR_WRAPPER_fFF_3 (WRAPPER_NAME (sincosf), _ZGVeN16vvv_sincosf)
#else
VECTOR_WRAPPER_fFF_2 (WRAPPER_NAME (sincosf), _ZGVeN16vvv_sincosf)
#endif
