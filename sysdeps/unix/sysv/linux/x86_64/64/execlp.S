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

	/* Save the portions of the argv argument list in registers.  */
	push	%r9
	cfi_adjust_cfa_offset(8)
	push	%r8
	cfi_adjust_cfa_offset(8)
	push	%rcx
	cfi_adjust_cfa_offset(8)
	push	%rdx
	cfi_adjust_cfa_offset(8)
	push	%rsi
	cfi_adjust_cfa_offset(8)

	/* Load the address of the argv array.  */
	mov	%rsp, %rsi

	/* Restore return address to the stack.  */
	push	%rax
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rip, 0)

	call	HIDDEN_JUMPTARGET (execvp)

	/* Pop all of the extra stack space in one go.  */
	ret	$40
END(execlp)

libc_hidden_def (execlp)
