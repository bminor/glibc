/* Save and set current context for ARC.
   Copyright (C) 2009-2019 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "ucontext-macros.h"

/* int swapcontext (ucontext_t *oucp, const ucontext_t *ucp).  */

ENTRY (__swapcontext)

	/* Save context into @oucp pointed to by r0.  */

	SAVE_REG (r13,   r0, 37)
	SAVE_REG (r14,   r0, 36)
	SAVE_REG (r15,   r0, 35)
	SAVE_REG (r16,   r0, 34)
	SAVE_REG (r17,   r0, 33)
	SAVE_REG (r18,   r0, 32)
	SAVE_REG (r19,   r0, 31)
	SAVE_REG (r20,   r0, 30)
	SAVE_REG (r21,   r0, 29)
	SAVE_REG (r22,   r0, 28)
	SAVE_REG (r23,   r0, 27)
	SAVE_REG (r24,   r0, 26)
	SAVE_REG (r25,   r0, 25)

	SAVE_REG (blink, r0,  7)
	SAVE_REG (fp,    r0,  8)
	SAVE_REG (sp,    r0, 23)

	/* Save 0 in r0 placeholder to return 0 when @oucp activated.  */
	mov r9, 0
	SAVE_REG (r9,    r0, 22)

	/* Load context from @ucp.  */

	mov r9, r1	/* Safekeep @ucp across syscall.  */

	/* rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, &oucp->uc_sigmask, _NSIG8) */
	mov r3, _NSIG8
	add r2, r0, UCONTEXT_SIGMASK
	add r1, r1, UCONTEXT_SIGMASK
	mov r0, SIG_SETMASK
	mov r8, __NR_rt_sigprocmask
	ARC_TRAP_INSN
	brhi r0, -1024, .Lcall_syscall_err

	LOAD_REG (r0,    r9, 22)
	LOAD_REG (r1,    r9, 21)
	LOAD_REG (r2,    r9, 20)
	LOAD_REG (r3,    r9, 19)
	LOAD_REG (r4,    r9, 18)
	LOAD_REG (r5,    r9, 17)
	LOAD_REG (r6,    r9, 16)
	LOAD_REG (r7,    r9, 15)

	LOAD_REG (r13,   r9, 37)
	LOAD_REG (r14,   r9, 36)
	LOAD_REG (r15,   r9, 35)
	LOAD_REG (r16,   r9, 34)
	LOAD_REG (r17,   r9, 33)
	LOAD_REG (r18,   r9, 32)
	LOAD_REG (r19,   r9, 31)
	LOAD_REG (r20,   r9, 30)
	LOAD_REG (r21,   r9, 29)
	LOAD_REG (r22,   r9, 28)
	LOAD_REG (r23,   r9, 27)
	LOAD_REG (r24,   r9, 26)
	LOAD_REG (r25,   r9, 25)

	LOAD_REG (blink, r9,  7)
	LOAD_REG (fp,    r9,  8)
	LOAD_REG (sp,    r9, 23)

	j    [blink]

PSEUDO_END (__swapcontext)
weak_alias (__swapcontext, swapcontext)
