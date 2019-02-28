/* Copyright (C) 2002-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <pthread-errnos.h>
#include <kernel-features.h>
#include <lowlevellock.h>

#include <stap-probe.h>

	.text

#define LOAD_PRIVATE_FUTEX_WAIT(reg) \
	movl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg
#define LOAD_PRIVATE_FUTEX_WAKE(reg) \
	movl	$(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), reg
#define LOAD_FUTEX_WAIT(reg) \
	xorl	$(FUTEX_WAIT | FUTEX_PRIVATE_FLAG), reg
#define LOAD_FUTEX_WAIT_ABS(reg) \
	xorl	$(FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME), reg
#define LOAD_FUTEX_WAKE(reg) \
	xorl	$(FUTEX_WAKE | FUTEX_PRIVATE_FLAG), reg


	.globl	__lll_lock_wait_private
	.type	__lll_lock_wait_private,@function
	.hidden	__lll_lock_wait_private
	.align	16
__lll_lock_wait_private:
	cfi_startproc
	pushq	%r10
	cfi_adjust_cfa_offset(8)
	pushq	%rdx
	cfi_adjust_cfa_offset(8)
	cfi_offset(%r10, -16)
	cfi_offset(%rdx, -24)
	xorq	%r10, %r10	/* No timeout.  */
	movl	$2, %edx
	LOAD_PRIVATE_FUTEX_WAIT (%esi)

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne	2f

1:	LIBC_PROBE (lll_lock_wait_private, 1, %rdi)
	movl	$SYS_futex, %eax
	syscall

2:	movl	%edx, %eax
	xchgl	%eax, (%rdi)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz	1b

	popq	%rdx
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rdx)
	popq	%r10
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r10)
	retq
	cfi_endproc
	.size	__lll_lock_wait_private,.-__lll_lock_wait_private

#if !IS_IN (libc)
	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	16
__lll_lock_wait:
	cfi_startproc
	pushq	%r10
	cfi_adjust_cfa_offset(8)
	pushq	%rdx
	cfi_adjust_cfa_offset(8)
	cfi_offset(%r10, -16)
	cfi_offset(%rdx, -24)
	xorq	%r10, %r10	/* No timeout.  */
	movl	$2, %edx
	LOAD_FUTEX_WAIT (%esi)

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne	2f

1:	LIBC_PROBE (lll_lock_wait, 2, %rdi, %rsi)
	movl	$SYS_futex, %eax
	syscall

2:	movl	%edx, %eax
	xchgl	%eax, (%rdi)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz	1b

	popq	%rdx
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rdx)
	popq	%r10
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r10)
	retq
	cfi_endproc
	.size	__lll_lock_wait,.-__lll_lock_wait

	/*      %rdi: futex
		%rsi: flags
		%rdx: timeout
		%eax: futex value
	*/
	.globl	__lll_timedlock_wait
	.type	__lll_timedlock_wait,@function
	.hidden	__lll_timedlock_wait
	.align	16
__lll_timedlock_wait:
	cfi_startproc
	cmpq	$0, (%rdx)
	js	5f

	pushq	%r9
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r9, 0)

	movq	%rdx, %r10
	movl	$0xffffffff, %r9d
	LOAD_FUTEX_WAIT_ABS (%esi)

	movl	$2, %edx
	cmpl	%edx, %eax
	jne	2f

1:	movl	$SYS_futex, %eax
	movl	$2, %edx
	syscall

2:	xchgl	%edx, (%rdi)	/* NB:   lock is implied */

	testl	%edx, %edx
	jz	3f

	cmpl	$-ETIMEDOUT, %eax
	je	4f
	cmpl	$-EINVAL, %eax
	jne	1b
4:	movl	%eax, %edx
	negl	%edx

3:	movl	%edx, %eax
	popq	%r9
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r9)
	retq

5:	movl	$ETIMEDOUT, %eax
	retq

	cfi_endproc
	.size	__lll_timedlock_wait,.-__lll_timedlock_wait
#endif


	.globl	__lll_unlock_wake_private
	.type	__lll_unlock_wake_private,@function
	.hidden	__lll_unlock_wake_private
	.align	16
__lll_unlock_wake_private:
	cfi_startproc
	pushq	%rsi
	cfi_adjust_cfa_offset(8)
	pushq	%rdx
	cfi_adjust_cfa_offset(8)
	cfi_offset(%rsi, -16)
	cfi_offset(%rdx, -24)

	movl	$0, (%rdi)
	LOAD_PRIVATE_FUTEX_WAKE (%esi)
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	syscall

	popq	%rdx
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rdx)
	popq	%rsi
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rsi)
	retq
	cfi_endproc
	.size	__lll_unlock_wake_private,.-__lll_unlock_wake_private

#if !IS_IN (libc)
	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
	.align	16
__lll_unlock_wake:
	cfi_startproc
	pushq	%rsi
	cfi_adjust_cfa_offset(8)
	pushq	%rdx
	cfi_adjust_cfa_offset(8)
	cfi_offset(%rsi, -16)
	cfi_offset(%rdx, -24)

	movl	$0, (%rdi)
	LOAD_FUTEX_WAKE (%esi)
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	syscall

	popq	%rdx
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rdx)
	popq	%rsi
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%rsi)
	retq
	cfi_endproc
	.size	__lll_unlock_wake,.-__lll_unlock_wake
#endif
