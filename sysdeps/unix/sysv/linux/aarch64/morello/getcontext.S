/* Save current context.

   Copyright (C) 2009-2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include "ucontext_i.h"
#include "ucontext-internal.h"

/* int getcontext (ucontext_t *ucp)

   Returns 0 on success -1 and errno on failure.
 */

	.text

ENTRY(__getcontext)
	/* For Morello, we will save only the capability registers. These are saved
	   in a special area in the context extension block after the FPSIMD
	   context.  */
	add	c9, c0, #oEXTENSION + FPSIMD_CONTEXT_SIZE

	/* Write the context extension morello header.  */
	mov	w3, #(MORELLO_MAGIC & 0xffff)
	movk	w3, #(MORELLO_MAGIC >> 16), lsl #16
	str	w3, [c9, #oCHEAD + oMAGIC]
	mov	w3, #MORELLO_CONTEXT_SIZE
	str	w3, [c9, #oCHEAD + oSIZE]

	add	c10, c9, #oC0
	/* The saved context will return to the getcontext() call point
	   with a return value of 0 */
	str	czr,	  [c10, 0 * SZCREG]

	stp	c18, c19, [c10, 18 * SZCREG]
	stp	c20, c21, [c10, 20 * SZCREG]
	stp	c22, c23, [c10, 22 * SZCREG]
	stp	c24, c25, [c10, 24 * SZCREG]
	stp	c26, c27, [c10, 26 * SZCREG]
	stp	c28, c29, [c10, 28 * SZCREG]
	str     c30,      [c10, 30 * SZCREG]

	/* Place LR into the saved PC, this will ensure that when
	   switching to this saved context with setcontext() control
	   will pass back to the caller of getcontext(), we have
	   already arrange to return the appropriate return value in x0
	   above.  */
	str	c30, [c9, oPCC]

	/* Save the current CSP */
	mov	c2, csp
	str     c2, [c9, oCSP]

	/* Initialize the pstate.  */
	str	xzr, [c0, oPSTATE]

	/* Figure out where to place the first context extension
	   block.  */
	add     c2, c0, #oEXTENSION

	/* Write the context extension fpsimd header.  */
	mov	w3, #(FPSIMD_MAGIC & 0xffff)
	movk	w3, #(FPSIMD_MAGIC >> 16), lsl #16
	str	w3, [c2, #oHEAD + oMAGIC]
	mov	w3, #FPSIMD_CONTEXT_SIZE
	str	w3, [c2, #oHEAD + oSIZE]

	/* Fill in the FP SIMD context.  */
	add	c3, c2, #oV0 + 8 * SZVREG
	stp	 q8,  q9, [c3], # 2 * SZVREG
	stp	q10, q11, [c3], # 2 * SZVREG
	stp	q12, q13, [c3], # 2 * SZVREG
	stp	q14, q15, [c3], # 2 * SZVREG

	add	c3, c2, oFPSR

	mrs	x4, fpsr
	str	w4, [c3]

	mrs	x4, fpcr
	str	w4, [c3, oFPCR - oFPSR]

	/* Write the termination context extension header after the Morello
	   context.  */
	add	c2, c2, #FPSIMD_CONTEXT_SIZE + MORELLO_CONTEXT_SIZE
	str	wzr, [c2, #oHEAD + oMAGIC]
	str	wzr, [c2, #oHEAD + oSIZE]

	/* Grab the signal mask */
	/* rt_sigprocmask (SIG_BLOCK, NULL, &ucp->uc_sigmask, _NSIG8) */
	add	c2, c0, #UCONTEXT_SIGMASK
	mov	x0, SIG_BLOCK
	mov	x1, 0
	mov	x3, _NSIG8
	mov	x8, SYS_ify (rt_sigprocmask)
	svc	0
	cbnz	x0, 1f

	/* Return 0 for success */
	mov	x0, 0
	RET
1:
	b	C_SYMBOL_NAME(__syscall_error)

	PSEUDO_END (__getcontext)
weak_alias (__getcontext, getcontext)
