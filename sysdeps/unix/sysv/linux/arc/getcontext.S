/* Save current context for ARC.
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

/* int getcontext (ucontext_t *ucp)
   Save machine context in @ucp and return 0 on success, -1 on error
    - saves callee saved registers only
    - layout mandated by uncontext_t:m_context (hence different from setjmp).  */

ENTRY (__getcontext)

	/* Callee saved registers.  */
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

	/* Save 0 in r0 placeholder to return 0 when this @ucp activated.  */
	mov r9, 0
	SAVE_REG (r9,    r0, 22)

	/* rt_sigprocmask (SIG_BLOCK, NULL, &ucp->uc_sigmask, _NSIG8).  */
	mov r3, _NSIG8
	add r2, r0, UCONTEXT_SIGMASK
	mov r1, 0
	mov r0, SIG_BLOCK
	mov r8, __NR_rt_sigprocmask
	ARC_TRAP_INSN
	brhi    r0, -1024, .Lcall_syscall_err
	j.d	[blink]
	mov r0, 0	/* Success, error handled in .Lcall_syscall_err.  */

PSEUDO_END (__getcontext)
weak_alias (__getcontext, getcontext)
