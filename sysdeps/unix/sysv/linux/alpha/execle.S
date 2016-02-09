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
	cfi_startproc
	lda	sp, -48(sp)
	cfi_adjust_cfa_offset(48)
	.frame  sp, 48, ra
	.prologue 0

	/* Save the portions of the argv argument list in registers.  */
	stq	a5, 40(sp)
	stq	a4, 32(sp)
	stq	a3, 24(sp)
	stq	a2, 16(sp)
	stq	a1,  8(sp)

	/* Find the env argument.  It is the array element after the argv
	   NULL terminator, which cannot be located before argv[1].  */
	lda	t0, 16(sp)
1:	ldq	t1, 0(t0)
	addq	t0, 8, t0
	bne	t1, 1b

	/* Load the argv and envp arguments; path is already in place.  */
	lda	a1, 8(sp)
	ldq	a2, 0(t0)

	lda	v0, SYS_ify(execve)
	call_pal PAL_callsys

	/* Discard the stack frame now.  */
	lda	sp, 48(sp)
	cfi_adjust_cfa_offset(-48)

	/* All returns are errors.  */
	br	SYSCALL_ERROR_LABEL

PSEUDO_END (execle)
	cfi_endproc

libc_hidden_def (execle)
