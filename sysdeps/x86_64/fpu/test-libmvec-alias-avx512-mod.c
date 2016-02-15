/* Part of test to build shared library to ensure link against
   *_finite aliases from libmvec.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <immintrin.h>

#include "test-double-vlen8.h"
#define VEC_TYPE __m512d

VECTOR_WRAPPER (WRAPPER_NAME (log), _ZGVeN8v___log_finite)
VECTOR_WRAPPER (WRAPPER_NAME (exp), _ZGVeN8v___exp_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (pow), _ZGVeN8vv___pow_finite)

#undef FUNC
#undef FLOAT
#undef BUILD_COMPLEX
#undef TEST_MSG
#undef CHOOSE
#undef FUNC_TEST
#undef VEC_TYPE
#undef VECTOR_WRAPPER
#undef VECTOR_WRAPPER_ff
#undef VEC_SUFF
#undef VEC_LEN

#include "test-float-vlen16.h"
#define VEC_TYPE __m512

VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVeN16v___logf_finite)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVeN16v___expf_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVeN16vv___powf_finite)
