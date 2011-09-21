/* memcpy with Enhanced REP MOVSB/STOSB
   Copyright (C) 2014 Free Software Foundation, Inc.
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

#include <sysdep.h>

#if !defined NOT_IN_libc \
    && (defined SHARED \
	|| defined USE_AS_MEMMOVE \
	|| !defined USE_MULTIARCH)

#include "asm-syntax.h"

#ifndef MEMCPY
# define MEMCPY		__memcpy_erms
# define MEMCPY_CHK	__memcpy_chk_erms
#endif

#ifdef USE_AS_BCOPY
# define STR2		12
# define STR1		STR2+4
# define N     		STR1+4
#else
# define STR1		12
# define STR2		STR1+4
# define N     		STR2+4
#endif

#define CFI_PUSH(REG)						\
  cfi_adjust_cfa_offset (4);					\
  cfi_rel_offset (REG, 0)

#define CFI_POP(REG)						\
  cfi_adjust_cfa_offset (-4);					\
  cfi_restore (REG)

#define PUSH(REG)	pushl REG; CFI_PUSH (REG)
#define POP(REG)	popl REG; CFI_POP (REG)

	.section .text.erms,"ax",@progbits
#if !defined USE_AS_BCOPY
ENTRY (MEMCPY_CHK)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (MEMCPY_CHK)
#endif
ENTRY (MEMCPY)
	PUSH	(%esi)
	PUSH	(%edi)
	movl	N(%esp), %ecx
	movl	STR1(%esp), %edi
	movl	STR2(%esp), %esi
	mov	%edi, %eax
#ifdef USE_AS_MEMPCPY
	add	%ecx, %eax
#endif

#ifdef USE_AS_MEMMOVE
	cmp	%esi, %edi
	jbe	L(copy_forward)
	lea	(%esi,%ecx), %edx
	cmp	%edx, %edi
	jb	L(copy_backward)
L(copy_forward):
#endif

	rep	movsb
	POP	(%edi)
	POP	(%esi)
	ret

#ifdef USE_AS_MEMMOVE
L(copy_backward):
	lea	-1(%edi,%ecx), %edi
	lea	-1(%esi,%ecx), %esi
	std
	rep	movsb
	cld
	POP	(%edi)
	POP	(%esi)
	ret
#endif

END (MEMCPY)

#endif
