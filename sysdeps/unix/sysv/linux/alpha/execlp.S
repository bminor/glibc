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
	cfi_startproc
	ldgp	gp, 0(pv)
	lda	sp, -48(sp)
	cfi_adjust_cfa_offset(48)
	stq	ra, 0(sp)
	cfi_rel_offset(ra, 0)
	.prologue 1

	/* Save the portions of the argv argument list in registers.  */
	stq	a5, 40(sp)
	stq	a4, 32(sp)
	stq	a3, 24(sp)
	stq	a2, 16(sp)
	stq	a1,  8(sp)

	/* Load the argv and envp arguments; path is already in place.  */
	lda	a1, 8(sp)
	ldq	a2, __environ
#ifdef PIC
	bsr	ra, __execvpe		!samegp
#else
	jsr	ra, __execvpe
	ldgp	gp, 0(ra)
#endif

	lda	sp, 48(sp)
	cfi_adjust_cfa_offset(-48)
	ret

END (execlp)
	cfi_endproc

libc_hidden_def (execlp)
