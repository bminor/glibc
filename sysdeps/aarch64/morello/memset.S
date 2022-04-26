/* Copyright (C) 2022 Free Software Foundation, Inc.

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

#ifndef MEMSET
# define MEMSET memset
#endif

/* Assumptions:
 *
 * ARMv8-a, AArch64, Morello, Advanced SIMD, unaligned accesses.
 *
 */

#if defined(__CHERI_PURE_CAPABILITY__)
#define dstin	c0
#define val	x1
#define valw	w1
#define count	x2
#define dst	c3
#define xdst	x3
#define dstend	c4
#define xdstend	x4
#define zva_val	x5
#else
#define dstin	x0
#define val	x1
#define valw	w1
#define count	x2
#define dst	x3
#define xdst	x3
#define dstend	x4
#define xdstend	x4
#define zva_val	x5
#endif

ENTRY (MEMSET)
	PTR_ARG (0)
	SIZE_ARG (2)

	dup	v0.16B, valw
	add	dstend, dstin, count

	cmp	count, 96
	b.hi	L(set_long)
	cmp	count, 16
	b.hs	L(set_medium)
	mov	val, v0.D[0]

	/* Set 0..15 bytes.  */
	tbz	count, 3, 1f
	str	val, [dstin]
	str	val, [dstend, -8]
	ret
	.p2align 4
1:	tbz	count, 2, 2f
	str	valw, [dstin]
	str	valw, [dstend, -4]
	ret
2:	cbz	count, 3f
	strb	valw, [dstin]
	tbz	count, 1, 3f
	strh	valw, [dstend, -2]
3:	ret

	/* Set 17..96 bytes.  */
L(set_medium):
	str	q0, [dstin]
	tbnz	count, 6, L(set96)
	str	q0, [dstend, -16]
	tbz	count, 5, 1f
	str	q0, [dstin, 16]
	str	q0, [dstend, -32]
1:	ret

	.p2align 4
	/* Set 64..96 bytes.  Write 64 bytes from the start and
	   32 bytes from the end.  */
L(set96):
	str	q0, [dstin, 16]
	stp	q0, q0, [dstin, 32]
	stp	q0, q0, [dstend, -32]
	ret

	.p2align 4
L(set_long):
	and	valw, valw, 255
#if defined(__CHERI_PURE_CAPABILITY__)
	alignd	dst, dstin, 4
#else
	bic	dst, dstin, 15
#endif
	str	q0, [dstin]
	cmp	count, 160
	ccmp	valw, 0, 0, hs
	b.ne	L(no_zva)

#ifndef SKIP_ZVA_CHECK
	mrs	zva_val, dczid_el0
	and	zva_val, zva_val, 31
	cmp	zva_val, 4		/* ZVA size is 64 bytes.  */
	b.ne	L(no_zva)
#endif
	str	q0, [dst, 16]
	stp	q0, q0, [dst, 32]
#if defined(__CHERI_PURE_CAPABILITY__)
	alignd	dst, dst, 6
#else
	bic	dst, dst, 63
#endif
	sub	count, xdstend, xdst	/* Count is now 64 too large.  */
	sub	count, count, 128	/* Adjust count and bias for loop.  */

	.p2align 4
L(zva_loop):
	add	dst, dst, 64
	dc	zva, dst
	subs	count, count, 64
	b.hi	L(zva_loop)
	stp	q0, q0, [dstend, -64]
	stp	q0, q0, [dstend, -32]
	ret

L(no_zva):
	sub	count, xdstend, xdst	/* Count is 16 too large.  */
	sub	dst, dst, 16		/* Dst is biased by -32.  */
	sub	count, count, 64 + 16	/* Adjust count and bias for loop.  */
L(no_zva_loop):
	stp	q0, q0, [dst, 32]
	stp	q0, q0, [dst, 64]!
	subs	count, count, 64
	b.hi	L(no_zva_loop)
	stp	q0, q0, [dstend, -64]
	stp	q0, q0, [dstend, -32]
	ret

END (MEMSET)
libc_hidden_builtin_def (MEMSET)
