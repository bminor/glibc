/* Highly optimized version for ix86, x>=6.
   Copyright (C) 1999-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#define PARMS	4		/* no space for saved regs */
#define STR1	PARMS
#define STR2	STR1+4

        .text
ENTRY (strcmp)

	movl	STR1(%esp), %ecx
	movl	STR2(%esp), %edx
#ifdef __CHKP__
	bndldx	STR1(%esp,%ecx,1), %bnd0
	bndldx	STR2(%esp,%edx,1), %bnd1
	bndcl	(%ecx), %bnd0
	bndcl	(%edx), %bnd1
#endif

L(oop):
#ifdef __CHKP__
	bndcu	(%ecx), %bnd0
	bndcu	(%edx), %bnd1
#endif
	movb	(%ecx), %al
	cmpb	(%edx), %al
	jne	L(neq)
	incl	%ecx
	incl	%edx
	testb	%al, %al
	jnz	L(oop)

	xorl	%eax, %eax
	/* when strings are equal, pointers rest one beyond
	   the end of the NUL terminators.  */
	ret

L(neq):	movl	$1, %eax
	movl	$-1, %ecx
	cmovbl	%ecx, %eax

	ret
END (strcmp)
libc_hidden_builtin_def (strcmp)
