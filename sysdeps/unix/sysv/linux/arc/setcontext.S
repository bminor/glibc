/* Set current context for ARC.
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

/* int setcontext (const ucontext_t *ucp)
     - Restores the machine context in @ucp and resumes execution
       (doesn't return to caller).  */

ENTRY (__setcontext)

	mov  r9, r0	/* Stash @ucp across syscall.  */

	/* rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, NULL, _NSIG8) */
	mov  r3, _NSIG8
	mov  r2, 0
	add  r1, r0, UCONTEXT_SIGMASK
	mov  r0, SIG_SETMASK
	mov  r8, __NR_rt_sigprocmask
	ARC_TRAP_INSN
	brhi r0, -1024, .Lcall_syscall_err

	/* Restore scratch/arg regs for makecontext() case.  */
	LOAD_REG (r0,    r9, 22)
	LOAD_REG (r1,    r9, 21)
	LOAD_REG (r2,    r9, 20)
	LOAD_REG (r3,    r9, 19)
	LOAD_REG (r4,    r9, 18)
	LOAD_REG (r5,    r9, 17)
	LOAD_REG (r6,    r9, 16)
	LOAD_REG (r7,    r9, 15)

	/* Restore callee saved registers.  */
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

PSEUDO_END (__setcontext)
weak_alias (__setcontext, setcontext)


/* Helper for activating makecontext() created context
     - r13 has @func, r14 has uc_link.  */

ENTRY (__startcontext)

	.cfi_label .Ldummy
	cfi_undefined (blink)

        /* Call user @func, loaded in r13 by setcontext().  */
        jl   [r13]

        /* If uc_link (r14) call setcontext with that.  */
        mov  r0, r14
        breq r0, 0, 1f

        bl   __setcontext
1:
        /* Exit with status 0.  */
        b    HIDDEN_JUMPTARGET(exit)
END (__startcontext)
