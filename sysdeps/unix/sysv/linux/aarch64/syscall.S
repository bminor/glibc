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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* syscall (int nr, ...)

   AArch64 system calls take between 0 and 7 arguments. On entry here nr
   is in w0 and any other system call arguments are in register x1..x7.

   For kernel entry we need to move the system call nr to x8 then
   load the remaining arguments to register. */

ENTRY (syscall)
	uxtw	x8, w0
#ifdef __CHERI_PURE_CAPABILITY__
	cbz	x9, L(skip_args)
	gclen   x10, c9
	ldr	c0, [c9]
	cmp	x10, 16
	bls	L(skip_args)
	ldr	c1, [c9,16]
	cmp	x10, 32
	bls	L(skip_args)
	ldr	c2, [c9,32]
	cmp	x10, 48
	bls	L(skip_args)
	ldr	c3, [c9,48]
	cmp	x10, 64
	bls	L(skip_args)
	ldr	c4, [c9,64]
	cmp	x10, 80
	bls	L(skip_args)
	ldr	c5, [c9,80]
	cmp	x10, 96
	bls	L(skip_args)
	ldr	c6, [c9,96]
L(skip_args):
#else
	mov	x0, x1
	mov	x1, x2
	mov	x2, x3
	mov	x3, x4
	mov	x4, x5
	mov	x5, x6
	mov	x6, x7
#endif
	svc	0x0
	cmn	x0, #4095
	b.cs	1f
	RET
1:
	b	SYSCALL_ERROR
PSEUDO_END (syscall)
