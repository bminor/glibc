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

ENTRY(execl)
	mov	4(%esp), %edx
	lea	8(%esp), %eax

	push	%eax			/* alignment padding */
	cfi_adjust_cfa_offset(4)
	push	%eax			/* create argv argument */
	cfi_adjust_cfa_offset(4)
	push	%edx			/* create path argument */
	cfi_adjust_cfa_offset(4)

	/* Let execv deal with pic vs non-pic loading of __environ.  */
	call	HIDDEN_JUMPTARGET (execv)

	add	$12, %esp
	cfi_adjust_cfa_offset(-12)
	ret
END(execl)

libc_hidden_def (execl)
