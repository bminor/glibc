/* Copyright (C) 2016 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

ENTRY(execle)
	lea	8(%esp), %eax	/* find argv array */

	/* Find the env argument.  It is the array element after the argv
	   NULL terminator, which cannot be located before argv[1].  */
	mov	4(%eax), %edx
	xor	%ecx, %ecx
1:	inc	%ecx
	test	%edx, %edx
	mov	4(%eax, %ecx, 4), %edx
	jnz	1b

	push	%edx		/* create env argument */
	cfi_adjust_cfa_offset(4)
	push	%eax		/* create argv argument */
	cfi_adjust_cfa_offset(4)
	push	12(%esp)	/* copy path argument */
	cfi_adjust_cfa_offset(4)

	call	HIDDEN_JUMPTARGET (execve)

	add	$12, %esp
	cfi_adjust_cfa_offset(-12)
	ret
END(execle)

libc_hidden_def (execle)
