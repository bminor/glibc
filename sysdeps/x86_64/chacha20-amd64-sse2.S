/* Optimized SSE2 implementation of ChaCha20 cipher.
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

/* chacha20-amd64-ssse3.S  -  SSSE3 implementation of ChaCha20 cipher

   Copyright (C) 2017-2019 Jussi Kivilinna <jussi.kivilinna@iki.fi>

   This file is part of Libgcrypt.

   Libgcrypt is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, see <https://www.gnu.org/licenses/>.
*/

/* Based on D. J. Bernstein reference implementation at
   http://cr.yp.to/chacha.html:

   chacha-regs.c version 20080118
   D. J. Bernstein
   Public domain.  */

#include <sysdep.h>
#include <isa-level.h>

#if MINIMUM_X86_ISA_LEVEL <= 2

#ifdef PIC
#  define rRIP (%rip)
#else
#  define rRIP
#endif

/* 'ret' instruction replacement for straight-line speculation mitigation */
#define ret_spec_stop \
        ret; int3;

/* register macros */
#define INPUT %rdi
#define DST   %rsi
#define SRC   %rdx
#define NBLKS %rcx
#define ROUND %eax

/* stack structure */
#define STACK_VEC_X12 (16)
#define STACK_VEC_X13 (16 + STACK_VEC_X12)
#define STACK_TMP     (16 + STACK_VEC_X13)
#define STACK_TMP1    (16 + STACK_TMP)
#define STACK_TMP2    (16 + STACK_TMP1)

#define STACK_MAX     (16 + STACK_TMP2)

/* vector registers */
#define X0 %xmm0
#define X1 %xmm1
#define X2 %xmm2
#define X3 %xmm3
#define X4 %xmm4
#define X5 %xmm5
#define X6 %xmm6
#define X7 %xmm7
#define X8 %xmm8
#define X9 %xmm9
#define X10 %xmm10
#define X11 %xmm11
#define X12 %xmm12
#define X13 %xmm13
#define X14 %xmm14
#define X15 %xmm15

/**********************************************************************
  helper macros
 **********************************************************************/

/* 4x4 32-bit integer matrix transpose */
#define TRANSPOSE_4x4(x0, x1, x2, x3, t1, t2, t3) \
	movdqa    x0, t2; \
	punpckhdq x1, t2; \
	punpckldq x1, x0; \
	\
	movdqa    x2, t1; \
	punpckldq x3, t1; \
	punpckhdq x3, x2; \
	\
	movdqa     x0, x1; \
	punpckhqdq t1, x1; \
	punpcklqdq t1, x0; \
	\
	movdqa     t2, x3; \
	punpckhqdq x2, x3; \
	punpcklqdq x2, t2; \
	movdqa     t2, x2;

/* fill xmm register with 32-bit value from memory */
#define PBROADCASTD(mem32, xreg) \
	movd mem32, xreg; \
	pshufd $0, xreg, xreg;

/**********************************************************************
  4-way chacha20
 **********************************************************************/

#define ROTATE2(v1,v2,c,tmp1,tmp2)	\
	movdqa v1, tmp1; 		\
	movdqa v2, tmp2; 		\
	psrld $(32 - (c)), v1;		\
	pslld $(c), tmp1;		\
	paddb tmp1, v1;			\
	psrld $(32 - (c)), v2;		\
	pslld $(c), tmp2;		\
	paddb tmp2, v2;

#define XOR(ds,s) \
	pxor s, ds;

#define PLUS(ds,s) \
	paddd s, ds;

#define QUARTERROUND2(a1,b1,c1,d1,a2,b2,c2,d2,ign,tmp1,tmp2)	\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE2(d1, d2, 16, tmp1, tmp2);			\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2, 12, tmp1, tmp2);			\
	PLUS(a1,b1); PLUS(a2,b2); XOR(d1,a1); XOR(d2,a2);	\
	    ROTATE2(d1, d2, 8, tmp1, tmp2);			\
	PLUS(c1,d1); PLUS(c2,d2); XOR(b1,c1); XOR(b2,c2);	\
	    ROTATE2(b1, b2,  7, tmp1, tmp2);

	.section .text.sse2,"ax",@progbits

chacha20_data:
	.align 16
L(counter1):
	.long 1,0,0,0
L(inc_counter):
	.long 0,1,2,3
L(unsigned_cmp):
	.long 0x80000000,0x80000000,0x80000000,0x80000000

	.hidden __chacha20_sse2_blocks4
ENTRY (__chacha20_sse2_blocks4)
	/* input:
	 *	%rdi: input
	 *	%rsi: dst
	 *	%rdx: src
	 *	%rcx: nblks (multiple of 4)
	 */

	pushq %rbp;
	cfi_adjust_cfa_offset(8);
	cfi_rel_offset(rbp, 0)
	movq %rsp, %rbp;
	cfi_def_cfa_register(%rbp);

	subq $STACK_MAX, %rsp;
	andq $~15, %rsp;

L(loop4):
	mov $20, ROUND;

	/* Construct counter vectors X12 and X13 */
	movdqa L(inc_counter) rRIP, X0;
	movdqa L(unsigned_cmp) rRIP, X2;
	PBROADCASTD((12 * 4)(INPUT), X12);
	PBROADCASTD((13 * 4)(INPUT), X13);
	paddd X0, X12;
	movdqa X12, X1;
	pxor X2, X0;
	pxor X2, X1;
	pcmpgtd X1, X0;
	psubd X0, X13;
	movdqa X12, (STACK_VEC_X12)(%rsp);
	movdqa X13, (STACK_VEC_X13)(%rsp);

	/* Load vectors */
	PBROADCASTD((0 * 4)(INPUT), X0);
	PBROADCASTD((1 * 4)(INPUT), X1);
	PBROADCASTD((2 * 4)(INPUT), X2);
	PBROADCASTD((3 * 4)(INPUT), X3);
	PBROADCASTD((4 * 4)(INPUT), X4);
	PBROADCASTD((5 * 4)(INPUT), X5);
	PBROADCASTD((6 * 4)(INPUT), X6);
	PBROADCASTD((7 * 4)(INPUT), X7);
	PBROADCASTD((8 * 4)(INPUT), X8);
	PBROADCASTD((9 * 4)(INPUT), X9);
	PBROADCASTD((10 * 4)(INPUT), X10);
	PBROADCASTD((11 * 4)(INPUT), X11);
	PBROADCASTD((14 * 4)(INPUT), X14);
	PBROADCASTD((15 * 4)(INPUT), X15);
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);

L(round2_4):
	QUARTERROUND2(X0, X4,  X8, X12,   X1, X5,  X9, X13, tmp:=,X11,X15)
	movdqa (STACK_TMP)(%rsp), X11;
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X8, (STACK_TMP)(%rsp);
	movdqa X9, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X6, X10, X14,   X3, X7, X11, X15, tmp:=,X8,X9)
	QUARTERROUND2(X0, X5, X10, X15,   X1, X6, X11, X12, tmp:=,X8,X9)
	movdqa (STACK_TMP)(%rsp), X8;
	movdqa (STACK_TMP1)(%rsp), X9;
	movdqa X11, (STACK_TMP)(%rsp);
	movdqa X15, (STACK_TMP1)(%rsp);
	QUARTERROUND2(X2, X7,  X8, X13,   X3, X4,  X9, X14, tmp:=,X11,X15)
	sub $2, ROUND;
	jnz L(round2_4);

	/* tmp := X15 */
	movdqa (STACK_TMP)(%rsp), X11;
	PBROADCASTD((0 * 4)(INPUT), X15);
	PLUS(X0, X15);
	PBROADCASTD((1 * 4)(INPUT), X15);
	PLUS(X1, X15);
	PBROADCASTD((2 * 4)(INPUT), X15);
	PLUS(X2, X15);
	PBROADCASTD((3 * 4)(INPUT), X15);
	PLUS(X3, X15);
	PBROADCASTD((4 * 4)(INPUT), X15);
	PLUS(X4, X15);
	PBROADCASTD((5 * 4)(INPUT), X15);
	PLUS(X5, X15);
	PBROADCASTD((6 * 4)(INPUT), X15);
	PLUS(X6, X15);
	PBROADCASTD((7 * 4)(INPUT), X15);
	PLUS(X7, X15);
	PBROADCASTD((8 * 4)(INPUT), X15);
	PLUS(X8, X15);
	PBROADCASTD((9 * 4)(INPUT), X15);
	PLUS(X9, X15);
	PBROADCASTD((10 * 4)(INPUT), X15);
	PLUS(X10, X15);
	PBROADCASTD((11 * 4)(INPUT), X15);
	PLUS(X11, X15);
	movdqa (STACK_VEC_X12)(%rsp), X15;
	PLUS(X12, X15);
	movdqa (STACK_VEC_X13)(%rsp), X15;
	PLUS(X13, X15);
	movdqa X13, (STACK_TMP)(%rsp);
	PBROADCASTD((14 * 4)(INPUT), X15);
	PLUS(X14, X15);
	movdqa (STACK_TMP1)(%rsp), X15;
	movdqa X14, (STACK_TMP1)(%rsp);
	PBROADCASTD((15 * 4)(INPUT), X13);
	PLUS(X15, X13);
	movdqa X15, (STACK_TMP2)(%rsp);

	/* Update counter */
	addq $4, (12 * 4)(INPUT);

	TRANSPOSE_4x4(X0, X1, X2, X3, X13, X14, X15);
	movdqu X0, (64 * 0 + 16 * 0)(DST)
	movdqu X1, (64 * 1 + 16 * 0)(DST)
	movdqu X2, (64 * 2 + 16 * 0)(DST)
	movdqu X3, (64 * 3 + 16 * 0)(DST)
	TRANSPOSE_4x4(X4, X5, X6, X7, X0, X1, X2);
	movdqa (STACK_TMP)(%rsp), X13;
	movdqa (STACK_TMP1)(%rsp), X14;
	movdqa (STACK_TMP2)(%rsp), X15;
	movdqu X4, (64 * 0 + 16 * 1)(DST)
	movdqu X5, (64 * 1 + 16 * 1)(DST)
	movdqu X6, (64 * 2 + 16 * 1)(DST)
	movdqu X7, (64 * 3 + 16 * 1)(DST)
	TRANSPOSE_4x4(X8, X9, X10, X11, X0, X1, X2);
	movdqu X8,  (64 * 0 + 16 * 2)(DST)
	movdqu X9,  (64 * 1 + 16 * 2)(DST)
	movdqu X10, (64 * 2 + 16 * 2)(DST)
	movdqu X11, (64 * 3 + 16 * 2)(DST)
	TRANSPOSE_4x4(X12, X13, X14, X15, X0, X1, X2);
	movdqu X12, (64 * 0 + 16 * 3)(DST)
	movdqu X13, (64 * 1 + 16 * 3)(DST)
	movdqu X14, (64 * 2 + 16 * 3)(DST)
	movdqu X15, (64 * 3 + 16 * 3)(DST)

	sub $4, NBLKS;
	lea (4 * 64)(DST), DST;
	lea (4 * 64)(SRC), SRC;
	jnz L(loop4);

	/* eax zeroed by round loop. */
	leave;
	cfi_adjust_cfa_offset(-8)
	cfi_def_cfa_register(%rsp);
	ret_spec_stop;
END (__chacha20_sse2_blocks4)

#endif /* if MINIMUM_X86_ISA_LEVEL <= 2 */
