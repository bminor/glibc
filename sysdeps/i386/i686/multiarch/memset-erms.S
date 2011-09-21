/* memset with Enhanced REP MOVSB/STOSB
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

#ifndef NOT_IN_libc

#include <sysdep.h>

#define CFI_PUSH(REG)						\
  cfi_adjust_cfa_offset (4);					\
  cfi_rel_offset (REG, 0)

#define CFI_POP(REG)						\
  cfi_adjust_cfa_offset (-4);					\
  cfi_restore (REG)

#define PUSH(REG)	pushl REG; CFI_PUSH (REG)
#define POP(REG)	popl REG; CFI_POP (REG)

#define STR1  8
#ifdef USE_AS_BZERO
#define N     STR1+4
#else
#define STR2  STR1+4
#define N     STR2+4
#endif

	.section .text.erms,"ax",@progbits
#if defined SHARED && !defined NOT_IN_libc && !defined USE_AS_BZERO
ENTRY (__memset_chk_erms)
	movl	12(%esp), %eax
	cmpl	%eax, 16(%esp)
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memset_chk_erms)
#endif
ENTRY (__memset_erms)
	PUSH    (%edi)
	movl	N(%esp), %ecx
	movl	STR1(%esp), %edi
#ifdef USE_AS_BZERO
	xor	%eax, %eax
#else
	movzbl	STR2(%esp), %eax
	mov	%edi, %edx
#endif
	rep	stosb
#ifndef USE_AS_BZERO
	mov	%edx, %eax
#endif
	POP     (%edi)
	ret
END (__memset_erms)

#endif
