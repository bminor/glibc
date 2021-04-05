/* Copyright (C) 2022 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <jmpbuf-offsets.h>
#include <stap-probe.h>

        /* Keep traditional entry points in with sigsetjmp(). */
ENTRY (setjmp)
	mov	x1, #1
	b	1f
END (setjmp)

ENTRY (_setjmp)
	mov	x1, #0
	b	1f
END (_setjmp)
libc_hidden_def (_setjmp)

ENTRY (__sigsetjmp)
1:
	stp	c19, c20, [c0, #JB_X19<<4]
	stp	c21, c22, [c0, #JB_X21<<4]
	stp	c23, c24, [c0, #JB_X23<<4]
	stp	c25, c26, [c0, #JB_X25<<4]
	stp	c27, c28, [c0, #JB_X27<<4]
	stp	c29, c30, [c0, #JB_X29<<4]

	/* setjmp probe takes 3 arguments, address of jump buffer
	   first argument (8@x0), return value second argument (-4@x1),
	   and target address (8@x30), respectively.  */
	LIBC_PROBE (setjmp, 3, 8@c0, -4@c1, 8@c30)
	stp	 q8,  q9, [c0, #JB_D8<<4]
	stp	q10, q11, [c0, #JB_D10<<4]
	stp	q12, q13, [c0, #JB_D12<<4]
	stp	q14, q15, [c0, #JB_D14<<4]

	mov	c2,  csp
	str	c2,  [c0, #JB_SP<<4]

#if IS_IN (rtld)
	/* In ld.so we never save the signal mask */
	mov	w0, #0
	RET
#else
	b	C_SYMBOL_NAME(__sigjmp_save)
#endif
END (__sigsetjmp)
hidden_def (__sigsetjmp)
