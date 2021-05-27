/* Modify saved context.

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

/* int swapcontext (ucontext_t *oucp, const ucontext_t *ucp) */

	.text
ENTRY(__swapcontext)
	/* Set the value returned when swapcontext() returns in this context.
	   And set up x1 to become the return address of the caller, so we
	   can return there with a normal RET instead of an indirect jump.  */

	/* For Morello, the capability registers are located in after the FPSIMD
	   context.  */
	add	c9, c0, #oEXTENSION + FPSIMD_CONTEXT_SIZE

	/* Write the context extension morello header.  */
	mov	w3, #(MORELLO_MAGIC & 0xffff)
	movk	w3, #(MORELLO_MAGIC >> 16), lsl #16
	str	w3, [c9, #oCHEAD + oMAGIC]
	mov	w3, #MORELLO_CONTEXT_SIZE
	str	w3, [c9, #oCHEAD + oSIZE]

	add	c10, c9, #oC0
	stp	czr, c30, [c10, 0 * SZCREG]
	/* Arrange the oucp context to return to 2f.  */
	adr	c30, 2f

	stp	c18, c19, [c10, 18 * SZCREG]
	stp	c20, c21, [c10, 20 * SZCREG]
	stp	c22, c23, [c10, 22 * SZCREG]
	stp	c24, c25, [c10, 24 * SZCREG]
	stp	c26, c27, [c10, 26 * SZCREG]
	stp	c28, c29, [c10, 28 * SZCREG]
	str     c30,      [c10, 30 * SZCREG]
	str     c30,      [c9, oPCC]
	mov	c11, csp
	str     c11,       [c9, oCSP]

	/* Figure out where to place the fpsimd context extension
	   block.  */
	add     c9, c0, #oEXTENSION

	/* Write the context extension fpsimd header.  */
	mov	w3, #(FPSIMD_MAGIC & 0xffff)
	movk	w3, #(FPSIMD_MAGIC >> 16), lsl #16
	str	w3, [c9, #oHEAD + oMAGIC]
	mov	w3, #FPSIMD_CONTEXT_SIZE
	str	w3, [c9, #oHEAD + oSIZE]

	/* Fill in the FP SIMD context.  */
	add	c10, c9, #oV0 + 8 * SZVREG
	stp	 q8,  q9, [c10], #2 * SZVREG
	stp	q10, q11, [c10], #2 * SZVREG
	stp	q12, q13, [c10], #2 * SZVREG
	stp	q14, q15, [c10], #2 * SZVREG

	add	c10, c9, #oFPSR
	mrs	x4, fpsr
	str	w4, [c10, #oFPSR - oFPSR]
	mrs	x4, fpcr
	str	w4, [c10, #oFPCR - oFPSR]

	/* Write the termination context extension header after the Morello
	   context.  */
	add	c9, c9, #FPSIMD_CONTEXT_SIZE + MORELLO_CONTEXT_SIZE
	str	wzr, [c9, #oHEAD + oMAGIC]
	str	wzr, [c9, #oHEAD + oSIZE]

	/* Preserve ucp.  */
	mov	c11, c1

	/* rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, &oucp->uc_sigmask,
			   _NSIG8) */
	/* Grab the signal mask */
	/* rt_sigprocmask (SIG_BLOCK, NULL, &ucp->uc_sigmask, _NSIG8) */
	add	c2, c0, #UCONTEXT_SIGMASK
	mov	x0, SIG_BLOCK
	mov	x1, 0
	mov	x3, _NSIG8
	mov	x8, SYS_ify (rt_sigprocmask)
	svc	0
	cbnz	x0, 1f

	mov     c15, c30
	mov	c0, c11
	bl	JUMPTARGET (__setcontext)
	mov     c30, c15
	RET

1:
	b	C_SYMBOL_NAME(__syscall_error)
2:
	/* The oucp context is restored here via an indirect branch,
	   x1 must be restored too which has the real return address.  */
	mov	c30, c1
	RET
PSEUDO_END (__swapcontext)
weak_alias (__swapcontext, swapcontext)
