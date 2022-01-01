/* Function atanhf vectorized with AVX-512.
   Copyright (C) 2021-2022 Free Software Foundation, Inc.
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
   https://www.gnu.org/licenses/.  */

/*
 * ALGORITHM DESCRIPTION:
 *
 *   Compute atanh(x) as 0.5 * log((1 + x)/(1 - x))
 *   using small lookup table that map to AVX-512 permute instructions
 *
 *   Special cases:
 *
 *   atanh(0)  = 0
 *   atanh(+1) = +INF
 *   atanh(-1) = -INF
 *   atanh(x)  = NaN if |x| > 1, or if x is a NaN or INF
 *
 */

/* Offsets for data table __svml_satanh_data_internal_avx512
 */
#define Log_tbl_H                     	0
#define Log_tbl_L                     	128
#define One                           	256
#define AbsMask                       	320
#define AddB5                         	384
#define RcpBitMask                    	448
#define poly_coeff3                   	512
#define poly_coeff2                   	576
#define poly_coeff1                   	640
#define poly_coeff0                   	704
#define Half                          	768
#define L2H                           	832
#define L2L                           	896

#include <sysdep.h>

        .text
	.section .text.exex512,"ax",@progbits
ENTRY(_ZGVeN16v_atanhf_skx)
        pushq     %rbp
        cfi_def_cfa_offset(16)
        movq      %rsp, %rbp
        cfi_def_cfa(6, 16)
        cfi_offset(6, -16)
        andq      $-64, %rsp
        subq      $192, %rsp
        vmovups   One+__svml_satanh_data_internal_avx512(%rip), %zmm4

/* round reciprocals to 1+5b mantissas */
        vmovups   AddB5+__svml_satanh_data_internal_avx512(%rip), %zmm14
        vmovups   RcpBitMask+__svml_satanh_data_internal_avx512(%rip), %zmm1
        vmovaps   %zmm0, %zmm11
        vandps    AbsMask+__svml_satanh_data_internal_avx512(%rip), %zmm11, %zmm6

/* 1+y */
        vaddps    {rn-sae}, %zmm4, %zmm6, %zmm9

/* 1-y */
        vsubps    {rn-sae}, %zmm6, %zmm4, %zmm8
        vxorps    %zmm6, %zmm11, %zmm10

/* Yp_high */
        vsubps    {rn-sae}, %zmm4, %zmm9, %zmm2

/* -Ym_high */
        vsubps    {rn-sae}, %zmm4, %zmm8, %zmm5

/* RcpP ~ 1/Yp */
        vrcp14ps  %zmm9, %zmm12

/* RcpM ~ 1/Ym */
        vrcp14ps  %zmm8, %zmm13

/* input outside (-1, 1) ? */
        vcmpps    $21, {sae}, %zmm4, %zmm6, %k0
        vpaddd    %zmm14, %zmm12, %zmm15
        vpaddd    %zmm14, %zmm13, %zmm0

/* Yp_low */
        vsubps    {rn-sae}, %zmm2, %zmm6, %zmm3
        vandps    %zmm1, %zmm15, %zmm7
        vandps    %zmm1, %zmm0, %zmm12

/* Ym_low */
        vaddps    {rn-sae}, %zmm5, %zmm6, %zmm5

/* Reduced argument: Rp = (RcpP*Yp - 1)+RcpP*Yp_low */
        vfmsub213ps {rn-sae}, %zmm4, %zmm7, %zmm9

/* Reduced argument: Rm = (RcpM*Ym - 1)+RcpM*Ym_low */
        vfmsub231ps {rn-sae}, %zmm12, %zmm8, %zmm4
        vmovups   Log_tbl_L+__svml_satanh_data_internal_avx512(%rip), %zmm8
        vmovups   Log_tbl_L+64+__svml_satanh_data_internal_avx512(%rip), %zmm13

/* exponents */
        vgetexpps {sae}, %zmm7, %zmm15
        vfmadd231ps {rn-sae}, %zmm7, %zmm3, %zmm9

/* Table lookups */
        vmovups   __svml_satanh_data_internal_avx512(%rip), %zmm6
        vgetexpps {sae}, %zmm12, %zmm14
        vfnmadd231ps {rn-sae}, %zmm12, %zmm5, %zmm4

/* Prepare table index */
        vpsrld    $18, %zmm7, %zmm3
        vpsrld    $18, %zmm12, %zmm2
        vmovups   Log_tbl_H+64+__svml_satanh_data_internal_avx512(%rip), %zmm7
        vmovups   poly_coeff1+__svml_satanh_data_internal_avx512(%rip), %zmm12

/* Km-Kp */
        vsubps    {rn-sae}, %zmm15, %zmm14, %zmm1
        kmovw     %k0, %edx
        vmovaps   %zmm3, %zmm0
        vpermi2ps %zmm13, %zmm8, %zmm3
        vpermt2ps %zmm13, %zmm2, %zmm8
        vpermi2ps %zmm7, %zmm6, %zmm0
        vpermt2ps %zmm7, %zmm2, %zmm6
        vsubps    {rn-sae}, %zmm3, %zmm8, %zmm5

/* K*L2H + Th */
        vmovups   L2H+__svml_satanh_data_internal_avx512(%rip), %zmm2

/* K*L2L + Tl */
        vmovups   L2L+__svml_satanh_data_internal_avx512(%rip), %zmm3

/* polynomials */
        vmovups   poly_coeff3+__svml_satanh_data_internal_avx512(%rip), %zmm7
        vmovups   poly_coeff0+__svml_satanh_data_internal_avx512(%rip), %zmm13

/* table values */
        vsubps    {rn-sae}, %zmm0, %zmm6, %zmm0
        vfmadd231ps {rn-sae}, %zmm1, %zmm2, %zmm0
        vfmadd213ps {rn-sae}, %zmm5, %zmm3, %zmm1
        vmovups   poly_coeff2+__svml_satanh_data_internal_avx512(%rip), %zmm3
        vmovaps   %zmm3, %zmm2
        vfmadd231ps {rn-sae}, %zmm9, %zmm7, %zmm2
        vfmadd231ps {rn-sae}, %zmm4, %zmm7, %zmm3
        vfmadd213ps {rn-sae}, %zmm12, %zmm9, %zmm2
        vfmadd213ps {rn-sae}, %zmm12, %zmm4, %zmm3
        vfmadd213ps {rn-sae}, %zmm13, %zmm9, %zmm2
        vfmadd213ps {rn-sae}, %zmm13, %zmm4, %zmm3

/* (K*L2L + Tl) + Rp*PolyP */
        vfmadd213ps {rn-sae}, %zmm1, %zmm9, %zmm2
        vorps     Half+__svml_satanh_data_internal_avx512(%rip), %zmm10, %zmm9

/* (K*L2L + Tl) + Rp*PolyP -Rm*PolyM */
        vfnmadd213ps {rn-sae}, %zmm2, %zmm4, %zmm3
        vaddps    {rn-sae}, %zmm3, %zmm0, %zmm4
        vmulps    {rn-sae}, %zmm9, %zmm4, %zmm0
        testl     %edx, %edx

/* Go to special inputs processing branch */
        jne       L(SPECIAL_VALUES_BRANCH)
                                # LOE rbx r12 r13 r14 r15 edx zmm0 zmm11

/* Restore registers
 * and exit the function
 */

L(EXIT):
        movq      %rbp, %rsp
        popq      %rbp
        cfi_def_cfa(7, 8)
        cfi_restore(6)
        ret
        cfi_def_cfa(6, 16)
        cfi_offset(6, -16)

/* Branch to process
 * special inputs
 */

L(SPECIAL_VALUES_BRANCH):
        vmovups   %zmm11, 64(%rsp)
        vmovups   %zmm0, 128(%rsp)
                                # LOE rbx r12 r13 r14 r15 edx zmm0

        xorl      %eax, %eax
                                # LOE rbx r12 r13 r14 r15 eax edx

        vzeroupper
        movq      %r12, 16(%rsp)
        /*  DW_CFA_expression: r12 (r12) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -176; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0c, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x50, 0xff, 0xff, 0xff, 0x22
        movl      %eax, %r12d
        movq      %r13, 8(%rsp)
        /*  DW_CFA_expression: r13 (r13) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -184; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0d, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x48, 0xff, 0xff, 0xff, 0x22
        movl      %edx, %r13d
        movq      %r14, (%rsp)
        /*  DW_CFA_expression: r14 (r14) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -192; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0e, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x40, 0xff, 0xff, 0xff, 0x22
                                # LOE rbx r15 r12d r13d

/* Range mask
 * bits check
 */

L(RANGEMASK_CHECK):
        btl       %r12d, %r13d

/* Call scalar math function */
        jc        L(SCALAR_MATH_CALL)
                                # LOE rbx r15 r12d r13d

/* Special inputs
 * processing loop
 */

L(SPECIAL_VALUES_LOOP):
        incl      %r12d
        cmpl      $16, %r12d

/* Check bits in range mask */
        jl        L(RANGEMASK_CHECK)
                                # LOE rbx r15 r12d r13d

        movq      16(%rsp), %r12
        cfi_restore(12)
        movq      8(%rsp), %r13
        cfi_restore(13)
        movq      (%rsp), %r14
        cfi_restore(14)
        vmovups   128(%rsp), %zmm0

/* Go to exit */
        jmp       L(EXIT)
        /*  DW_CFA_expression: r12 (r12) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -176; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0c, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x50, 0xff, 0xff, 0xff, 0x22
        /*  DW_CFA_expression: r13 (r13) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -184; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0d, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x48, 0xff, 0xff, 0xff, 0x22
        /*  DW_CFA_expression: r14 (r14) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -64; DW_OP_and; DW_OP_const4s: -192; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0e, 0x0e, 0x38, 0x1c, 0x0d, 0xc0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0x40, 0xff, 0xff, 0xff, 0x22
                                # LOE rbx r12 r13 r14 r15 zmm0

/* Scalar math fucntion call
 * to process special input
 */

L(SCALAR_MATH_CALL):
        movl      %r12d, %r14d
        movss     64(%rsp,%r14,4), %xmm0
        call      atanhf@PLT
                                # LOE rbx r14 r15 r12d r13d xmm0

        movss     %xmm0, 128(%rsp,%r14,4)

/* Process special inputs in loop */
        jmp       L(SPECIAL_VALUES_LOOP)
                                # LOE rbx r15 r12d r13d
END(_ZGVeN16v_atanhf_skx)

        .section .rodata, "a"
        .align 64

#ifdef __svml_satanh_data_internal_avx512_typedef
typedef unsigned int VUINT32;
typedef struct {
        __declspec(align(64)) VUINT32 Log_tbl_H[32][1];
        __declspec(align(64)) VUINT32 Log_tbl_L[32][1];
        __declspec(align(64)) VUINT32 One[16][1];
        __declspec(align(64)) VUINT32 AbsMask[16][1];
        __declspec(align(64)) VUINT32 AddB5[16][1];
        __declspec(align(64)) VUINT32 RcpBitMask[16][1];
        __declspec(align(64)) VUINT32 poly_coeff3[16][1];
        __declspec(align(64)) VUINT32 poly_coeff2[16][1];
        __declspec(align(64)) VUINT32 poly_coeff1[16][1];
        __declspec(align(64)) VUINT32 poly_coeff0[16][1];
        __declspec(align(64)) VUINT32 Half[16][1];
        __declspec(align(64)) VUINT32 L2H[16][1];
        __declspec(align(64)) VUINT32 L2L[16][1];
    } __svml_satanh_data_internal_avx512;
#endif
__svml_satanh_data_internal_avx512:
        /*== Log_tbl_H ==*/
        .long 0x00000000
        .long 0x3cfc0000
        .long 0x3d780000
        .long 0x3db78000
        .long 0x3df10000
        .long 0x3e14c000
        .long 0x3e300000
        .long 0x3e4a8000
        .long 0x3e648000
        .long 0x3e7dc000
        .long 0x3e8b4000
        .long 0x3e974000
        .long 0x3ea30000
        .long 0x3eae8000
        .long 0x3eb9c000
        .long 0x3ec4e000
        .long 0x3ecfa000
        .long 0x3eda2000
        .long 0x3ee48000
        .long 0x3eeea000
        .long 0x3ef8a000
        .long 0x3f013000
        .long 0x3f05f000
        .long 0x3f0aa000
        .long 0x3f0f4000
        .long 0x3f13d000
        .long 0x3f184000
        .long 0x3f1ca000
        .long 0x3f20f000
        .long 0x3f252000
        .long 0x3f295000
        .long 0x3f2d7000
        /*== Log_tbl_L ==*/
        .align 64
        .long 0x00000000
        .long 0x3726c39e
        .long 0x38a30c01
        .long 0x37528ae5
        .long 0x38e0edc5
        .long 0xb8ab41f8
        .long 0xb7cf8f58
        .long 0x3896a73d
        .long 0xb5838656
        .long 0x380c36af
        .long 0xb8235454
        .long 0x3862bae1
        .long 0x38c5e10e
        .long 0x38dedfac
        .long 0x38ebfb5e
        .long 0xb8e63c9f
        .long 0xb85c1340
        .long 0x38777bcd
        .long 0xb6038656
        .long 0x37d40984
        .long 0xb8b85028
        .long 0xb8ad5a5a
        .long 0x3865c84a
        .long 0x38c3d2f5
        .long 0x383ebce1
        .long 0xb8a1ed76
        .long 0xb7a332c4
        .long 0xb779654f
        .long 0xb8602f73
        .long 0x38f85db0
        .long 0x37b4996f
        .long 0xb8bfb3ca
        /*== One ==*/
        .align 64
        .long 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
        /*== AbsMask ==*/
        .align 64
        .long 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
        /*== AddB5 ==*/
        .align 64
        .long 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000, 0x00020000
        /*== RcpBitMask ==*/
        .align 64
        .long 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000, 0xfffc0000
        /*== poly_coeff3 ==*/
        .align 64
        .long 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810, 0xbe800810
        /*== poly_coeff2 ==*/
        .align 64
        .long 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e, 0x3eaab11e
        /*== poly_coeff1 ==*/
        .align 64
        .long 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000
        /*== poly_coeff0 ==*/
        .align 64
        .long 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
        /*== Half ==*/
        .align 64
        .long 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000, 0x3f000000
        /*== L2H = log(2)_high ==*/
        .align 64
        .long 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000, 0x3f317000
        /*== L2L = log(2)_low ==*/
        .align 64
        .long 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4, 0x3805fdf4
        .align 64
        .type	__svml_satanh_data_internal_avx512,@object
        .size	__svml_satanh_data_internal_avx512,.-__svml_satanh_data_internal_avx512
