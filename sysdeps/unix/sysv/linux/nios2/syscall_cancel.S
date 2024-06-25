/* Cancellable syscall wrapper.  Linux/nios2 version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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
#include <descr-const.h>

/* long int __syscall_cancel_arch (int *cancelhandling,
				   __syscall_arg_t nr,
				   __syscall_arg_t arg1,
				   __syscall_arg_t arg2,
				   __syscall_arg_t arg3,
				   __syscall_arg_t arg4,
				   __syscall_arg_t arg5,
				   __syscall_arg_t arg6)  */

ENTRY (__syscall_cancel_arch)
#ifdef SHARED
	addi	sp, sp, -8
	stw	r22, 0(sp)
	nextpc	r22
1:
	movhi	r8, %hiadj(_gp_got - 1b)
	addi	r8, r8, %lo(_gp_got - 1b)
	stw	ra, 4(sp)
	add	r22, r22, r8
#else
	addi	sp, sp, -4
	cfi_def_cfa_offset (4)
	stw	ra, 0(sp)
	cfi_offset (31, -4)
#endif

	.globl __syscall_cancel_arch_start
__syscall_cancel_arch_start:
	ldw	r3, 0(r4)
	andi	r3, r3, TCB_CANCELED_BITMASK
	bne	r3, zero, 3f
	mov	r10, r6
	mov	r2, r5
#ifdef SHARED
# define STACK_ADJ 4
#else
# define STACK_ADJ 0
#endif
	ldw	r9, (16 + STACK_ADJ)(sp)
	mov	r5, r7
	ldw	r8, (12 + STACK_ADJ)(sp)
	ldw	r7, (8 + STACK_ADJ)(sp)
	ldw	r6, (4 + STACK_ADJ)(sp)
	mov	r4, r10
	trap

	.globl __syscall_cancel_arch_end
__syscall_cancel_arch_end:
	beq	r7, zero, 2f
	sub	r2, zero, r2
2:
#ifdef SHARED
	ldw	ra, 4(sp)
	ldw	r22, 0(sp)
	addi	sp, sp, 8
#else
	ldw	ra, (0 + STACK_ADJ)(sp)
	cfi_remember_state
	cfi_restore (31)
	addi	sp, sp, 4
	cfi_def_cfa_offset (0)
#endif
	ret

3:
#ifdef SHARED
	ldw	r2, %call(__syscall_do_cancel)(r22)
	callr	r2
#else
	cfi_restore_state
	call	__syscall_do_cancel
#endif

END (__syscall_cancel_arch)
