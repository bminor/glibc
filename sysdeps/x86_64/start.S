/* Startup code compliant to the ELF x86-64 ABI.
   Copyright (C) 2001-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2001.

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

   %rdx		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   %rsp		The stack contains the arguments and environment:
		0(%rsp)				argc
		LP_SIZE(%rsp)			argv[0]
		...
		(LP_SIZE*argc)(%rsp)		NULL
		(LP_SIZE*(argc+1))(%rsp)	envp[0]
		...
						NULL
*/

#include <sysdep.h>

	.text
	.globl _start
	.type _start,@function
_start:
	cfi_startproc
	/* Clearing frame pointer is insufficient, use CFI.  */
	cfi_undefined (rip)
	/* Clear the frame pointer.  The ABI suggests this be done, to mark
	   the outermost frame obviously.  */
	xorl %ebp, %ebp
	call __google_init

	/* Extract the arguments as encoded on the stack and set up
	   the arguments for __libc_start_main (int (*main) (int, char **, char **),
		   int argc, char *argv,
		   void (*init) (void), void (*fini) (void),
		   void (*rtld_fini) (void), void *stack_end).
	   The arguments are passed via registers and on the stack:
	main:		%rdi
	argc:		%rsi
	argv:		%rdx
	init:		%rcx
	fini:		%r8
	rtld_fini:	%r9
	stack_end:	stack.	*/

	mov %RDX_LP, %R9_LP	/* Address of the shared library termination
				   function.  */
#ifdef __ILP32__
	mov (%rsp), %esi	/* Simulate popping 4-byte argument count.  */
	add $4, %esp
#else
	popq %rsi		/* Pop the argument count.  */
#endif
	/* argv starts just at the current stack top.  */
	mov %RSP_LP, %RDX_LP
	/* Align the stack to a 16 byte boundary to follow the ABI.  */
	and  $~15, %RSP_LP

	/* Push garbage because we push 8 more bytes.  */
	pushq %rax

	/* Provide the highest stack address to the user code (for stacks
	   which grow downwards).  */
	pushq %rsp

#ifdef SHARED
	/* Pass address of our own entry points to .fini and .init.  */
	mov __libc_csu_fini@GOTPCREL(%rip), %R8_LP
	mov __libc_csu_init@GOTPCREL(%rip), %RCX_LP

	mov main@GOTPCREL(%rip), %RDI_LP

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.	  */
	call __libc_start_main@PLT
#else
	/* Pass address of our own entry points to .fini and .init.  */
	mov $__libc_csu_fini, %R8_LP
	mov $__libc_csu_init, %RCX_LP

	mov $main, %RDI_LP

	/* Call the user's main function, and exit with its value.
	   But let the libc call main.	  */
	call __libc_start_main
#endif

	hlt			/* Crash if somehow `exit' does return.	 */
	cfi_endproc

/*
   %rsp		The stack contains the arguments and environment:
		0(%rsp)				ra
		1*8(%rsp)			argc
		2*8(%rsp)			argv[0]
		...
		(argc+2)*8(%rsp)		NULL
		(argc+3)*8(%rsp)		envp[0]
		...
		(argc+2+env_count+1)*8(%rsp)	NULL
                (argc+2+env_count+2)*8(%rsp)	auxv[0]
		...
*/
__google_init:
	leaq __google_argc(%rip), %rbx  /* %rbx = &__google_argc  */
	movq 8(%rsp), %rax
	movq %rax, 0(%rbx)
	push %rsp
	pop %rax
	addq $2*8, %rax
	leaq __google_argv(%rip), %rbx
	movq %rax, 0(%rbx)
/* The next loop could be omitted if we are to trust argc from the kernel.  */
1:	cmpq $0, 0(%rax)  /* while(argv[i++] != 0) skip; */
	je 2f
	addq $8, %rax
	jmp 1b
2:	addq $8, %rax
	leaq __google_envp(%rip), %rbx
	movq %rax, 0(%rbx)
3:	cmpq $0, 0(%rax)  /* while (envp[i++] != 0) skip; */
	je 4f
	addq $8, %rax
	jmp 3b
4:	addq $8, %rax
	leaq __google_auxv(%rip), %rbx
	movq %rax, 0(%rbx)
	ret


/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

	.p2align 3
/* The {argc,argv,envp} variables are not exposed to keep the interface narrow
   (we don't need anything except auxv right now). If/when they are needed,
   simply mark them with ".globl".  */
__google_argc: .quad 0
__google_argv: .quad 0
__google_envp: .quad 0
	.globl __google_auxv
__google_auxv: .quad 0
