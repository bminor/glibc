/* Copyright (C) 2012-2014 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Assumptions:
 *
 * ARMv8-a, AArch64
 * Unaligned accesses
 */

#include <sysdep.h>

#define dstin		x0
#define dstin_w		w0
#define val		x1
#define valw		w1
#define count		x2
#define tmp1		x3
#define tmp1w		w3
#define tmp2		x4
#define tmp2w		w4
#define zva_len_x	x5
#define zva_len		w5
#define zva_mask_x	x6
#define zva_mask	w6
#define dst		x8
#define dst_w		w8
#define dstend		x9

	.globl	memset
	cfi_startproc

#if HAVE_IFUNC && !defined (IS_IN_rtld)
/* Rather than decode dczid_el0 every time, checking for zva disabled and
   unpacking the line size, do this once in the indirect function and choose
   an appropriate entry point which encodes these values as constants.  */

	.type	memset, %gnu_indirect_function
memset:
	mrs	x1, dczid_el0
	adrp	x0, 1f
	tst	x1, #16				/* test for zva disabled */
	and	x1, x1, #15
	add	x0, x0, #:lo12:1f
	csel	x1, xzr, x1, ne			/* squash index to 0 if so */
	ldrsw	x2, [x0, x1, lsl #2]
	add	x0, x0, x2
	RET
	.size	memset, .-memset

	.section .rodata
1:	.long	memset_nozva - 1b		// 0
	.long	memset_nozva - 1b		// 1
	.long	memset_nozva - 1b		// 2
	.long	memset_nozva - 1b		// 3
	.long	memset_zva_64 - 1b		// 4
	.long	memset_zva_128 - 1b		// 5
	.long	memset_zva_256 - 1b		// 6
	.long	memset_zva_512 - 1b		// 7
	.long	memset_zva_1024 - 1b	// 8
	.long	memset_zva_2048 - 1b	// 9
	.long	memset_zva_4096 - 1b	// 10
	.long	memset_zva_8192 - 1b	// 11
	.long	memset_zva_16384 - 1b	// 12
	.long	memset_zva_32768 - 1b	// 13
	.long	memset_zva_65536 - 1b	// 14
	.long	memset_zva_131072 - 1b	// 15
	.previous

/* The 64 byte zva size is too small, and needs unrolling for efficiency.  */

	.p2align 6
	.type	memset_zva_64, %function
memset_zva_64:
	CALL_MCOUNT
	and	valw, valw, #255
	cmp	count, #256
	ccmp	valw, #0, #0, hs	/* hs ? cmp val,0 : !z */
	b.ne	L(nz_or_small)

	stp	xzr, xzr, [dstin]	/* first 16 aligned 1.  */
	and	tmp2, dstin, #-16
	and	dst, dstin, #-64

	stp	xzr, xzr, [tmp2, #16]	/* first 64 aligned 16.  */
	add	dstend, dstin, count
	add	dst, dst, #64

	stp	xzr, xzr, [tmp2, #32]
	sub	count, dstend, dst	/* recompute for misalign */
	add	tmp1, dst, #64

	stp	xzr, xzr, [tmp2, #48]
	sub	count, count, #128	/* pre-bias */

	stp	xzr, xzr, [tmp2, #64]

	.p2align 6,,24
0:	dc	zva, dst
	subs	count, count, #128
	dc	zva, tmp1
	add	dst, dst, #128
	add	tmp1, tmp1, #128
	b.hs	0b

	adds	count, count, #128	/* undo pre-bias */
	b.ne	L(zva_tail)
	RET

	.size	memset_zva_64, . - memset_zva_64

/* For larger zva sizes, a simple loop ought to suffice.  */
/* ??? Needs performance testing, when such hardware becomes available.  */

.macro do_zva len
	.p2align 4
	.type	memset_zva_\len, %function
memset_zva_\len:
	CALL_MCOUNT
	and	valw, valw, #255
	cmp	count, #\len
	ccmp	valw, #0, #0, hs	/* hs ? cmp val,0 : !z */
	b.ne	L(nz_or_small)

	add	dstend, dstin, count
	mov	zva_len, #\len
	mov	zva_mask, #\len-1
	b	memset_zva_n

	.size	memset_zva_\len, . - memset_zva_\len
.endm

	do_zva 128	// 5
	do_zva 256	// 6
	do_zva 512	// 7
	do_zva 1024	// 8
	do_zva 2048	// 9
	do_zva 4096	// 10
	do_zva 8192	// 11
	do_zva 16384	// 12
	do_zva 32768	// 13
	do_zva 65536	// 14
	do_zva 131072	// 15

	.p2align 6
#else
/* Without IFUNC, we must load the zva data from the dczid register.  */

	.p2align 6
	.type	memset, %function
memset:
	and	valw, valw, #255
	cmp	count, #256
	ccmp	valw, #0, #0, hs	/* hs ? cmp val,0 : !z */
	b.ne	L(nz_or_small)

	mrs	tmp1, dczid_el0
	tbnz	tmp1, #4, L(nz_or_small)

	and	tmp1w, tmp1w, #15
	mov	zva_len, #4
	add	dstend, dstin, count
	lsl	zva_len, zva_len, tmp1w
	cmp	count, zva_len_x
	sub	zva_mask, zva_len, #1
	b.lo	L(ge_64)

	/* Fall through into memset_zva_n.  */
	.size	memset, . - memset
#endif /* HAVE_IFUNC */

/* Main part of the zva path.  On arrival here, we've already checked for
   minimum size and that VAL is zero.  Also, we've set up zva_len and mask. */

	.type	memset_zva_n, %function
memset_zva_n:
	stp	xzr, xzr, [dstin]	/* first 16 aligned 1.  */
	neg	tmp1w, dstin_w
	sub	count, count, zva_len_x	/* pre-bias */
	mov	dst, dstin
	ands	tmp1w, tmp1w, zva_mask
	b.ne	3f

	.p2align 6,,16
2:	dc	zva, dst
	subs	count, count, zva_len_x
	add	dst, dst, zva_len_x
	b.hs	2b

	adds	count, count, zva_len_x	/* undo pre-bias */
	b.ne	L(zva_tail)
	RET

	.p2align 4
3:	and	tmp2, dstin, #-16
	sub	count, count, tmp1	/* account for misalign */
	add	dst, dstin, tmp1

	.p2align 6,,24
4:	stp	xzr, xzr, [tmp2, #16]
	stp	xzr, xzr, [tmp2, #32]
	subs	tmp1w, tmp1w, #64
	stp	xzr, xzr, [tmp2, #48]
	stp	xzr, xzr, [tmp2, #64]!
	b.hi	4b

	b	2b

	.size	memset_zva_n, . - memset_zva_n

/* The non-zva path.  */

	.p2align 6
	.type	memset_nozva, %function
memset_nozva:
	CALL_MCOUNT
	and	valw, valw, #255
L(nz_or_small):
	orr	valw, valw, valw, lsl #8  /* replicate the byte */
	cmp	count, #64
	orr	valw, valw, valw, lsl #16
	add	dstend, dstin, count	  /* remember end of buffer */
	orr	val, val, val, lsl #32
	b.hs	L(ge_64)

	/* Small data -- original count is less than 64 bytes.  */
L(le_63):
	cmp	count, #16
	b.lo	L(le_15)
	stp	val, val, [dstin]
	tbz	count, #5, L(le_31)
	stp	val, val, [dstin, #16]
	stp	val, val, [dstend, #-32]
L(le_31):
	stp	val, val, [dstend, #-16]
	RET
	.p2align 6,,16
L(le_15):
	tbz	count, #3, L(le_7)
	str	val, [dstin]
	str	val, [dstend, #-8]
	RET
	.p2align 6,,16
L(le_7):
	tbz	count, #2, L(le_3)
	str	valw, [dstin]
	str	valw, [dstend, #-4]
	RET
	.p2align 6,,20
L(le_3):
	tbz	count, #1, L(le_1)
	strh	valw, [dstend, #-2]
L(le_1):
	tbz	count, #0, L(le_0)
	strb	valw, [dstin]
L(le_0):
	RET

	.p2align 6
L(ge_64):
	and	dst, dstin, #-16	/* align the pointer / pre-bias.  */
	stp	val, val, [dstin]	/* first 16 align 1 */
	sub	count, dstend, dst	/* begin misalign recompute */
	subs	count, count, #16+64	/* finish recompute + pre-bias */
	b.ls	L(loop_tail)

	.p2align 6,,24
L(loop):
	stp	val, val, [dst, #16]
	stp	val, val, [dst, #32]
	subs	count, count, #64
	stp	val, val, [dst, #48]
	stp	val, val, [dst, #64]!
	b.hs	L(loop)

	adds	count, count, #64	/* undo pre-bias */
	b.ne	L(loop_tail)
	RET

	/* Tail of the zva loop.  Less than ZVA bytes, but possibly lots
	   more than 64.  Note that dst is aligned but unbiased.  */
L(zva_tail):
	subs	count, count, #64	/* pre-bias */
	sub	dst, dst, #16		/* pre-bias */
	b.hi	L(loop)

	/* Tail of the stp loop; less than 64 bytes left.
	   Note that dst is still aligned and biased by -16.  */
L(loop_tail):
	stp	val, val, [dstend, #-64]
	stp	val, val, [dstend, #-48]
	stp	val, val, [dstend, #-32]
	stp	val, val, [dstend, #-16]
	RET

	.size	memset_nozva, . - memset_nozva
	cfi_endproc

strong_alias (memset, __memset)
libc_hidden_builtin_def (memset)
