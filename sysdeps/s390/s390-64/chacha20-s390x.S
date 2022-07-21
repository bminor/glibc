/* Optimized s390x implementation of ChaCha20 cipher.
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

/* chacha20-s390x.S  -  zSeries implementation of ChaCha20 cipher

   Copyright (C) 2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>

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

#include <sysdep.h>

#ifdef HAVE_S390_VX_ASM_SUPPORT

/* CFA expressions are used for pointing CFA and registers to
 * SP relative offsets. */
# define DW_REGNO_SP 15

/* Fixed length encoding used for integers for now. */
# define DW_SLEB128_7BIT(value) \
        0x00|((value) & 0x7f)
# define DW_SLEB128_28BIT(value) \
        0x80|((value)&0x7f), \
        0x80|(((value)>>7)&0x7f), \
        0x80|(((value)>>14)&0x7f), \
        0x00|(((value)>>21)&0x7f)

# define cfi_cfa_on_stack(rsp_offs,cfa_depth) \
        .cfi_escape \
          0x0f, /* DW_CFA_def_cfa_expression */ \
            DW_SLEB128_7BIT(11), /* length */ \
          0x7f, /* DW_OP_breg15, rsp + constant */ \
            DW_SLEB128_28BIT(rsp_offs), \
          0x06, /* DW_OP_deref */ \
          0x23, /* DW_OP_plus_constu */ \
            DW_SLEB128_28BIT((cfa_depth)+160)

.machine "z13+vx"
.text

.balign 16
.Lconsts:
.Lwordswap:
	.byte 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3
.Lbswap128:
	.byte 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
.Lbswap32:
	.byte 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12
.Lone:
	.long 0, 0, 0, 1
.Ladd_counter_0123:
	.long 0, 1, 2, 3
.Ladd_counter_4567:
	.long 4, 5, 6, 7

/* register macros */
#define INPUT %r2
#define DST   %r3
#define SRC   %r4
#define NBLKS %r0
#define ROUND %r1

/* stack structure */

#define STACK_FRAME_STD    (8 * 16 + 8 * 4)
#define STACK_FRAME_F8_F15 (8 * 8)
#define STACK_FRAME_Y0_Y15 (16 * 16)
#define STACK_FRAME_CTR    (4 * 16)
#define STACK_FRAME_PARAMS (6 * 8)

#define STACK_MAX   (STACK_FRAME_STD + STACK_FRAME_F8_F15 + \
		     STACK_FRAME_Y0_Y15 + STACK_FRAME_CTR + \
		     STACK_FRAME_PARAMS)

#define STACK_F8     (STACK_MAX - STACK_FRAME_F8_F15)
#define STACK_F9     (STACK_F8 + 8)
#define STACK_F10    (STACK_F9 + 8)
#define STACK_F11    (STACK_F10 + 8)
#define STACK_F12    (STACK_F11 + 8)
#define STACK_F13    (STACK_F12 + 8)
#define STACK_F14    (STACK_F13 + 8)
#define STACK_F15    (STACK_F14 + 8)
#define STACK_Y0_Y15 (STACK_F8 - STACK_FRAME_Y0_Y15)
#define STACK_CTR    (STACK_Y0_Y15 - STACK_FRAME_CTR)
#define STACK_INPUT  (STACK_CTR - STACK_FRAME_PARAMS)
#define STACK_DST    (STACK_INPUT + 8)
#define STACK_SRC    (STACK_DST + 8)
#define STACK_NBLKS  (STACK_SRC + 8)
#define STACK_POCTX  (STACK_NBLKS + 8)
#define STACK_POSRC  (STACK_POCTX + 8)

#define STACK_G0_H3  STACK_Y0_Y15

/* vector registers */
#define A0 %v0
#define A1 %v1
#define A2 %v2
#define A3 %v3

#define B0 %v4
#define B1 %v5
#define B2 %v6
#define B3 %v7

#define C0 %v8
#define C1 %v9
#define C2 %v10
#define C3 %v11

#define D0 %v12
#define D1 %v13
#define D2 %v14
#define D3 %v15

#define E0 %v16
#define E1 %v17
#define E2 %v18
#define E3 %v19

#define F0 %v20
#define F1 %v21
#define F2 %v22
#define F3 %v23

#define G0 %v24
#define G1 %v25
#define G2 %v26
#define G3 %v27

#define H0 %v28
#define H1 %v29
#define H2 %v30
#define H3 %v31

#define IO0 E0
#define IO1 E1
#define IO2 E2
#define IO3 E3
#define IO4 F0
#define IO5 F1
#define IO6 F2
#define IO7 F3

#define S0 G0
#define S1 G1
#define S2 G2
#define S3 G3

#define TMP0 H0
#define TMP1 H1
#define TMP2 H2
#define TMP3 H3

#define X0 A0
#define X1 A1
#define X2 A2
#define X3 A3
#define X4 B0
#define X5 B1
#define X6 B2
#define X7 B3
#define X8 C0
#define X9 C1
#define X10 C2
#define X11 C3
#define X12 D0
#define X13 D1
#define X14 D2
#define X15 D3

#define Y0 E0
#define Y1 E1
#define Y2 E2
#define Y3 E3
#define Y4 F0
#define Y5 F1
#define Y6 F2
#define Y7 F3
#define Y8 G0
#define Y9 G1
#define Y10 G2
#define Y11 G3
#define Y12 H0
#define Y13 H1
#define Y14 H2
#define Y15 H3

/**********************************************************************
  helper macros
 **********************************************************************/

#define _ /*_*/

#define START_STACK(last_r) \
	lgr %r0, %r15; \
	lghi %r1, ~15; \
	stmg %r6, last_r, 6 * 8(%r15); \
	aghi %r0, -STACK_MAX; \
	ngr %r0, %r1; \
	lgr %r1, %r15; \
	cfi_def_cfa_register(1); \
	lgr %r15, %r0; \
	stg %r1, 0(%r15); \
	cfi_cfa_on_stack(0, 0); \
	std %f8, STACK_F8(%r15); \
	std %f9, STACK_F9(%r15); \
	std %f10, STACK_F10(%r15); \
	std %f11, STACK_F11(%r15); \
	std %f12, STACK_F12(%r15); \
	std %f13, STACK_F13(%r15); \
	std %f14, STACK_F14(%r15); \
	std %f15, STACK_F15(%r15);

#define END_STACK(last_r) \
	lg %r1, 0(%r15); \
	ld %f8, STACK_F8(%r15); \
	ld %f9, STACK_F9(%r15); \
	ld %f10, STACK_F10(%r15); \
	ld %f11, STACK_F11(%r15); \
	ld %f12, STACK_F12(%r15); \
	ld %f13, STACK_F13(%r15); \
	ld %f14, STACK_F14(%r15); \
	ld %f15, STACK_F15(%r15); \
	lmg %r6, last_r, 6 * 8(%r1); \
	lgr %r15, %r1; \
	cfi_def_cfa_register(DW_REGNO_SP);

#define PLUS(dst,src) \
	vaf dst, dst, src;

#define XOR(dst,src) \
	vx dst, dst, src;

#define ROTATE(v1,c) \
	verllf v1, v1, (c)(0);

#define WORD_ROTATE(v1,s) \
	vsldb v1, v1, v1, ((s) * 4);

#define DST_8(OPER, I, J) \
	OPER(A##I, J); OPER(B##I, J); OPER(C##I, J); OPER(D##I, J); \
	OPER(E##I, J); OPER(F##I, J); OPER(G##I, J); OPER(H##I, J);

/**********************************************************************
  round macros
 **********************************************************************/

/**********************************************************************
  8-way chacha20 ("vertical")
 **********************************************************************/

#define QUARTERROUND4_V8_POLY(x0,x1,x2,x3,x4,x5,x6,x7,\
			      x8,x9,x10,x11,x12,x13,x14,x15,\
			      y0,y1,y2,y3,y4,y5,y6,y7,\
			      y8,y9,y10,y11,y12,y13,y14,y15,\
			      op1,op2,op3,op4,op5,op6,op7,op8,\
			      op9,op10,op11,op12) \
	op1;							\
	PLUS(x0, x1); PLUS(x4, x5);				\
	PLUS(x8, x9); PLUS(x12, x13);				\
	PLUS(y0, y1); PLUS(y4, y5);				\
	PLUS(y8, y9); PLUS(y12, y13);				\
	    op2;						\
	    XOR(x3, x0);  XOR(x7, x4);				\
	    XOR(x11, x8); XOR(x15, x12);			\
	    XOR(y3, y0);  XOR(y7, y4);				\
	    XOR(y11, y8); XOR(y15, y12);			\
		op3;						\
		ROTATE(x3, 16); ROTATE(x7, 16);			\
		ROTATE(x11, 16); ROTATE(x15, 16);		\
		ROTATE(y3, 16); ROTATE(y7, 16);			\
		ROTATE(y11, 16); ROTATE(y15, 16);		\
	op4;							\
	PLUS(x2, x3); PLUS(x6, x7);				\
	PLUS(x10, x11); PLUS(x14, x15);				\
	PLUS(y2, y3); PLUS(y6, y7);				\
	PLUS(y10, y11); PLUS(y14, y15);				\
	    op5;						\
	    XOR(x1, x2); XOR(x5, x6);				\
	    XOR(x9, x10); XOR(x13, x14);			\
	    XOR(y1, y2); XOR(y5, y6);				\
	    XOR(y9, y10); XOR(y13, y14);			\
		op6;						\
		ROTATE(x1,12); ROTATE(x5,12);			\
		ROTATE(x9,12); ROTATE(x13,12);			\
		ROTATE(y1,12); ROTATE(y5,12);			\
		ROTATE(y9,12); ROTATE(y13,12);			\
	op7;							\
	PLUS(x0, x1); PLUS(x4, x5);				\
	PLUS(x8, x9); PLUS(x12, x13);				\
	PLUS(y0, y1); PLUS(y4, y5);				\
	PLUS(y8, y9); PLUS(y12, y13);				\
	    op8;						\
	    XOR(x3, x0); XOR(x7, x4);				\
	    XOR(x11, x8); XOR(x15, x12);			\
	    XOR(y3, y0); XOR(y7, y4);				\
	    XOR(y11, y8); XOR(y15, y12);			\
		op9;						\
		ROTATE(x3,8); ROTATE(x7,8);			\
		ROTATE(x11,8); ROTATE(x15,8);			\
		ROTATE(y3,8); ROTATE(y7,8);			\
		ROTATE(y11,8); ROTATE(y15,8);			\
	op10;							\
	PLUS(x2, x3); PLUS(x6, x7);				\
	PLUS(x10, x11); PLUS(x14, x15);				\
	PLUS(y2, y3); PLUS(y6, y7);				\
	PLUS(y10, y11); PLUS(y14, y15);				\
	    op11;						\
	    XOR(x1, x2); XOR(x5, x6);				\
	    XOR(x9, x10); XOR(x13, x14);			\
	    XOR(y1, y2); XOR(y5, y6);				\
	    XOR(y9, y10); XOR(y13, y14);			\
		op12;						\
		ROTATE(x1,7); ROTATE(x5,7);			\
		ROTATE(x9,7); ROTATE(x13,7);			\
		ROTATE(y1,7); ROTATE(y5,7);			\
		ROTATE(y9,7); ROTATE(y13,7);

#define QUARTERROUND4_V8(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,\
			 y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,y10,y11,y12,y13,y14,y15) \
	QUARTERROUND4_V8_POLY(x0,x1,x2,x3,x4,x5,x6,x7,\
			      x8,x9,x10,x11,x12,x13,x14,x15,\
			      y0,y1,y2,y3,y4,y5,y6,y7,\
			      y8,y9,y10,y11,y12,y13,y14,y15,\
			      ,,,,,,,,,,,)

#define TRANSPOSE_4X4_2(v0,v1,v2,v3,va,vb,vc,vd,tmp0,tmp1,tmp2,tmpa,tmpb,tmpc) \
	  vmrhf tmp0, v0, v1;					\
	  vmrhf tmp1, v2, v3;					\
	  vmrlf tmp2, v0, v1;					\
	  vmrlf   v3, v2, v3;					\
	  vmrhf tmpa, va, vb;					\
	  vmrhf tmpb, vc, vd;					\
	  vmrlf tmpc, va, vb;					\
	  vmrlf   vd, vc, vd;					\
	  vpdi v0, tmp0, tmp1, 0;				\
	  vpdi v1, tmp0, tmp1, 5;				\
	  vpdi v2, tmp2,   v3, 0;				\
	  vpdi v3, tmp2,   v3, 5;				\
	  vpdi va, tmpa, tmpb, 0;				\
	  vpdi vb, tmpa, tmpb, 5;				\
	  vpdi vc, tmpc,   vd, 0;				\
	  vpdi vd, tmpc,   vd, 5;

.balign 8
.globl __chacha20_s390x_vx_blocks8
ENTRY (__chacha20_s390x_vx_blocks8)
	/* input:
	 *	%r2: input
	 *	%r3: dst
	 *	%r4: src
	 *	%r5: nblks (multiple of 8)
	 */

	START_STACK(%r8);
	lgr NBLKS, %r5;

	larl %r7, .Lconsts;

	/* Load counter. */
	lg %r8, (12 * 4)(INPUT);
	rllg %r8, %r8, 32;

.balign 4
	/* Process eight chacha20 blocks per loop. */
.Lloop8:
	vlm Y0, Y3, 0(INPUT);

	slgfi NBLKS, 8;
	lghi ROUND, (20 / 2);

	/* Construct counter vectors X12/X13 & Y12/Y13. */
	vl X4, (.Ladd_counter_0123 - .Lconsts)(%r7);
	vl Y4, (.Ladd_counter_4567 - .Lconsts)(%r7);
	vrepf Y12, Y3, 0;
	vrepf Y13, Y3, 1;
	vaccf X5, Y12, X4;
	vaccf Y5, Y12, Y4;
	vaf X12, Y12, X4;
	vaf Y12, Y12, Y4;
	vaf X13, Y13, X5;
	vaf Y13, Y13, Y5;

	vrepf X0, Y0, 0;
	vrepf X1, Y0, 1;
	vrepf X2, Y0, 2;
	vrepf X3, Y0, 3;
	vrepf X4, Y1, 0;
	vrepf X5, Y1, 1;
	vrepf X6, Y1, 2;
	vrepf X7, Y1, 3;
	vrepf X8, Y2, 0;
	vrepf X9, Y2, 1;
	vrepf X10, Y2, 2;
	vrepf X11, Y2, 3;
	vrepf X14, Y3, 2;
	vrepf X15, Y3, 3;

	/* Store counters for blocks 0-7. */
	vstm X12, X13, (STACK_CTR + 0 * 16)(%r15);
	vstm Y12, Y13, (STACK_CTR + 2 * 16)(%r15);

	vlr Y0, X0;
	vlr Y1, X1;
	vlr Y2, X2;
	vlr Y3, X3;
	vlr Y4, X4;
	vlr Y5, X5;
	vlr Y6, X6;
	vlr Y7, X7;
	vlr Y8, X8;
	vlr Y9, X9;
	vlr Y10, X10;
	vlr Y11, X11;
	vlr Y14, X14;
	vlr Y15, X15;

	/* Update and store counter. */
	agfi %r8, 8;
	rllg %r5, %r8, 32;
	stg %r5, (12 * 4)(INPUT);

.balign 4
.Lround2_8:
	QUARTERROUND4_V8(X0, X4,  X8, X12,   X1, X5,  X9, X13,
			 X2, X6, X10, X14,   X3, X7, X11, X15,
			 Y0, Y4,  Y8, Y12,   Y1, Y5,  Y9, Y13,
			 Y2, Y6, Y10, Y14,   Y3, Y7, Y11, Y15);
	QUARTERROUND4_V8(X0, X5, X10, X15,   X1, X6, X11, X12,
			 X2, X7,  X8, X13,   X3, X4,  X9, X14,
			 Y0, Y5, Y10, Y15,   Y1, Y6, Y11, Y12,
			 Y2, Y7,  Y8, Y13,   Y3, Y4,  Y9, Y14);
	brctg ROUND, .Lround2_8;

	/* Store blocks 4-7. */
	vstm Y0, Y15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 0-3. */
	vlm Y0, Y1, (STACK_CTR + 0 * 16)(%r15);

	lghi ROUND, 1;
	j .Lfirst_output_4blks_8;

.balign 4
.Lsecond_output_4blks_8:
	/* Load blocks 4-7. */
	vlm X0, X15, STACK_Y0_Y15(%r15);

	/* Load counters for blocks 4-7. */
	vlm Y0, Y1, (STACK_CTR + 2 * 16)(%r15);

	lghi ROUND, 0;

.balign 4
	/* Output four chacha20 blocks per loop. */
.Lfirst_output_4blks_8:
	vlm Y12, Y15, 0(INPUT);
	PLUS(X12, Y0);
	PLUS(X13, Y1);
	vrepf Y0, Y12, 0;
	vrepf Y1, Y12, 1;
	vrepf Y2, Y12, 2;
	vrepf Y3, Y12, 3;
	vrepf Y4, Y13, 0;
	vrepf Y5, Y13, 1;
	vrepf Y6, Y13, 2;
	vrepf Y7, Y13, 3;
	vrepf Y8, Y14, 0;
	vrepf Y9, Y14, 1;
	vrepf Y10, Y14, 2;
	vrepf Y11, Y14, 3;
	vrepf Y14, Y15, 2;
	vrepf Y15, Y15, 3;
	PLUS(X0, Y0);
	PLUS(X1, Y1);
	PLUS(X2, Y2);
	PLUS(X3, Y3);
	PLUS(X4, Y4);
	PLUS(X5, Y5);
	PLUS(X6, Y6);
	PLUS(X7, Y7);
	PLUS(X8, Y8);
	PLUS(X9, Y9);
	PLUS(X10, Y10);
	PLUS(X11, Y11);
	PLUS(X14, Y14);
	PLUS(X15, Y15);

	vl Y15, (.Lbswap32 - .Lconsts)(%r7);
	TRANSPOSE_4X4_2(X0, X1, X2, X3, X4, X5, X6, X7,
			Y9, Y10, Y11, Y12, Y13, Y14);
	TRANSPOSE_4X4_2(X8, X9, X10, X11, X12, X13, X14, X15,
			Y9, Y10, Y11, Y12, Y13, Y14);

	vlm Y0, Y14, 0(SRC);
	vperm X0, X0, X0, Y15;
	vperm X1, X1, X1, Y15;
	vperm X2, X2, X2, Y15;
	vperm X3, X3, X3, Y15;
	vperm X4, X4, X4, Y15;
	vperm X5, X5, X5, Y15;
	vperm X6, X6, X6, Y15;
	vperm X7, X7, X7, Y15;
	vperm X8, X8, X8, Y15;
	vperm X9, X9, X9, Y15;
	vperm X10, X10, X10, Y15;
	vperm X11, X11, X11, Y15;
	vperm X12, X12, X12, Y15;
	vperm X13, X13, X13, Y15;
	vperm X14, X14, X14, Y15;
	vperm X15, X15, X15, Y15;
	vl Y15, (15 * 16)(SRC);

	XOR(Y0, X0);
	XOR(Y1, X4);
	XOR(Y2, X8);
	XOR(Y3, X12);
	XOR(Y4, X1);
	XOR(Y5, X5);
	XOR(Y6, X9);
	XOR(Y7, X13);
	XOR(Y8, X2);
	XOR(Y9, X6);
	XOR(Y10, X10);
	XOR(Y11, X14);
	XOR(Y12, X3);
	XOR(Y13, X7);
	XOR(Y14, X11);
	XOR(Y15, X15);
	vstm Y0, Y15, 0(DST);

	aghi SRC, 256;
	aghi DST, 256;

	clgije ROUND, 1, .Lsecond_output_4blks_8;

	clgijhe NBLKS, 8, .Lloop8;


	END_STACK(%r8);
	xgr %r2, %r2;
	br %r14;
END (__chacha20_s390x_vx_blocks8)

#endif /* HAVE_S390_VX_ASM_SUPPORT */
