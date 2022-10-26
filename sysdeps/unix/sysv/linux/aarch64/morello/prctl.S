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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* prctl (int option, ...) with 4 optional variadic arguments.  */

ENTRY (__prctl)
	mov	x8, #__NR_prctl
	cbz	x9, L(skip_args)
	gclen   x10, c9
	ldr	c1, [c9]
	cmp	x10, 16
	bls	L(skip_args)
	ldr	c2, [c9,16]
	cmp	x10, 32
	bls	L(skip_args)
	ldr	c3, [c9,32]
	cmp	x10, 48
	bls	L(skip_args)
	ldr	c4, [c9,48]
L(skip_args):
	svc	0x0
	cmn	x0, #4095
	b.cs	.Lsyscall_error
	ret
PSEUDO_END (__prctl)
libc_hidden_def (__prctl)
weak_alias (__prctl, prctl)
