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

	/* Morello we get called with:
	   ip0		&PLTGOT[2]
	   ip1		temp(dl resolver entry point)
	   [csp, #16]	lr
	   [csp, #0]	&PLTGOT[n]

	   Stack frame layout:
	   [csp,   #...] lr
	   [csp,   #...] &PLTGOT[n]
	   [csp,    #192] La_aarch64_regs
	   [csp,    #96] La_aarch64_retval
	   [csp,    #80] frame size return from pltenter
	   [csp,    #64] dl_profile_call saved c1
	   [csp,    #48] dl_profile_call saved c0
	   [csp,    #32] t1
	   [csp,     #0] c29, lr   <- c29
	 */

# define OFFSET_T1		32
# define OFFSET_SAVED_CALL_X0	OFFSET_T1 + 16
# define OFFSET_FS		OFFSET_SAVED_CALL_X0 + 32
# define OFFSET_RV		OFFSET_FS + 16
# define OFFSET_RG		OFFSET_RV + DL_SIZEOF_RV

# define SF_SIZE		OFFSET_RG + DL_SIZEOF_RG

# define OFFSET_PLTGOTN		SF_SIZE
# define OFFSET_LR		OFFSET_PLTGOTN + 16

	/* Save arguments.  */
	mov	c11, csp
	sub	x11, x11, #SF_SIZE
	scvalue	csp, csp, x11
	cfi_adjust_cfa_offset (SF_SIZE)
	stp	c29, c30, [csp, #0]
	mov	c29, csp
	cfi_def_cfa_register (c29)
	cfi_rel_offset (c29, 0)
	cfi_rel_offset (lr, 8)

	stp	c0, c1, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*0]
	cfi_rel_offset (c0, OFFSET_RG + DL_OFFSET_RG_X0 + 32*0 + 0)
	cfi_rel_offset (c1, OFFSET_RG + DL_OFFSET_RG_X0 + 32*0 + 16)
	stp	c2, c3, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*1]
	cfi_rel_offset (c2, OFFSET_RG + DL_OFFSET_RG_X0 + 32*1 + 0)
	cfi_rel_offset (c3, OFFSET_RG + DL_OFFSET_RG_X0 + 32*1 + 16)
	stp	c4, c5, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*2]
	cfi_rel_offset (c4, OFFSET_RG + DL_OFFSET_RG_X0 + 32*2 + 0)
	cfi_rel_offset (c5, OFFSET_RG + DL_OFFSET_RG_X0 + 32*2 + 16)
	stp	c6, c7, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*3]
	cfi_rel_offset (c6, OFFSET_RG + DL_OFFSET_RG_X0 + 32*3 + 0)
	cfi_rel_offset (c7, OFFSET_RG + DL_OFFSET_RG_X0 + 32*3 + 16)

	stp	q0, q1, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	cfi_rel_offset (q0, OFFSET_RG + DL_OFFSET_RG_V0 + 32*0)
	cfi_rel_offset (q1, OFFSET_RG + DL_OFFSET_RG_V0 + 32*0 + 16)
	stp	q2, q3, [c29, #OFFSET_RG+ DL_OFFSET_RG_V0 + 32*1]
	cfi_rel_offset (q2, OFFSET_RG + DL_OFFSET_RG_V0 + 32*1 + 0)
	cfi_rel_offset (q3, OFFSET_RG + DL_OFFSET_RG_V0 + 32*1 + 16)
	stp	q4, q5, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	cfi_rel_offset (q4, OFFSET_RG + DL_OFFSET_RG_V0 + 32*2 + 0)
	cfi_rel_offset (q5, OFFSET_RG + DL_OFFSET_RG_V0 + 32*2 + 16)
	stp	q6, q7, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]
	cfi_rel_offset (q6, OFFSET_RG + DL_OFFSET_RG_V0 + 32*3 + 0)
	cfi_rel_offset (q7, OFFSET_RG + DL_OFFSET_RG_V0 + 32*3 + 16)

	add	c0, c29, #SF_SIZE + 32
	ldr	c1, [c29, #OFFSET_LR]
	stp	c0, c1, [c29, #OFFSET_RG + DL_OFFSET_RG_SP]

	/* Get pointer to linker struct.  */
	ldr	c0, [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_profile_fixup().  */
	ldr	c1, [c29, OFFSET_PLTGOTN]	/* Recover &PLTGOT[n] */

	mov	c11, c1
	sub	x1, x1, x16
	add	x1, x1, x1, lsl #1
	lsl	x1, x1, #3
	sub	x1, x1, #(RELA_SIZE<<3)
	lsr	x1, x1, #3
	scvalue	c1, c11, x1

	stp	c0, c1, [c29, #OFFSET_SAVED_CALL_X0]

	/* Set up extra args for _dl_profile_fixup */
	ldr	c2, [c29, #OFFSET_LR]		/* load saved LR */
	add	c3, c29, #OFFSET_RG		/* address of La_aarch64_reg */
	add	c4, c29, #OFFSET_FS		/* address of framesize */
	bl	_dl_profile_fixup

	ldr	ip0l, [c29, #OFFSET_FS]		/* framesize == 0 */
	cmp	x16, #0
	bge	1f
	cfi_remember_state

	/* Save the return.  */
	mov	ip0, c0

	/* Get arguments and return address back.  */
	ldp	c0, c1, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*0]
	ldp	c2, c3, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*1]
	ldp	c4, c5, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*2]
	ldp	c6, c7, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*3]
	ldp	q0, q1, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	ldp	q2, q3, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*1]
	ldp	q4, q5, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	ldp	q6, q7, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]

	cfi_def_cfa_register (csp)
	ldp	c29, c30, [c29, #0]
	cfi_restore(c29)
	cfi_restore(c30)

	add	csp, csp, SF_SIZE + 16
	cfi_adjust_cfa_offset (- SF_SIZE - 32)

	/* Jump to the newly found address.  */
	br	ip0

	cfi_restore_state
1:
	/* The new frame size is in ip0.  */

	mov	c11, c1
	sub	x1, x29, x16
	and	x12, x1, #0xfffffffffffffff0
	scvalue	csp, csp, x12
	scvalue	c1, c11, x1

	str	c0, [c29, #OFFSET_T1]

	mov	c0, csp
	add	c1, c29, #SF_SIZE + 16
	mov	c2, ip0
	bl	memcpy

	ldr	ip0, [c29, #OFFSET_T1]

	/* Call the function.  */
	ldp	c0, c1, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*0]
	ldp	c2, c3, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*1]
	ldp	c4, c5, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*2]
	ldp	c6, c7, [c29, #OFFSET_RG + DL_OFFSET_RG_X0 + 32*3]
	ldp	q0, q1, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	ldp	q2, q3, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*1]
	ldp	q4, q5, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	ldp	q6, q7, [c29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]
	blr	ip0
	stp	c0, c1, [c29, #OFFSET_RV + DL_OFFSET_RV_X0]
	stp	q0, q1, [c29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*0]
	stp	q2, q3, [c29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*1]

	/* Setup call to pltexit  */
	ldp	c0, c1, [c29, #OFFSET_SAVED_CALL_X0]
	add	c2, c29, #OFFSET_RG
	add	c3, c29, #OFFSET_RV
	bl	_dl_audit_pltexit

	ldp	c0, c1, [c29, #OFFSET_RV + DL_OFFSET_RV_X0]
	ldp	d0, d1, [c29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*0]
	ldp	d2, d3, [c29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*1]
	/* LR from within La_aarch64_reg */
	ldr	lr, [c29, #OFFSET_RG + DL_OFFSET_RG_LR]
	cfi_restore(lr)

	mov	csp, c29
	cfi_def_cfa_register (csp)
	ldr	c29, [c29, #0]
	cfi_restore(c29)
	add	csp, csp, SF_SIZE + 32
	cfi_adjust_cfa_offset (- SF_SIZE - 32)

	br	lr

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous
