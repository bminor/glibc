/* Copyright (C) 2022 Free Software Foundation, Inc.

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
#include <jmpbuf-offsets.h>
#include <stap-probe.h>

/* __longjmp(jmpbuf, val) */

ENTRY (__longjmp)
	cfi_def_cfa(c0, 0)
	cfi_offset(c19, JB_X19<<4)
	cfi_offset(c20, JB_X20<<4)
	cfi_offset(c21, JB_X21<<4)
	cfi_offset(c22, JB_X22<<4)
	cfi_offset(c23, JB_X23<<4)
	cfi_offset(c24, JB_X24<<4)
	cfi_offset(c25, JB_X25<<4)
	cfi_offset(c26, JB_X26<<4)
	cfi_offset(c27, JB_X27<<4)
	cfi_offset(c28, JB_X28<<4)
	cfi_offset(c29, JB_X29<<4)
	cfi_offset(c30, JB_LR<<4)

	cfi_offset(q8, JB_D8<<4)
	cfi_offset(q9, JB_D9<<4)
	cfi_offset(q10, JB_D10<<4)
	cfi_offset(q11, JB_D11<<4)
	cfi_offset(q12, JB_D12<<4)
	cfi_offset(q13, JB_D13<<4)
	cfi_offset(q14, JB_D14<<4)
	cfi_offset(q15, JB_D15<<4)

	ldp	c19, c20, [c0, #JB_X19<<4]
	ldp	c21, c22, [c0, #JB_X21<<4]
	ldp	c23, c24, [c0, #JB_X23<<4]
	ldp	c25, c26, [c0, #JB_X25<<4]
	ldp	c27, c28, [c0, #JB_X27<<4]
	ldp	c29, c30, [c0, #JB_X29<<4]

	/* longjmp probe takes 3 arguments, address of jump buffer as
	   first argument (8@x0), return value as second argument (-4@x1),
	   and target address (8@x30), respectively.  */
	LIBC_PROBE (longjmp, 3, 8@c0, -4@c1, 8@c30)
	ldp	 q8,  q9, [c0, #JB_D8<<4]
	ldp	q10, q11, [c0, #JB_D10<<4]
	ldp	q12, q13, [c0, #JB_D12<<4]
	ldp	q14, q15, [c0, #JB_D14<<4]

        /* Originally this was implemented with a series of
	   .cfi_restore() directives.

           The theory was that cfi_restore should revert to previous
           frame value is the same as the current value.  In practice
           this doesn't work, even after cfi_restore() gdb continues
           to try to recover a previous frame value offset from x0,
           which gets stuffed after a few more instructions.  The
           cfi_same_value() mechanism appears to work fine.  */

	cfi_same_value(c19)
	cfi_same_value(c20)
	cfi_same_value(c21)
	cfi_same_value(c22)
	cfi_same_value(c23)
	cfi_same_value(c24)
	cfi_same_value(c25)
	cfi_same_value(c26)
	cfi_same_value(c27)
	cfi_same_value(c28)
	cfi_same_value(c29)
	cfi_same_value(c30)
	cfi_same_value(q8)
	cfi_same_value(q9)
	cfi_same_value(q10)
	cfi_same_value(q11)
	cfi_same_value(q12)
	cfi_same_value(q13)
	cfi_same_value(q14)
	cfi_same_value(q15)

	ldr	c5, [c0, #JB_SP<<4]
	mov	csp, c5

	/* longjmp_target probe takes 3 arguments, address of jump buffer
	   as first argument (8@x0), return value as second argument (-4@x1),
	   and target address (8@x30), respectively.  */
	LIBC_PROBE (longjmp_target, 3, 8@c0, -4@c1, 8@c30)
	cmp	x1, #0
	mov	x0, #1
	csel	x0, x1, x0, ne
	/* Use br instead of ret because ret is guaranteed to mispredict */
	br	c30
END (__longjmp)
