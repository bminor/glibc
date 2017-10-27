/* strcpy/strcpy optimized with stosb.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#define PARMS	4+8	/* space for 2 saved regs */
#define DEST	PARMS
#define SRC	DEST+4

#ifndef USE_AS_STPCPY
# define STRCPY __strcpy_stosb
#endif

	.text
ENTRY (STRCPY)
	pushl	%edi
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%edi, 4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%esi, 0)

	movl	DEST(%esp), %edi
	movl	SRC(%esp), %esi

#ifndef USE_AS_STPCPY
	movl	%edi, %edx
#endif

L(repeat):
	lodsb
	stosb
	testb	%al,%al
	jne	L(repeat)

#ifdef USE_AS_STPCPY
	leal	-1(%edi), %eax
#else
	movl	%edx, %eax
#endif

	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%esi)
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%edi)

	ret
END (STRCPY)
