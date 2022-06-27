/* strnlen - calculate the length of a string with limit.

   Copyright (C) 2013-2022 Free Software Foundation, Inc.

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
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* Assumptions:
 *
 * ARMv8-a, AArch64, Advanced SIMD.
 * MTE compatible.
 */

#define srcin		x0
#define cntin		x1
#define result		x0

#define src		x2
#define synd		x3
#define	shift		x4
#define tmp		x4
#define cntrem		x5

#define qdata		q0
#define vdata		v0
#define vhas_chr	v1
#define vend		v2
#define dend		d2

/*
   Core algorithm:

   For each 16-byte chunk we calculate a 64-bit nibble mask value with four bits
   per byte. We take 4 bits of every comparison byte with shift right and narrow
   by 4 instruction. Since the bits in the nibble mask reflect the order in
   which things occur in the original string, counting trailing zeros identifies
   exactly which byte matched.  */

ENTRY (__strnlen)
	PTR_ARG (0)
	SIZE_ARG (1)
	bic	src, srcin, 15
	cbz	cntin, L(nomatch)
	ld1	{vdata.16b}, [src], 16
	cmeq	vhas_chr.16b, vdata.16b, 0
	lsl	shift, srcin, 2
	shrn	vend.8b, vhas_chr.8h, 4		/* 128->64 */
	fmov	synd, dend
	lsr	synd, synd, shift
	cbz	synd, L(start_loop)
L(finish):
	rbit	synd, synd
	clz	synd, synd
	lsr	result, synd, 2
	cmp	cntin, result
	csel	result, cntin, result, ls
	ret

L(start_loop):
	sub	tmp, src, srcin
	subs	cntrem, cntin, tmp
	b.ls	L(nomatch)

	/* Make sure that it won't overread by a 16-byte chunk */
	add	tmp, cntrem, 15
	tbnz	tmp, 4, L(loop32_2)

	.p2align 5
L(loop32):
	ldr	qdata, [src], 16
	cmeq	vhas_chr.16b, vdata.16b, 0
	umaxp	vend.16b, vhas_chr.16b, vhas_chr.16b		/* 128->64 */
	fmov	synd, dend
	cbnz	synd, L(end)
L(loop32_2):
	ldr	qdata, [src], 16
	subs	cntrem, cntrem, 32
	cmeq	vhas_chr.16b, vdata.16b, 0
	b.ls	L(end)
	umaxp	vend.16b, vhas_chr.16b, vhas_chr.16b		/* 128->64 */
	fmov	synd, dend
	cbz	synd, L(loop32)

L(end):
	shrn	vend.8b, vhas_chr.8h, 4		/* 128->64 */
	sub	src, src, 16
	mov	synd, vend.d[0]
	sub	result, src, srcin
#ifndef __AARCH64EB__
	rbit	synd, synd
#endif
	clz	synd, synd
	add	result, result, synd, lsr 2
	cmp	cntin, result
	csel	result, cntin, result, ls
	ret

L(nomatch):
	mov	result, cntin
	ret

END (__strnlen)
libc_hidden_def (__strnlen)
weak_alias (__strnlen, strnlen)
libc_hidden_def (strnlen)
