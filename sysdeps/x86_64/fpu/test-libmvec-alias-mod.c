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

#include "test-double-vlen2.h"
#define VEC_TYPE __m128d

VECTOR_WRAPPER (WRAPPER_NAME (log), _ZGVbN2v___log_finite)
VECTOR_WRAPPER (WRAPPER_NAME (exp), _ZGVbN2v___exp_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (pow), _ZGVbN2vv___pow_finite)

#undef VEC_TYPE
#undef VECTOR_WRAPPER
#undef VECTOR_WRAPPER_ff
#undef VEC_SUFF
#undef VEC_LEN

#include "test-double-vlen4.h"
#define VEC_TYPE __m256d

VECTOR_WRAPPER (WRAPPER_NAME (log), _ZGVcN4v___log_finite)
VECTOR_WRAPPER (WRAPPER_NAME (exp), _ZGVcN4v___exp_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (pow), _ZGVcN4vv___pow_finite)

#undef VEC_SUFF
#define VEC_SUFF _vlen4_avx2

VECTOR_WRAPPER (WRAPPER_NAME (log), _ZGVdN4v___log_finite)
VECTOR_WRAPPER (WRAPPER_NAME (exp), _ZGVdN4v___exp_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (pow), _ZGVdN4vv___pow_finite)

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

#include "test-float-vlen4.h"
#define VEC_TYPE __m128

VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVbN4v___logf_finite)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVbN4v___expf_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVbN4vv___powf_finite)

#undef VEC_TYPE
#undef VECTOR_WRAPPER
#undef VECTOR_WRAPPER_ff
#undef VEC_SUFF
#undef VEC_LEN

#include "test-float-vlen8.h"
#define VEC_TYPE __m256

VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVcN8v___logf_finite)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVcN8v___expf_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVcN8vv___powf_finite)

#undef VEC_SUFF
#define VEC_SUFF _vlen8_avx2

VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVdN8v___logf_finite)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVdN8v___expf_finite)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVdN8vv___powf_finite)
