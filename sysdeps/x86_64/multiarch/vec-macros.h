/* Macro helpers for VEC_{type}({vec_num})
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#ifndef _VEC_MACROS_H
#define _VEC_MACROS_H			1

#ifndef VEC_SIZE
# error "Never include this file directly. Always include a vector config."
#endif

/* Defines so we can use SSE2 / AVX2 / EVEX / EVEX512 encoding with same
   VEC(N) values.  */
#define VEC_hi_xmm0				xmm16
#define VEC_hi_xmm1				xmm17
#define VEC_hi_xmm2				xmm18
#define VEC_hi_xmm3				xmm19
#define VEC_hi_xmm4				xmm20
#define VEC_hi_xmm5				xmm21
#define VEC_hi_xmm6				xmm22
#define VEC_hi_xmm7				xmm23
#define VEC_hi_xmm8				xmm24
#define VEC_hi_xmm9				xmm25
#define VEC_hi_xmm10			xmm26
#define VEC_hi_xmm11			xmm27
#define VEC_hi_xmm12			xmm28
#define VEC_hi_xmm13			xmm29
#define VEC_hi_xmm14			xmm30
#define VEC_hi_xmm15			xmm31

#define VEC_hi_ymm0				ymm16
#define VEC_hi_ymm1				ymm17
#define VEC_hi_ymm2				ymm18
#define VEC_hi_ymm3				ymm19
#define VEC_hi_ymm4				ymm20
#define VEC_hi_ymm5				ymm21
#define VEC_hi_ymm6				ymm22
#define VEC_hi_ymm7				ymm23
#define VEC_hi_ymm8				ymm24
#define VEC_hi_ymm9				ymm25
#define VEC_hi_ymm10			ymm26
#define VEC_hi_ymm11			ymm27
#define VEC_hi_ymm12			ymm28
#define VEC_hi_ymm13			ymm29
#define VEC_hi_ymm14			ymm30
#define VEC_hi_ymm15			ymm31

#define VEC_hi_zmm0				zmm16
#define VEC_hi_zmm1				zmm17
#define VEC_hi_zmm2				zmm18
#define VEC_hi_zmm3				zmm19
#define VEC_hi_zmm4				zmm20
#define VEC_hi_zmm5				zmm21
#define VEC_hi_zmm6				zmm22
#define VEC_hi_zmm7				zmm23
#define VEC_hi_zmm8				zmm24
#define VEC_hi_zmm9				zmm25
#define VEC_hi_zmm10			zmm26
#define VEC_hi_zmm11			zmm27
#define VEC_hi_zmm12			zmm28
#define VEC_hi_zmm13			zmm29
#define VEC_hi_zmm14			zmm30
#define VEC_hi_zmm15			zmm31

#define PRIMITIVE_VEC(vec, num)		vec##num

#define VEC_any_xmm(i)			PRIMITIVE_VEC(xmm, i)
#define VEC_any_ymm(i)			PRIMITIVE_VEC(ymm, i)
#define VEC_any_zmm(i)			PRIMITIVE_VEC(zmm, i)

#define VEC_hi_xmm(i)			PRIMITIVE_VEC(VEC_hi_xmm, i)
#define VEC_hi_ymm(i)			PRIMITIVE_VEC(VEC_hi_ymm, i)
#define VEC_hi_zmm(i)			PRIMITIVE_VEC(VEC_hi_zmm, i)

#endif
