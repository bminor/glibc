/* Copyright (C) 2005-2022 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <libc-symbols.h>

#include "dl-link.h"

#define ip0 c16
#define ip0l c16
#define ip1 c17
#define lr  c30

#define RELA_SIZE 24

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, #function
	cfi_startproc
	.align 2
_dl_runtime_resolve:
	/* Morello we get called with:
	   ip0		&PLTGOT[2]
	   ip1		temp(dl resolver entry point)
	   [sp, #16]	lr
	   [sp, #0]	&PLTGOT[n]
	 */

	cfi_rel_offset (lr, 16)

	/* Save arguments.  */
	stp	c8, c9, [csp, #-(160+8*16)]!
	cfi_adjust_cfa_offset (160+8*16)
	cfi_rel_offset (c8, 0)
	cfi_rel_offset (c9, 16)

	stp	c6, c7, [csp,  #32]
	cfi_rel_offset (c6, 32)
	cfi_rel_offset (c7, 48)

	stp	c4, c5, [csp,  #64]
	cfi_rel_offset (c4, 64)
	cfi_rel_offset (c5, 80)

	stp	c2, c3, [csp,  #96]
	cfi_rel_offset (c2, 96)
	cfi_rel_offset (c3, 112)

	stp	c0, c1, [csp,  #128]
	cfi_rel_offset (c0, 128)
	cfi_rel_offset (c1, 144)

	stp	q0, q1, [csp, #(160+0*16)]
	cfi_rel_offset (q0, 160+0*16)
	cfi_rel_offset (q1, 160+1*16)

	stp	q2, q3, [csp, #(160+2*16)]
	cfi_rel_offset (q0, 160+2*16)
	cfi_rel_offset (q1, 160+3*16)

	stp	q4, q5, [csp, #(160+4*16)]
	cfi_rel_offset (q0, 160+4*16)
	cfi_rel_offset (q1, 160+5*16)

	stp	q6, q7, [csp, #(160+6*16)]
	cfi_rel_offset (q0, 160+6*16)
	cfi_rel_offset (q1, 160+7*16)

	/* Get pointer to linker struct.  */
	ldr	c0, [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_fixup().  */
	ldr	c1, [csp, 160+8*16]	/* Recover &PLTGOT[n] */

	sub	x1, x1, x16
	add	x1, x1, x1, lsl #1
	lsr	x1, x1, #1
	sub	x1, x1, #RELA_SIZE

	/* Call fixup routine.  */
	bl	_dl_fixup

	/* Save the return.  */
	mov	ip0, c0

	/* Get arguments and return address back.  */
	ldp	q0, q1, [csp, #(160+0*16)]
	ldp	q2, q3, [csp, #(160+2*16)]
	ldp	q4, q5, [csp, #(160+4*16)]
	ldp	q6, q7, [csp, #(160+6*16)]
	ldp	c0, c1, [csp, #128]
	ldp	c2, c3, [csp, #96]
	ldp	c4, c5, [csp, #64]
	ldp	c6, c7, [csp, #32]
	ldp	c8, c9, [csp], #(160+8*16)
	cfi_adjust_cfa_offset (-(160+8*16))

	ldp	ip1, lr, [csp], #32
	cfi_adjust_cfa_offset (-32)

	/* Jump to the newly found address.  */
	br	ip0

	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve
#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	cfi_startproc
	.align 2
_dl_runtime_profile:

	/* TODO: requires definition of La_aarch64_* layout
	   and register state saved correctly for varargs ABI.  */
	mov c0, 0
	ldr c0, [c0]

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous
