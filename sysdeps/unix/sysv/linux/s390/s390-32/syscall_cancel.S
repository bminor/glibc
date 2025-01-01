/* Cancellable syscall wrapper.  Linux/s390 version.
   Copyright (C) 2023-2025 Free Software Foundation, Inc.
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
	stm	%r6,%r7,24(%r15)
	cfi_offset (%r6, -72)
	cfi_offset (%r7, -68)

	.globl __syscall_cancel_arch_start
__syscall_cancel_arch_start:
	/* if (*cancelhandling & CANCELED_BITMASK)
	     __syscall_do_cancel()  */
	tm	3(%r2),TCB_CANCELED_BITMASK
	jne	1f

	/* Issue a 6 argument syscall, the nr [%r1] being the syscall
	   number.  */
	lr	%r1,%r3
	lr	%r2,%r4
	lr	%r3,%r5
	lr	%r4,%r6
	lm	%r5,%r7,96(%r15)
	svc	0

	.globl __syscall_cancel_arch_end
__syscall_cancel_arch_end:
	lm	%r6,%r7,24(%r15)
	cfi_remember_state
	cfi_restore (%r7)
	cfi_restore (%r6)
	br	%r14
1:
	cfi_restore_state
	jg	__syscall_do_cancel
END (__syscall_cancel_arch)
