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
#include <linux/auxvec.h>
#include <cheri_perms.h>

weak_extern (__rela_dyn_start)
weak_extern (__rela_dyn_end)

/* This is the canonical entry point, usually the first thing in the text
   segment.

   Note that the code in the .init section has already been run.
   This includes _init and _libc_init


   At this entry point, most registers' values are unspecified, except:

   x0/w0	Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		8(sp)			argv[0]
		...
		(8*argc)(sp)		NULL
		(8*(argc+1))(sp)	envp[0]
		...
					NULL
 */

	.text
ENTRY(_start)

	/* Create an initial frame with 0 LR and FP */
	cfi_undefined (c30)
	mov	c29, czr
	mov	c30, czr

	/* TODO: Use the presence of __rela_dyn_start.  */
	mov	c5, c0
	cbnz	x0, L(rtld_done)

	/* Initialize capabilities.  */
#ifndef SHARED
	/* The GNU Linker emits a R_MORELLO_RELATIVE relocation along with
	   __rela_dyn_start and __rela_dyn_end symbols around the relocation section.
	   r_offset contains the location, size and permissions:

		capability location		// offset 0 (64 bits)
		length				// offset 8 (56 bits)
		permissions			//           (8 bits)
		addend (capability offset)	// offset 16

	   according to the ABI document in:
	   https://github.com/ARM-software/abi-aa/blob/main/aaelf64-morello/aaelf64-morello.rst#dynamic-linking-with-morello

	The addend is the capability offset.

	   Note that this will not work with clang, since lld emits a __cap_relocs
	   struct that has a different layout.  If we need to support clang+lld, then
	   we need to copy the capability initialization steps from Bionic, in:
	   libc/arch-morello/bionic/__morello_init_static.S.  */

	adrp	c0, __rela_dyn_start
	add 	c0, c0, #:lo12:__rela_dyn_start
	adrp	c1, __rela_dyn_end
	add	c1, c1, #:lo12:__rela_dyn_end

1:
	/* No capabilities to initialize. Skip to stack adjustment.  */
	cmp	c0, c1
	b.eq	3f

	/* Load the address of capability.  */
	ldr	x2, [c0, #0]

	/* Construct capability location from DDC and r_offset.  */
	cvtd	c2, x2

	/* Base in x3.  */
	ldr	x3, [c2]

	/* Length + permissions in x5.  */
	ldr	x5, [c2, #8]

	/* Permissions in x6.  */
	lsr	x6, x5, #56

	/* Permissions encoded as:
	   4 = X (executable)
	   2 = W (read/write)
	   1 = R (read only)

	   Permissions to clear in x8 and x11.  */
	mov	x8, (CAP_PERM_STORE_CAP | CAP_PERM_EXECUTE)
	movk	x8, (CAP_PERM_STORE >> 16), lsl #16
	mov	x9, (CAP_PERM_EXECUTE)
	mov	x10, (CAP_PERM_STORE_CAP)
	movk	x10, (CAP_PERM_STORE >> 16), lsl #16
	mov	x11, (CAP_PERM_UNSEAL | CAP_PERM_SEAL | CAP_PERM_COMPARTMENT_ID)
	cmp	x6, #0x2
	csel	x8, x9, x8, eq
	cmp	x6, #0x4
	csel	x8, x10, x8, eq

	/* Length in x5.  */
	and	x5, x5, #0xffffffffffffff

	/* Offset in x7.  */
	ldr	x7, [c0, #16]

	/* Construct capability from DDC with value equal to base.  */
	cvtd	c3, x3

	/* Set bounds to [base ; base + length].  */
	scbndse c3, c3, x5

	/* Compute the capability value from addend.  */
	add	c3, c3, x7

	/* Clear permissions.  */
	clrperm c3, c3, x8
	clrperm	c3, c3, x11

	/* Seal capabilities, which provide execute permission, with MORELLO_RB */
	tst	x8, #CAP_PERM_EXECUTE
	b.ne	2f

	seal	c3, c3, rb

2:
	/* Store the constructed capability to the target location.  */
	str	c3, [c2]

	/* Move to the next __rela_dyn entry.  */
	add	c0, c0, #24
	b	1b

3:
#endif /* !SHARED */

	/* Setup rtld_fini in argument register */
	mov	c5, czr

L(rtld_done):

	/* Load argc and a pointer to argv */
	ldr	c1, [csp, #0]
	add	c2, csp, #16

	/* Setup stack limit in argument register */
	mov	c6, csp

#ifdef PIC
# ifdef SHARED
	adrp	c0, :got:main
	ldr	c0, [c0, #:got_lo12:main]
# else
	adrp	c0, __wrap_main
	add	c0, c0, :lo12:__wrap_main
# endif /* SHARED */
#else
	/* Set up the other arguments in registers */
	MOVL (0, main)
	cvtd	c0, x0
#endif /* PIC */

	mov	x3, #0		/* Used to be init.  */
	mov	x4, #0		/* Used to be fini.  */

	/* __libc_start_main (main, argc, argv, init, fini, rtld_fini,
			      stack_end) */

	/* Let the libc call main and exit with its return code.  */
	bl	__libc_start_main

	/* should never get here....*/
	bl	abort
END(_start)

#if defined PIC && !defined SHARED
	/* When main is not defined in the executable but in a shared library
	   then a wrapper is needed in crt1.o of the static-pie enabled libc,
	   because crt1.o and rcrt1.o share code and the later must avoid the
	   use of GOT relocations before __libc_start_main is called.  */
ENTRY(__wrap_main)
	b	main
END(__wrap_main)
#endif

	/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
