/* Copy SRC to DEST returning the address of the terminating '\0' in DEST.
   For Intel 80x86, x>=3.
   Copyright (C) 1994-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper (drepper@gnu.ai.mit.edu).

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
#include "asm-syntax.h"

#ifdef USE_AS_STPCPY
# define PARMS		4		/* no space for saved regs */
# define STORE_ADDR	%eax
#else
# define PARMS		4+4		/* space for 1 saved reg */
# define STORE_ADDR	%ebx
# define STRCPY		strcpy
#endif

#define DEST		PARMS
#define SRC		DEST+4

	.text
ENTRY (STRCPY)

#ifndef USE_AS_STPCPY
	pushl %ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%ebx, 0)
#endif

	movl DEST(%esp), STORE_ADDR
	movl SRC(%esp), %ecx
	subl STORE_ADDR, %ecx	/* magic: reduce number of loop variants
				   to one using addressing mode */

#ifdef USE_AS_STPCPY
	/* Here we would like to write

	subl $4, %eax
	ALIGN (4)

	but the assembler is too smart and optimizes for the shortest
	form where the number only needs one byte.  But if we could
	have the long form we would not need the alignment.  */

	.byte 0x81, 0xe8	/* This is `subl $0x00000004, %eax' */
	.long 0x00000004
#else
	movl STORE_ADDR, %eax
	subl $4, STORE_ADDR
	.p2align 4
#endif

	/* Four times unfolded loop with only one loop counter.  This
	   is achieved by the use of index+base addressing mode.  As the
	   loop counter we use the destination address because this is
	   also the result.  */
L(1):	addl $4, STORE_ADDR		/* increment loop counter */

	movb (STORE_ADDR,%ecx), %dl	/* load current char */
	movb %dl, (STORE_ADDR)		/* and store it */
	testb %dl, %dl			/* was it NUL? */
	jz L(2)				/* yes, then exit */

	movb 1(STORE_ADDR,%ecx), %dl	/* load current char */
	movb %dl, 1(STORE_ADDR)		/* and store it */
	testb %dl, %dl			/* was it NUL? */
	jz L(3)				/* yes, then exit */

	movb 2(STORE_ADDR,%ecx), %dl	/* load current char */
	movb %dl, 2(STORE_ADDR)		/* and store it */
	testb %dl, %dl			/* was it NUL? */
	jz L(4)				/* yes, then exit */

	movb 3(STORE_ADDR,%ecx), %dl	/* load current char */
	movb %dl, 3(STORE_ADDR)		/* and store it */
	testb %dl, %dl			/* was it NUL? */
	jnz L(1)			/* no, then continue loop */

#ifdef USE_AS_STPCPY
	incl %eax			/* correct loop counter */
L(4):	incl %eax
L(3):	incl %eax
L(2):
#else
L(4):
L(3):
L(2):
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%ebx)
#endif

	ret
END (STRCPY)

#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
