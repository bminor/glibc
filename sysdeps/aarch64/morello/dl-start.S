/* ld.so _start code.
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

#include <sysdep.h>

ENTRY(_start)

	/* Create an initial frame with 0 LR and FP */
	cfi_undefined (c30)
	mov	c29, czr
	mov	c30, czr
	mov	c0, csp
	bl	_dl_start
	/* Returns user entry point in x0.  */
	mov	c21, c0
.globl _dl_start_user
.type _dl_start_user, %function
_dl_start_user:
	/* Get argc.  */
	ldr	x1, [csp]
	/* Get argv.  */
	add	c2, csp, 16
	/* Compute envp.  */
	add	c3, c2, x1, lsl 4
	add	c3, c3, 16

	adrp	c16, capinit_rtld_local
	ldr	c16, [c16, :lo12:capinit_rtld_local]
	ldr	c0, [c16]
	bl	_dl_init
	/* Load the finalizer function.  */
	adrp	c0, capinit_dl_fini
	ldr	c0, [c0, :lo12:capinit_dl_fini]
	/* Jump to the user's entry point.  */
	mov     c16, c21
	br      c16
END(_start)

	.section	.data.rel.ro.local,"aw"
	.align	4
	.type	capinit_rtld_local, %object
	.size	capinit_rtld_local, 16
capinit_rtld_local:
	.capinit	_rtld_local
	.xword	0
	.xword	0
	.type	capinit_dl_fini, %object
	.size	capinit_dl_fini, 16
capinit_dl_fini:
	.capinit	_dl_fini
	.xword	0
	.xword	0
