/* Startup code for ARC.
   Copyright (C) 1995-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


#define __ASSEMBLY__ 1
#include <entry.h>
#ifndef ENTRY_POINT
# error ENTRY_POINT needs to be defined for ARC
#endif

/* When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL.  */

	.text
	.align 4
	.global __start
	.type __start,@function
__start:
	mov	fp, 0
	ld_s	r1, [sp]	; argc

	mov_s	r5, r0		; rltd_fini
	add_s	r2, sp, 4	; argv
	and	sp, sp, -8
	mov	r6, sp

	/* __libc_start_main (main, argc, argv, init, fini, rtld_fini, stack_end).  */

#ifdef SHARED
	ld	r0, [pcl, @main@gotpc]
	ld	r3, [pcl, @__libc_csu_init@gotpc]
	ld	r4, [pcl, @__libc_csu_fini@gotpc]
	bl	__libc_start_main@plt
#else
	mov_s	r0, main
	mov_s	r3, __libc_csu_init
	mov	r4, __libc_csu_fini
	bl	__libc_start_main
#endif

	/* Should never get here.  */
	flag    1
	.size  __start,.-__start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
