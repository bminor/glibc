/* Startup code compliant to the ELF i386 ABI.
   Copyright (C) 1995-2014 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.  The SVR4/i386 ABI (pages 3-31, 3-32) says that when the entry
   point runs, most registers' values are unspecified, except for:

   %edx		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %esp		The stack contains the arguments and environment:
		0(%esp)			argc
		4(%esp)			argv[0]
		...
		(4*argc)(%esp)		NULL
		(4*(argc+1))(%esp)	envp[0]
		...
					NULL
*/

	.text
	.globl _start
	.type _start,@function
_start:
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	xorl %ebp, %ebp
	call __google_init

	/* Extract the arguments as encoded on the stack and set up
	   the arguments for `main': argc, argv.  envp will be determined
	   later in __libc_start_main.  */
	popl %esi		/* Pop the argument count.  */
	movl %esp, %ecx		/* argv starts just at the current stack top.*/

	/* Before pushing the arguments align the stack to a 16-byte
	(SSE needs 16-byte alignment) boundary to avoid penalties from
	misaligned accesses.  Thanks to Edward Seidl <seidl@janed.com>
	for pointing this out.  */
	andl $0xfffffff0, %esp
	pushl %eax		/* Push garbage because we allocate
				   28 more bytes.  */

	/* Provide the highest stack address to the user code (for stacks
	   which grow downwards).  */
	pushl %esp

	pushl %edx		/* Push address of the shared library
				   termination function.  */

#ifdef SHARED
	/* Load PIC register.  */
	call 1f
	addl $_GLOBAL_OFFSET_TABLE_, %ebx

	/* Push address of our own entry points to .fini and .init.  */
	leal __libc_csu_fini@GOTOFF(%ebx), %eax
	pushl %eax
	leal __libc_csu_init@GOTOFF(%ebx), %eax
	pushl %eax

	pushl %ecx		/* Push second argument: argv.  */
	pushl %esi		/* Push first argument: argc.  */

	pushl main@GOT(%ebx)

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.    */
	call __libc_start_main@PLT
#else
	/* Push address of our own entry points to .fini and .init.  */
	pushl $__libc_csu_fini
	pushl $__libc_csu_init

	pushl %ecx		/* Push second argument: argv.  */
	pushl %esi		/* Push first argument: argc.  */

	pushl $main

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.    */
	call __libc_start_main
#endif

	hlt			/* Crash if somehow `exit' does return.  */

#ifdef SHARED
1:	movl	(%esp), %ebx
	ret
#endif

/*
  %esp		The stack contains the arguments and environment:
		0(%esp)				ra
		1*4(%esp)			argc
		2*4(%esp)			argv[0]
		...
		(argc+2)*4(%esp)		NULL
		(argc+3)*4(%esp)		envp[0]
		...
		(argc+2+env_count+1)*4(%esp)	NULL
                (argc+2+env_count+2)*4(%esp)	auxv[0]
		...
*/

__google_init:
#ifdef SHARED
	call 1b
	addl $_GLOBAL_OFFSET_TABLE_, %ebx
	leal __google_argc@GOTOFF(%ebx), %ecx
#else
	leal __google_argc, %ecx
#endif
	movl 4(%esp), %eax
	movl %eax, 0(%ecx)
	push %esp
	pop %eax
	addl $2*4, %eax
#ifdef SHARED
	leal __google_argv@GOTOFF(%ebx), %ecx
#else
	leal __google_argv, %ecx
#endif
	movl %eax, 0(%ecx)
/* The next loop could be omitted if we are to trust argc from the kernel.  */
1:	cmpl $0, 0(%eax)  /* while(argv[i++] != 0) skip; */
	je 2f
	addl $4, %eax
	jmp 1b
2:	addl $4, %eax
#ifdef SHARED
	leal __google_envp@GOTOFF(%ebx), %ecx
#else
	leal __google_envp, %ecx
#endif
	movl %eax, 0(%ecx)
3:	cmpl $0, 0(%eax)  /* while (envp[i++] != 0) skip; */
	je 4f
	addl $4, %eax
	jmp 3b
4:	addl $4, %eax
#ifdef SHARED
	leal __google_auxv@GOTOFF(%ebx), %ecx
#else
	leal __google_auxv, %ecx
#endif
	movl %eax, 0(%ecx)
	ret

/* To fulfill the System V/i386 ABI we need this symbol.  Yuck, it's so
   meaningless since we don't support machines < 80386.  */
	.section .rodata
	.globl _fp_hw
_fp_hw:	.long 3
	.size _fp_hw, 4
	.type _fp_hw,@object

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

/* The {argc,argv,envp} variables are not exposed to keep the interface narrow
   (we don't need anything except auxv right now). If/when they are needed,
   simply mark them with ".globl".  */
__google_argc: .long 0
__google_argv: .long 0
__google_envp: .long 0
	.globl __google_auxv
__google_auxv: .long 0


