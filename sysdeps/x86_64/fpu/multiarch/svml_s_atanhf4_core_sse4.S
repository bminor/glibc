/* Function atanhf vectorized with SSE4.
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
#define sOne                          	16
#define sPoly                         	32
#define iBrkValue                     	160
#define iOffExpoMask                  	176
#define sHalf                         	192
#define sSign                         	208
#define sTopMask12                    	224
#define TinyRange                     	240
#define sLn2                          	256

#include <sysdep.h>

        .text
	.section .text.sse4,"ax",@progbits
ENTRY(_ZGVbN4v_atanhf_sse4)
        subq      $72, %rsp
        cfi_def_cfa_offset(80)
        movaps    %xmm0, %xmm5

/* Load constants including One = 1 */
        movups    sOne+__svml_satanh_data_internal(%rip), %xmm4
        movaps    %xmm5, %xmm3

/* Strip off the sign, so treat X as positive until right at the end */
        movups    SgnMask+__svml_satanh_data_internal(%rip), %xmm7
        movaps    %xmm4, %xmm8
        andps     %xmm5, %xmm7
        movaps    %xmm4, %xmm10
        movups    sTopMask12+__svml_satanh_data_internal(%rip), %xmm11
        movaps    %xmm4, %xmm14
        movaps    %xmm11, %xmm9

/*
 * Compute V = 2 * X trivially, and UHi + U_lo = 1 - X in two pieces,
 * the upper part UHi being <= 12 bits long. Then we have
 * atanh(X) = 1/2 * log((1 + X) / (1 - X)) = 1/2 * log1p(V / (UHi + ULo)).
 */
        movaps    %xmm7, %xmm12

/*
 * Check whether |X| < 1, in which case we use the main function.
 * Otherwise set the rangemask so that the callout will get used.
 * Note that this will also use the callout for NaNs since not(NaN < 1).
 */
        movaps    %xmm7, %xmm6
        movaps    %xmm7, %xmm2
        cmpnltps  %xmm4, %xmm6
        cmpltps   TinyRange+__svml_satanh_data_internal(%rip), %xmm2
        mulps     %xmm5, %xmm3
        subps     %xmm7, %xmm8
        addps     %xmm7, %xmm12
        movmskps  %xmm6, %edx
        subps     %xmm8, %xmm10
        addps     %xmm5, %xmm3
        subps     %xmm7, %xmm10
        andps     %xmm8, %xmm9

/*
 * Now we feed into the log1p code, using H in place of _VARG1 and
 * later incorporating L into the reduced argument.
 * compute 1+x as high, low parts
 */
        movaps    %xmm4, %xmm7

/*
 * Now compute R = 1/(UHi+ULo) * (1 - E) and the error term E
 * The first FMR is exact (we force R to 12 bits just in case it
 * isn't already, to make absolutely sure), and since E is ~ 2^-12,
 * the rounding error in the other one is acceptable.
 */
        rcpps     %xmm9, %xmm15
        subps     %xmm9, %xmm8
        andps     %xmm11, %xmm15

/*
 * Split V as well into upper 12 bits and lower part, so that we can get
 * a preliminary quotient estimate without rounding error.
 */
        andps     %xmm12, %xmm11
        mulps     %xmm15, %xmm9
        addps     %xmm8, %xmm10
        subps     %xmm11, %xmm12

/* Hence get initial quotient estimate QHi + QLo = R * VHi + R * VLo */
        mulps     %xmm15, %xmm11
        mulps     %xmm15, %xmm10
        subps     %xmm9, %xmm14
        mulps     %xmm12, %xmm15
        subps     %xmm10, %xmm14

/* Compute D = E + E^2 */
        movaps    %xmm14, %xmm13
        movaps    %xmm4, %xmm8
        mulps     %xmm14, %xmm13

/* reduction: compute r,n */
        movdqu    iBrkValue+__svml_satanh_data_internal(%rip), %xmm9
        addps     %xmm13, %xmm14

/*
 * Compute R * (VHi + VLo) * (1 + E + E^2)
 * = R *  (VHi + VLo) * (1 + D)
 * = QHi + (QHi * D + QLo + QLo * D)
 */
        movaps    %xmm14, %xmm0
        mulps     %xmm15, %xmm14
        mulps     %xmm11, %xmm0
        addps     %xmm14, %xmm15
        movdqu    iOffExpoMask+__svml_satanh_data_internal(%rip), %xmm12
        movaps    %xmm4, %xmm14

/* Record the sign for eventual reincorporation. */
        movups    sSign+__svml_satanh_data_internal(%rip), %xmm1
        addps     %xmm15, %xmm0

/*
 * Now finally accumulate the high and low parts of the
 * argument to log1p, H + L, with a final compensated summation.
 */
        movaps    %xmm0, %xmm6
        andps     %xmm5, %xmm1

/* Or the sign bit in with the tiny result to handle atanh(-0) correctly */
        orps      %xmm1, %xmm3
        addps     %xmm11, %xmm6
        maxps     %xmm6, %xmm7
        minps     %xmm6, %xmm8
        subps     %xmm6, %xmm11
        movaps    %xmm7, %xmm10
        andps     %xmm2, %xmm3
        addps     %xmm8, %xmm10
        addps     %xmm11, %xmm0
        subps     %xmm10, %xmm7
        psubd     %xmm9, %xmm10
        addps     %xmm7, %xmm8
        pand      %xmm10, %xmm12
        psrad     $23, %xmm10
        cvtdq2ps  %xmm10, %xmm13
        addps     %xmm8, %xmm0

/* final reconstruction */
        mulps     sLn2+__svml_satanh_data_internal(%rip), %xmm13
        pslld     $23, %xmm10
        paddd     %xmm9, %xmm12
        psubd     %xmm10, %xmm14

/* polynomial evaluation */
        subps     %xmm4, %xmm12
        mulps     %xmm0, %xmm14
        movups    sPoly+112+__svml_satanh_data_internal(%rip), %xmm0
        addps     %xmm12, %xmm14
        mulps     %xmm14, %xmm0

/* Finally, halve the result and reincorporate the sign */
        movups    sHalf+__svml_satanh_data_internal(%rip), %xmm4
        pxor      %xmm1, %xmm4
        addps     sPoly+96+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+80+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+64+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+48+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+32+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+16+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        addps     sPoly+__svml_satanh_data_internal(%rip), %xmm0
        mulps     %xmm14, %xmm0
        mulps     %xmm14, %xmm0
        addps     %xmm0, %xmm14
        movaps    %xmm2, %xmm0
        addps     %xmm13, %xmm14
        mulps     %xmm14, %xmm4
        andnps    %xmm4, %xmm0
        orps      %xmm3, %xmm0
        testl     %edx, %edx

/* Go to special inputs processing branch */
        jne       L(SPECIAL_VALUES_BRANCH)
                                # LOE rbx rbp r12 r13 r14 r15 edx xmm0 xmm5

/* Restore registers
 * and exit the function
 */

L(EXIT):
        addq      $72, %rsp
        cfi_def_cfa_offset(8)
        ret
        cfi_def_cfa_offset(80)

/* Branch to process
 * special inputs
 */

L(SPECIAL_VALUES_BRANCH):
        movups    %xmm5, 32(%rsp)
        movups    %xmm0, 48(%rsp)
                                # LOE rbx rbp r12 r13 r14 r15 edx

        xorl      %eax, %eax
        movq      %r12, 16(%rsp)
        cfi_offset(12, -64)
        movl      %eax, %r12d
        movq      %r13, 8(%rsp)
        cfi_offset(13, -72)
        movl      %edx, %r13d
        movq      %r14, (%rsp)
        cfi_offset(14, -80)
                                # LOE rbx rbp r15 r12d r13d

/* Range mask
 * bits check
 */

L(RANGEMASK_CHECK):
        btl       %r12d, %r13d

/* Call scalar math function */
        jc        L(SCALAR_MATH_CALL)
                                # LOE rbx rbp r15 r12d r13d

/* Special inputs
 * processing loop
 */

L(SPECIAL_VALUES_LOOP):
        incl      %r12d
        cmpl      $4, %r12d

/* Check bits in range mask */
        jl        L(RANGEMASK_CHECK)
                                # LOE rbx rbp r15 r12d r13d

        movq      16(%rsp), %r12
        cfi_restore(12)
        movq      8(%rsp), %r13
        cfi_restore(13)
        movq      (%rsp), %r14
        cfi_restore(14)
        movups    48(%rsp), %xmm0

/* Go to exit */
        jmp       L(EXIT)
        cfi_offset(12, -64)
        cfi_offset(13, -72)
        cfi_offset(14, -80)
                                # LOE rbx rbp r12 r13 r14 r15 xmm0

/* Scalar math fucntion call
 * to process special input
 */

L(SCALAR_MATH_CALL):
        movl      %r12d, %r14d
        movss     32(%rsp,%r14,4), %xmm0
        call      atanhf@PLT
                                # LOE rbx rbp r14 r15 r12d r13d xmm0

        movss     %xmm0, 48(%rsp,%r14,4)

/* Process special inputs in loop */
        jmp       L(SPECIAL_VALUES_LOOP)
                                # LOE rbx rbp r15 r12d r13d
END(_ZGVbN4v_atanhf_sse4)

        .section .rodata, "a"
        .align 16

#ifdef __svml_satanh_data_internal_typedef
typedef unsigned int VUINT32;
typedef struct {
        __declspec(align(16)) VUINT32 SgnMask[4][1];
        __declspec(align(16)) VUINT32 sOne[4][1];
        __declspec(align(16)) VUINT32 sPoly[8][4][1];
        __declspec(align(16)) VUINT32 iBrkValue[4][1];
        __declspec(align(16)) VUINT32 iOffExpoMask[4][1];
        __declspec(align(16)) VUINT32 sHalf[4][1];
        __declspec(align(16)) VUINT32 sSign[4][1];
        __declspec(align(16)) VUINT32 sTopMask12[4][1];
        __declspec(align(16)) VUINT32 TinyRange[4][1];
        __declspec(align(16)) VUINT32 sLn2[4][1];
} __svml_satanh_data_internal;
#endif
__svml_satanh_data_internal:
        /*== SgnMask ==*/
        .long 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
        /*== sOne = SP 1.0 ==*/
        .align 16
        .long 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000
        /*== sPoly[] = SP polynomial ==*/
        .align 16
        .long 0xbf000000, 0xbf000000, 0xbf000000, 0xbf000000 /* -5.0000000000000000000000000e-01 P0 */
        .long 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94, 0x3eaaaa94 /*  3.3333265781402587890625000e-01 P1 */
        .long 0xbe80058e, 0xbe80058e, 0xbe80058e, 0xbe80058e /* -2.5004237890243530273437500e-01 P2 */
        .long 0x3e4ce190, 0x3e4ce190, 0x3e4ce190, 0x3e4ce190 /*  2.0007920265197753906250000e-01 P3 */
        .long 0xbe28ad37, 0xbe28ad37, 0xbe28ad37, 0xbe28ad37 /* -1.6472326219081878662109375e-01 P4 */
        .long 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12, 0x3e0fcb12 /*  1.4042308926582336425781250e-01 P5 */
        .long 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3, 0xbe1ad9e3 /* -1.5122179687023162841796875e-01 P6 */
        .long 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed, 0x3e0d84ed /*  1.3820238411426544189453125e-01 P7 */
        /*== iBrkValue = SP 2/3 ==*/
        .align 16
        .long 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab, 0x3f2aaaab
        /*== iOffExpoMask = SP significand mask ==*/
        .align 16
        .long 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff
        /*== sHalf ==*/
        .align 16
        .long 0x3F000000, 0x3F000000, 0x3F000000, 0x3F000000
        /*== sSign ==*/
        .align 16
        .long 0x80000000, 0x80000000, 0x80000000, 0x80000000
        /*== sTopMask12 ==*/
        .align 16
        .long 0xFFFFF000, 0xFFFFF000, 0xFFFFF000, 0xFFFFF000
        /*== TinyRange ==*/
        .align 16
        .long 0x0C000000, 0x0C000000, 0x0C000000, 0x0C000000
        /*== sLn2 = SP ln(2) ==*/
        .align 16
        .long 0x3f317218, 0x3f317218, 0x3f317218, 0x3f317218
        .align 16
        .type	__svml_satanh_data_internal,@object
        .size	__svml_satanh_data_internal,.-__svml_satanh_data_internal
