/* Function atanhf vectorized with AVX2.
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
 *
 *   Special cases:
 *
 *   atanh(0)  = 0
 *   atanh(+1) = +INF
 *   atanh(-1) = -INF
 *   atanh(x)  = NaN if |x| > 1, or if x is a NaN or INF
 *
 */

/* Offsets for data table __svml_satanh_data_internal
 */
#define SgnMask                       	0
#define sOne                          	32
#define sPoly                         	64
#define iBrkValue                     	320
#define iOffExpoMask                  	352
#define sHalf                         	384
#define sSign                         	416
#define sTopMask12                    	448
#define TinyRange                     	480
#define sLn2                          	512

#include <sysdep.h>

        .text
	.section .text.avx2,"ax",@progbits
ENTRY(_ZGVdN8v_atanhf_avx2)
        pushq     %rbp
        cfi_def_cfa_offset(16)
        movq      %rsp, %rbp
        cfi_def_cfa(6, 16)
        cfi_offset(6, -16)
        andq      $-32, %rsp
        subq      $96, %rsp

/* Load constants including One = 1 */
        vmovups   sOne+__svml_satanh_data_internal(%rip), %ymm5
        vmovups   sTopMask12+__svml_satanh_data_internal(%rip), %ymm13
        vmovaps   %ymm0, %ymm6

/* Strip off the sign, so treat X as positive until right at the end */
        vandps    SgnMask+__svml_satanh_data_internal(%rip), %ymm6, %ymm10
        vsubps    %ymm10, %ymm5, %ymm1

/*
 * Compute V = 2 * X trivially, and UHi + U_lo = 1 - X in two pieces,
 * the upper part UHi being <= 12 bits long. Then we have
 * atanh(X) = 1/2 * log((1 + X) / (1 - X)) = 1/2 * log1p(V / (UHi + ULo)).
 */
        vaddps    %ymm10, %ymm10, %ymm14

/*
 * Check whether |X| < 1, in which case we use the main function.
 * Otherwise set the rangemask so that the callout will get used.
 * Note that this will also use the callout for NaNs since not(NaN < 1).
 */
        vcmpnlt_uqps %ymm5, %ymm10, %ymm7
        vsubps    %ymm1, %ymm5, %ymm9
        vcmplt_oqps TinyRange+__svml_satanh_data_internal(%rip), %ymm10, %ymm4
        vrcpps    %ymm1, %ymm11
        vsubps    %ymm10, %ymm9, %ymm12
        vandps    %ymm13, %ymm11, %ymm0

/* No need to split sU when FMA is available */
        vfnmadd213ps %ymm5, %ymm0, %ymm1
        vmovaps   %ymm6, %ymm8
        vfmadd213ps %ymm6, %ymm6, %ymm8
        vfnmadd231ps %ymm0, %ymm12, %ymm1

/*
 * Split V as well into upper 12 bits and lower part, so that we can get
 * a preliminary quotient estimate without rounding error.
 */
        vandps    %ymm13, %ymm14, %ymm15
        vmovmskps %ymm7, %edx
        vsubps    %ymm15, %ymm14, %ymm7

/* Hence get initial quotient estimate QHi + QLo = R * VHi + R * VLo */
        vmulps    %ymm15, %ymm0, %ymm10

/* Compute D = E + E^2 */
        vfmadd213ps %ymm1, %ymm1, %ymm1

/* Record the sign for eventual reincorporation. */
        vandps    sSign+__svml_satanh_data_internal(%rip), %ymm6, %ymm3

/* Or the sign bit in with the tiny result to handle atanh(-0) correctly */
        vorps     %ymm3, %ymm8, %ymm2
        vmulps    %ymm7, %ymm0, %ymm8

/*
 * Compute R * (VHi + VLo) * (1 + E + E^2)
 * = R *  (VHi + VLo) * (1 + D)
 * = QHi + (QHi * D + QLo + QLo * D)
 */
        vmulps    %ymm1, %ymm10, %ymm9
        vfmadd213ps %ymm8, %ymm8, %ymm1
        vaddps    %ymm1, %ymm9, %ymm1

/* reduction: compute r,n */
        vmovups   iBrkValue+__svml_satanh_data_internal(%rip), %ymm9

/*
 * Now finally accumulate the high and low parts of the
 * argument to log1p, H + L, with a final compensated summation.
 */
        vaddps    %ymm1, %ymm10, %ymm12
        vsubps    %ymm12, %ymm10, %ymm11

/*
 * Now we feed into the log1p code, using H in place of _VARG1 and
 * later incorporating L into the reduced argument.
 * compute 1+x as high, low parts
 */
        vmaxps    %ymm12, %ymm5, %ymm13
        vminps    %ymm12, %ymm5, %ymm14
        vaddps    %ymm11, %ymm1, %ymm0
        vaddps    %ymm14, %ymm13, %ymm1
        vpsubd    %ymm9, %ymm1, %ymm7
        vsubps    %ymm1, %ymm13, %ymm15
        vpsrad    $23, %ymm7, %ymm10
        vpand     iOffExpoMask+__svml_satanh_data_internal(%rip), %ymm7, %ymm8
        vaddps    %ymm15, %ymm14, %ymm13
        vpslld    $23, %ymm10, %ymm11
        vpaddd    %ymm9, %ymm8, %ymm15
        vaddps    %ymm13, %ymm0, %ymm14
        vcvtdq2ps %ymm10, %ymm0
        vpsubd    %ymm11, %ymm5, %ymm12

/* polynomial evaluation */
        vsubps    %ymm5, %ymm15, %ymm5
        vmulps    %ymm14, %ymm12, %ymm1
        vaddps    %ymm5, %ymm1, %ymm5
        vmovups   sPoly+224+__svml_satanh_data_internal(%rip), %ymm1
        vfmadd213ps sPoly+192+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+160+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+128+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+96+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+64+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+32+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vfmadd213ps sPoly+__svml_satanh_data_internal(%rip), %ymm5, %ymm1
        vmulps    %ymm1, %ymm5, %ymm7
        vfmadd213ps %ymm5, %ymm5, %ymm7

/* final reconstruction */
        vfmadd132ps sLn2+__svml_satanh_data_internal(%rip), %ymm7, %ymm0

/* Finally, halve the result and reincorporate the sign */
        vxorps    sHalf+__svml_satanh_data_internal(%rip), %ymm3, %ymm3
        vmulps    %ymm0, %ymm3, %ymm0
        vblendvps %ymm4, %ymm2, %ymm0, %ymm0
        testl     %edx, %edx

/* Go to special inputs processing branch */
        jne       L(SPECIAL_VALUES_BRANCH)
                                # LOE rbx r12 r13 r14 r15 edx ymm0 ymm6

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
        vmovups   %ymm6, 32(%rsp)
        vmovups   %ymm0, 64(%rsp)
                                # LOE rbx r12 r13 r14 r15 edx ymm0

        xorl      %eax, %eax
                                # LOE rbx r12 r13 r14 r15 eax edx

        vzeroupper
        movq      %r12, 16(%rsp)
        /*  DW_CFA_expression: r12 (r12) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -80; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0c, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xb0, 0xff, 0xff, 0xff, 0x22
        movl      %eax, %r12d
        movq      %r13, 8(%rsp)
        /*  DW_CFA_expression: r13 (r13) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -88; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0d, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xa8, 0xff, 0xff, 0xff, 0x22
        movl      %edx, %r13d
        movq      %r14, (%rsp)
        /*  DW_CFA_expression: r14 (r14) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -96; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0e, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xa0, 0xff, 0xff, 0xff, 0x22
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
        cmpl      $8, %r12d

/* Check bits in range mask */
        jl        L(RANGEMASK_CHECK)
                                # LOE rbx r15 r12d r13d

        movq      16(%rsp), %r12
        cfi_restore(12)
        movq      8(%rsp), %r13
        cfi_restore(13)
        movq      (%rsp), %r14
        cfi_restore(14)
        vmovups   64(%rsp), %ymm0

/* Go to exit */
        jmp       L(EXIT)
        /*  DW_CFA_expression: r12 (r12) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -80; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0c, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xb0, 0xff, 0xff, 0xff, 0x22
        /*  DW_CFA_expression: r13 (r13) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -88; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0d, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xa8, 0xff, 0xff, 0xff, 0x22
        /*  DW_CFA_expression: r14 (r14) (DW_OP_lit8; DW_OP_minus; DW_OP_const4s: -32; DW_OP_and; DW_OP_const4s: -96; DW_OP_plus)  */
        .cfi_escape 0x10, 0x0e, 0x0e, 0x38, 0x1c, 0x0d, 0xe0, 0xff, 0xff, 0xff, 0x1a, 0x0d, 0xa0, 0xff, 0xff, 0xff, 0x22
                                # LOE rbx r12 r13 r14 r15 ymm0

/* Scalar math fucntion call
 * to process special input
 */

L(SCALAR_MATH_CALL):
        movl      %r12d, %r14d
        movss     32(%rsp,%r14,4), %xmm0
        call      atanhf@PLT
                                # LOE rbx r14 r15 r12d r13d xmm0

        movss     %xmm0, 64(%rsp,%r14,4)

/* Process special inputs in loop */
        jmp       L(SPECIAL_VALUES_LOOP)
                                # LOE rbx r15 r12d r13d
END(_ZGVdN8v_atanhf_avx2)

        .section .rodata, "a"
        .align 32

#ifdef __svml_satanh_data_internal_typedef
typedef unsigned int VUINT32;
typedef struct {
        __declspec(align(32)) VUINT32 SgnMask[8][1];
        __declspec(align(32)) VUINT32 sOne[8][1];
        __declspec(align(32)) VUINT32 sPoly[8][8][1];
        __declspec(align(32)) VUINT32 iBrkValue[8][1];
        __declspec(align(32)) VUINT32 iOffExpoMask[8][1];
        __declspec(align(32)) VUINT32 sHalf[8][1];
        __declspec(align(32)) VUINT32 sSign[8][1];
        __declspec(align(32)) VUINT32 sTopMask12[8][1];
        __declspec(align(32)) VUINT32 TinyRange[8][1];
        __declspec(align(32)) VUINT32 sLn2[8][1];
} __svml_satanh_data_internal;
#endif
__svml_satanh_data_internal:
        /*== SgnMask ==*/
        .long 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
        /*== sOne = SP 1.0 ==*/
        .align 32
        .long 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
        /*== sPoly[] = SP polynomial ==*/
        .align 32
        .long 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000 /* -5.0000000000000000000000000e-01 P0 */
        .long 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94 /*  3.3333265781402587890625000e-01 P1 */
        .long 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e /* -2.5004237890243530273437500e-01 P2 */
        .long 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190 /*  2.0007920265197753906250000e-01 P3 */
        .long 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37 /* -1.6472326219081878662109375e-01 P4 */
        .long 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12 /*  1.4042308926582336425781250e-01 P5 */
        .long 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3 /* -1.5122179687023162841796875e-01 P6 */
        .long 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed /*  1.3820238411426544189453125e-01 P7 */
        /*== iBrkValue = SP 2/3 ==*/
        .align 32
        .long 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab
        /*== iOffExpoMask = SP significand mask ==*/
        .align 32
        .long 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff
        /*== sHalf ==*/
        .align 32
        .long 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000
        /*== sSign ==*/
        .align 32
        .long 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000
        /*== sTopMask12 ==*/
        .align 32
        .long 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000
        /*== TinyRange ==*/
        .align 32
        .long 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000
        /*== sLn2 = SP ln(2) ==*/
        .align 32
        .long 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218
        .align 32
        .type	__svml_satanh_data_internal,@object
        .size	__svml_satanh_data_internal,.-__svml_satanh_data_internal
