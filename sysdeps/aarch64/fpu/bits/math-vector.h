/* Platform-specific SIMD declarations of math functions.

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

#ifndef _MATH_H
#  error "Never include <bits/math-vector.h> directly;\
 include <math.h> instead."
#endif

/* Get default empty definitions for simd declarations.  */
#include <bits/libm-simd-decl-stubs.h>

#if __GNUC_PREREQ(9, 0)
#  define __ADVSIMD_VEC_MATH_SUPPORTED
typedef __Float32x4_t __f32x4_t;
typedef __Float64x2_t __f64x2_t;
#elif __glibc_clang_prereq(8, 0)
#  define __ADVSIMD_VEC_MATH_SUPPORTED
typedef __attribute__ ((__neon_vector_type__ (4))) float __f32x4_t;
typedef __attribute__ ((__neon_vector_type__ (2))) double __f64x2_t;
#endif

#if __GNUC_PREREQ(10, 0) || __glibc_clang_prereq(11, 0)
#  define __SVE_VEC_MATH_SUPPORTED
typedef __SVFloat32_t __sv_f32_t;
typedef __SVFloat64_t __sv_f64_t;
typedef __SVBool_t __sv_bool_t;
#endif

/* If vector types and vector PCS are unsupported in the working
   compiler, no choice but to omit vector math declarations.  */

#ifdef __ADVSIMD_VEC_MATH_SUPPORTED

#  define __vpcs __attribute__ ((__aarch64_vector_pcs__))

__vpcs __f32x4_t _ZGVnN4vv_atan2f (__f32x4_t, __f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_acosf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_asinf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_atanf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_cosf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_expf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_exp10f (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_exp2f (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_logf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_log10f (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_log1pf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_log2f (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_sinf (__f32x4_t);
__vpcs __f32x4_t _ZGVnN4v_tanf (__f32x4_t);

__vpcs __f64x2_t _ZGVnN2vv_atan2 (__f64x2_t, __f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_acos (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_asin (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_atan (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_cos (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_exp (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_exp10 (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_exp2 (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_log (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_log10 (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_log1p (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_log2 (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_sin (__f64x2_t);
__vpcs __f64x2_t _ZGVnN2v_tan (__f64x2_t);

#  undef __ADVSIMD_VEC_MATH_SUPPORTED
#endif /* __ADVSIMD_VEC_MATH_SUPPORTED */

#ifdef __SVE_VEC_MATH_SUPPORTED

__sv_f32_t _ZGVsMxvv_atan2f (__sv_f32_t, __sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_acosf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_asinf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_atanf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_cosf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_expf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_exp10f (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_exp2f (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_logf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_log10f (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_log1pf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_log2f (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_sinf (__sv_f32_t, __sv_bool_t);
__sv_f32_t _ZGVsMxv_tanf (__sv_f32_t, __sv_bool_t);

__sv_f64_t _ZGVsMxvv_atan2 (__sv_f64_t, __sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_acos (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_asin (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_atan (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_cos (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_exp (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_exp10 (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_exp2 (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_log (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_log10 (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_log1p (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_log2 (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_sin (__sv_f64_t, __sv_bool_t);
__sv_f64_t _ZGVsMxv_tan (__sv_f64_t, __sv_bool_t);

#  undef __SVE_VEC_MATH_SUPPORTED
#endif /* __SVE_VEC_MATH_SUPPORTED */
