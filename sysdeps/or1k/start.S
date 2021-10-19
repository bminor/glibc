/* start, OpenRISC version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.

   sp   The stack contains the arguments and environment:
    0(sp)     argc
    4(sp)     argv[0]
    ...
    (4*argc)(sp)    NULL
    (4*(argc+1))(sp)  envp[0]
    ...
    NULL
 */

#define __ASSEMBLY__
#include <sysdep.h>
#include <entry.h>

ENTRY (ENTRY_POINT)

	/* Setup Arguments to the __libc_start_main function.  */

	/* Take values for argc and argv off the stack.
	   These will be passed as arguments two and three to main
	   and thus go in registers r4 and r5, respectively.  */
	l.lwz	r4, 0(r1)
	l.addi	r5, r1, 4

	/* Pass in rtld_fini from dl-start.S.  */
	l.or	r8, r3, r3

#ifdef PIC
	/* Obtain a pointer to .got in r16 */
	l.jal	0x8
	 l.movhi r16, gotpchi(_GLOBAL_OFFSET_TABLE_-4)
	l.ori	r16, r16, gotpclo(_GLOBAL_OFFSET_TABLE_+0)
	l.add	r16, r16, r9

	/* Pass in the the main symbol.  */
	l.lwz	r3, got(main)(r16)
#else
	/* Pass in the the main symbol.  */
	l.movhi r3, hi(main)
	l.ori	r3, r3, lo(main)
#endif
	/* Used to be init and fini.  */
	l.movhi	r6, 0x0
	l.movhi	r7, 0x0

	/* Push stack limit onto the stack.
	   This provides the highest stack address to user code (as stack grows
	   downwards.
	   This is the seventh argument to __libc_start_main and thus needs to
	   be passed on the stack.  */
	l.sw	-4(r1), r1

	/* Adjust stack to account for a total of 7 args (i.e. the last one is
	   on the stack.  */
	l.addi	r1, r1, -4

	/* Clear the frame pointer and link register since this is the
	   outermost frame.  */
	l.movhi	r2, 0x0
	l.movhi	r9, 0x0

	/* Let the libc call main and exit with its return code.  */
#ifdef PIC
	l.j	plt(__libc_start_main)
#else
	l.j	__libc_start_main
#endif
	 l.nop
END (ENTRY_POINT)

	/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
