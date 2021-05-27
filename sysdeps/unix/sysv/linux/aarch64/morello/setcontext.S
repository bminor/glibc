/* Set current context.

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

/*  int __setcontext (const ucontext_t *ucp)

  Restores the machine context in UCP and thereby resumes execution
  in that context.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to restore anything
  other than the PRESERVED state.  */

	.text

ENTRY (__setcontext)
	/* Save a copy of UCP.  */
	mov	c9, c0

	/* Set the signal mask with
	   rt_sigprocmask (SIG_SETMASK, mask, NULL, _NSIG/8).  */
	mov	x0, #SIG_SETMASK
	add	c1, c9, #UCONTEXT_SIGMASK
	mov	x2, #0
	mov	x3, #_NSIG8
	mov	x8, SYS_ify (rt_sigprocmask)
	svc	0
	cbz	x0, 1f
	b	C_SYMBOL_NAME (__syscall_error)
1:
	/* Restore the capability registers. For Morello, they are saved after the
	   FPSIMD context.  */
	mov	c0, c9
	cfi_def_cfa (c0, 0)
	cfi_offset (c18, oC0 + 18 * SZCREG)
	cfi_offset (c19, oC0 + 19 * SZCREG)
	cfi_offset (c20, oC0 + 20 * SZCREG)
	cfi_offset (c21, oC0 + 21 * SZCREG)
	cfi_offset (c22, oC0 + 22 * SZCREG)
	cfi_offset (c23, oC0 + 23 * SZCREG)
	cfi_offset (c24, oC0 + 24 * SZCREG)
	cfi_offset (c25, oC0 + 25 * SZCREG)
	cfi_offset (c26, oC0 + 26 * SZCREG)
	cfi_offset (c27, oC0 + 27 * SZCREG)
	cfi_offset (c28, oC0 + 28 * SZCREG)
	cfi_offset (c29, oC0 + 29 * SZCREG)
	cfi_offset (c30, oC0 + 30 * SZCREG)

	cfi_offset ( d8, oV0 + 8 * SZVREG)
	cfi_offset ( d9, oV0 + 9 * SZVREG)
	cfi_offset (d10, oV0 + 10 * SZVREG)
	cfi_offset (d11, oV0 + 11 * SZVREG)
	cfi_offset (d12, oV0 + 12 * SZVREG)
	cfi_offset (d13, oV0 + 13 * SZVREG)
	cfi_offset (d14, oV0 + 14 * SZVREG)
	cfi_offset (d15, oV0 + 15 * SZVREG)

	add	c9, c0, #oEXTENSION + FPSIMD_CONTEXT_SIZE
	add	c10, c9, #oC0
	ldp	c18, c19, [c10, 18 * SZCREG]
	ldp	c20, c21, [c10, 20 * SZCREG]
	ldp	c22, c23, [c10, 22 * SZCREG]
	ldp	c24, c25, [c10, 24 * SZCREG]
	ldp	c26, c27, [c10, 26 * SZCREG]
	ldp	c28, c29, [c10, 28 * SZCREG]
	ldr     c30,      [c10, 30 * SZCREG]
	ldr     c2, [c9, oCSP]
	mov	csp, c2

	/* Check for FP SIMD context.  We don't support restoring
	   contexts created by the kernel, so this context must have
	   been created by getcontext.  Hence we can rely on the
	   first extension block being the FP SIMD context.  */
	add     c2, c0, #oEXTENSION

	mov	w3, #(FPSIMD_MAGIC & 0xffff)
	movk	w3, #(FPSIMD_MAGIC >> 16), lsl #16
	ldr	w1, [c2, #oHEAD + oMAGIC]
	cmp	w1, w3
	b.ne	2f

	/* Restore the FP SIMD context.  */
	add	c3, c2, #oV0 + 8 * SZVREG
	ldp	 q8,  q9, [c3], #2 * SZVREG
	ldp	q10, q11, [c3], #2 * SZVREG
	ldp	q12, q13, [c3], #2 * SZVREG
	ldp	q14, q15, [c3], #2 * SZVREG

	add	c3, c2, oFPSR

	ldr	w4, [c3]
	msr	fpsr, x4

	ldr	w4, [c3, oFPCR - oFPSR]
	msr	fpcr, x4

2:
	ldr     c16, [c9, oPCC]
	/* Restore arg registers.  */
	ldp	c2, c3, [c10, 2 * SZCREG]
	ldp	c4, c5, [c10, 4 * SZCREG]
	ldp	c6, c7, [c10, 6 * SZCREG]
	ldp	c0, c1, [c10, 0 * SZCREG]
	/* Jump to the new pc value.  */
	br	c16
PSEUDO_END (__setcontext)
weak_alias (__setcontext, setcontext)

ENTRY (__startcontext)
	mov	c0, c19
	cbnz	x0, __setcontext
1:	b       HIDDEN_JUMPTARGET (exit)
END (__startcontext)
