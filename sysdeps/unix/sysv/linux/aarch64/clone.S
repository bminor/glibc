/* Copyright (C) 1996-2022 Free Software Foundation, Inc.

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

#ifdef __CHERI_PURE_CAPABILITY__
# define arg0	c0
# define arg1	c1
# define arg2	c2
# define arg3	c3
# define arg4	c4
# define arg5	c5
# define arg6	c6
# define tmp0	c10
# define tmp1	c11
# define tmp2	c12
#else
# define arg0	x0
# define arg1	x1
# define arg2	x2
# define arg3	x3
# define arg4	x4
# define arg5	x5
# define arg6	x6
# define tmp0	x10
# define tmp1	x11
# define tmp2	x12
#endif

/* int clone(int (*fn)(void *arg),            arg0
	     void *child_stack,               arg1
	     int flags,                       arg2
	     void *arg,                       arg3
	     pid_t *ptid,                     arg4
	     struct user_desc *tls,           arg5
             pid_t *ctid);                    arg6
 */
        .text
ENTRY(__clone)
#ifndef __CHERI_PURE_CAPABILITY__
	PTR_ARG (0)
	PTR_ARG (1)
	PTR_ARG (3)
	PTR_ARG (4)
	PTR_ARG (5)
	PTR_ARG (6)
#else
	cbz	x9, L(skip_args)
	ldr	c4, [c9]
	ldr	c5, [c9,16]
	ldr	c6, [c9,32]
L(skip_args):
#endif
	/* Save args for the child.  */
	mov	tmp0, arg0
	mov	tmp1, arg2
	mov	tmp2, arg3

	/* Sanity check args.  */
	mov	x0, #-EINVAL
	cbz	x10, .Lsyscall_error
	/* Align sp.  */
#ifndef __CHERI_PURE_CAPABILITY__
	and	x1, x1, -16
#else
	alignd	c1, c1, 4
#endif
	cbz	x1, .Lsyscall_error

	/* Do the system call.  */
	/* X0:flags, x1:newsp, x2:parenttidptr, x3:newtls, x4:childtid.  */
	mov	arg0, arg2                  /* flags  */
	/* New sp is already in x1.  */
	mov	arg2, arg4			/* ptid  */
	mov	arg3, arg5			/* tls  */
	mov	arg4, arg6			/* ctid  */
	mov	x8, #SYS_ify(clone)
	svc	0x0

	cmp	x0, #0
	beq	thread_start
	blt	.Lsyscall_error
	RET
PSEUDO_END (__clone)

	.align 4
	.type thread_start, %function
thread_start:
	cfi_startproc
	cfi_undefined (x30)
	mov	x29, 0

	/* Pick the function arg and execute.  */
	mov	arg0, tmp2
	blr	tmp0

	/* We are done, pass the return value through x0.  */
	mov	x8, #SYS_ify(exit)
	svc	0x0
	cfi_endproc
	.size thread_start, .-thread_start

libc_hidden_def (__clone)
weak_alias (__clone, clone)
