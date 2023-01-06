/* Swap two contexts.  OpenRISC version.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include "ucontext_i.h"

/* int swapcontext (ucontext_t *oucp, const ucontext_t *ucp) */
	.text
ENTRY(__swapcontext)

	/* Same as getcontext.  */
	/* Store r1, the stack pointer.  */
	l.sw	(UCONTEXT_MCONTEXT +  1*4)(r3), r1
	/* Store r2, the frame pointer.  */
	l.sw	(UCONTEXT_MCONTEXT +  2*4)(r3), r2
	/* Store r9, the link register.  */
	l.sw	(UCONTEXT_MCONTEXT +  9*4)(r3), r9
	/* Store r9 to reg[11] too, as we need two links for makecontext.  */
	l.sw	(UCONTEXT_MCONTEXT + 11*4)(r3), r9
	/* Store r10, the TLS register.  */
	l.sw	(UCONTEXT_MCONTEXT + 10*4)(r3), r10
	/* Store r14-r30 even, callee saved registers.  */
	l.sw	(UCONTEXT_MCONTEXT + 14*4)(r3), r14
	l.sw	(UCONTEXT_MCONTEXT + 16*4)(r3), r16
	l.sw	(UCONTEXT_MCONTEXT + 18*4)(r3), r18
	l.sw	(UCONTEXT_MCONTEXT + 20*4)(r3), r20
	l.sw	(UCONTEXT_MCONTEXT + 22*4)(r3), r22
	l.sw	(UCONTEXT_MCONTEXT + 24*4)(r3), r24
	l.sw	(UCONTEXT_MCONTEXT + 26*4)(r3), r26
	l.sw	(UCONTEXT_MCONTEXT + 28*4)(r3), r28
	l.sw	(UCONTEXT_MCONTEXT + 30*4)(r3), r30

	/* Store ucp to non-argument syscall preserved register.  */
	l.ori	r30, r4, 0

	/* Get signal mask.  */
	/* rt_sigprocmask (SIG_BLOCK, NULL, &ucp->uc_sigmask, _NSIG8) */
	l.ori	r6, r0, _NSIG8
	l.addi	r5, r3, UCONTEXT_SIGMASK
	l.ori	r4, r0, 0
	l.ori	r3, r0, SIG_BLOCK
	l.ori	r11, r0, SYS_ify (rt_sigprocmask)
	/* Do the syscall.  */
	l.sys	1
	 l.nop

	/* if -4096 < ret < 0 holds, it's an error */
	l.sfgeui r11, 0xf001
	l.bf	1f
	 l.nop

	/* Same as setcontext.  */

	/* Restore signal mask.  */
	/* rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, NULL, _NSIG8) */
	l.ori	r6, r0, _NSIG8
	l.ori	r5, r0, 0
	l.addi	r4, r30, UCONTEXT_SIGMASK
	l.ori	r3, r0, SIG_SETMASK
	l.ori	r11, r0, SYS_ify (rt_sigprocmask)
	/* Do the syscall.  */
	l.sys 1
	 l.nop

	/* if -4096 < ret < 0 holds, it's an error */
	l.sfgeui r11, 0xf001
	l.bf	1f
	 l.nop

	/* Restore argument registers, for the makecontext case.  */
	l.lwz	r3, (UCONTEXT_MCONTEXT +  3*4)(r30)
	l.lwz	r4, (UCONTEXT_MCONTEXT +  4*4)(r30)
	l.lwz	r5, (UCONTEXT_MCONTEXT +  5*4)(r30)
	l.lwz	r6, (UCONTEXT_MCONTEXT +  6*4)(r30)
	l.lwz	r7, (UCONTEXT_MCONTEXT +  7*4)(r30)
	l.lwz	r8, (UCONTEXT_MCONTEXT +  8*4)(r30)

	/* Restore registers stored in getcontext.  */
	l.lwz	r1,  (UCONTEXT_MCONTEXT +  1*4)(r30)
	l.lwz	r2,  (UCONTEXT_MCONTEXT +  2*4)(r30)
	l.lwz	r9,  (UCONTEXT_MCONTEXT +  9*4)(r30)
	l.lwz	r10, (UCONTEXT_MCONTEXT + 10*4)(r30)
	l.lwz	r11, (UCONTEXT_MCONTEXT + 11*4)(r30)
	l.lwz	r14, (UCONTEXT_MCONTEXT + 14*4)(r30)
	l.lwz	r16, (UCONTEXT_MCONTEXT + 16*4)(r30)
	l.lwz	r18, (UCONTEXT_MCONTEXT + 18*4)(r30)
	l.lwz	r20, (UCONTEXT_MCONTEXT + 20*4)(r30)
	l.lwz	r22, (UCONTEXT_MCONTEXT + 22*4)(r30)
	l.lwz	r24, (UCONTEXT_MCONTEXT + 24*4)(r30)
	l.lwz	r26, (UCONTEXT_MCONTEXT + 26*4)(r30)
	l.lwz	r28, (UCONTEXT_MCONTEXT + 28*4)(r30)
	l.lwz	r30, (UCONTEXT_MCONTEXT + 30*4)(r30)

	l.jr	r11
	 l.ori	r11, r0, 0

1:	l.j	__syscall_error
	 l.ori	r3, r11, 0

END (__swapcontext)
weak_alias (__swapcontext, swapcontext)
