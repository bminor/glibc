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

ENTRY(execlp)
	/* Move return address into a register.  */
	pop	%rax
	cfi_adjust_cfa_offset(-8)
	cfi_register(%rip, %rax)

	/* Save the arguments in registers.  Stop as soon as we detect
	   the NULL terminator, as if we find one, we do not want to fall
	   into the on-stack conversion loop.  */
	sub	$24, %esp
	cfi_adjust_cfa_offset(24)

	mov	%edi, 4(%rsp)	/* argv[0] must be non-null.  */

	mov	%edx, 8(%rsp)
	test	%edx, %edx
	jz	9f

	mov	%ecx, 12(%rsp)
	test	%ecx, %ecx
	jz	9f

	mov	%r8d, 16(%rsp)
	test	%r8d, %r8d
	jz	9f

	mov	%r9d, 20(%rsp)
	test	%r9d, %r9d
	jz	9f

	/* Convert the on-stack pointer arguments to in-place
	   from a 64-bit padded array into a 32-bit packed array.
           Note that this is memory is callee owned.  */
	xor	%ecx, %ecx
1:	mov	24(%rsp, %rcx, 8), %edx
	mov	%edx, 24(%rsp, %rcx, 4)
	inc	%ecx
	test	%edx, %edx
	jnz	1b

9:
	/* Restore return address to the stack.  */
	push	%rax
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rip, 0)

	/* Load argv parameter.  Note that path (esi) is already loaded.  */
	lea	12(%rsp), %edi

	call	HIDDEN_JUMPTARGET (execvp)

	/* Pop all of the extra stack space in one go.  */
	ret	$24

END(execlp)

libc_hidden_def (execlp)
