/* memcmp with SSE2
   Copyright (C) 2009-2015 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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

#ifndef MEMCMP
# define MEMCMP memcmp
#endif

	.text
ENTRY (MEMCMP)
	testq	%rdx, %rdx
	je	L(return_zero)
#ifdef AS_WMEMCMP
	shl	$2, %rdx
#endif
	pxor	%xmm4, %xmm4
	movl	%edi, %eax
	andl	$4095, %eax
	cmpl	$4032, %eax
	ja	L(cross_page_start)
L(handle_end):
	movl	%esi, %eax
	andl	$4095, %eax
	cmpl	$4032, %eax
	ja	L(cross_page_start)
L(back_header):
	xor	%ecx, %ecx
	bts	%rdx, %rcx
	sub	$1, %rcx
	movdqu	(%rdi), %xmm0
	movdqu	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm4, %xmm0
	pmovmskb %xmm0, %eax
	and	%ecx, %eax
	jne	L(different)
	cmpq	$16, %rdx
	ja	L(next)
	ret
L(next):
	pmovmskb %xmm0, %r8d
	movdqu	16(%rdi), %xmm2
	movdqu	16(%rsi), %xmm6
	movdqu	32(%rdi), %xmm1
	pcmpeqb	%xmm6, %xmm2
	movdqu	32(%rsi), %xmm5
	pcmpeqb	%xmm4, %xmm2
	pcmpeqb	%xmm5, %xmm1
	movdqu	48(%rdi), %xmm7
	pmovmskb %xmm2, %eax
	movdqu	48(%rsi), %xmm3
	pcmpeqb	%xmm4, %xmm1
	pmovmskb %xmm1, %r9d
	sal	$16, %eax
	pcmpeqb	%xmm3, %xmm7
	salq	$32, %r9
	pcmpeqb	%xmm4, %xmm7
	orq	%r9, %rax
	orq	%r8, %rax
	pmovmskb %xmm7, %r8d
	salq	$48, %r8
	orq	%r8, %rax
	movq	%rax, %r8
	andq	%rcx, %rax
	jne	L(different)
	cmpq	$64, %rdx
	jb	L(return_zero)
	movq	%r8, %rax
	testq	%rax, %rax
	jne	L(different)
L(align_loop):
	leaq	64(%rdi), %rax
	andq	$-64, %rax
	subq	%rdi, %rax
	subq	%rax, %rdx
	addq	%rax, %rdi
	addq	%rax, %rsi
	cmpq	$64, %rdx
	ja	L(loop_start)
	testq	%rdx, %rdx
	jne	L(handle_end)
	xorl	%eax, %eax
	ret

	.p2align 4
L(different):
	bsfq	%rax, %rdx
#ifdef AS_WMEMCMP
	and	$-4, %rdx
	mov	(%rdi,%rdx), %eax
	mov	(%rsi,%rdx), %edx
	subl	%edx, %eax
	jg	L(ret1)
	jl	L(ret_neg_1)
	ret
L(ret1):
	mov $1, %eax
	ret
L(ret_neg_1):
	mov $-1, %eax
	ret
#else
	movzbl	(%rdi,%rdx), %eax
	movzbl	(%rsi,%rdx), %edx
	subl	%edx, %eax
	ret
#endif
L(return_zero):
	xor	%eax, %eax
	ret
	.p2align 4
L(loop):
	subq	$64, %rdx
	addq	$64, %rdi
	addq	$64, %rsi
	cmpq	$64, %rdx
	jbe	L(less_64_bytes)
L(loop_start):
	movdqu	(%rsi), %xmm0
	movdqu	16(%rsi), %xmm1
	pcmpeqb	(%rdi), %xmm0
	movdqu	32(%rsi), %xmm2
	pcmpeqb	16(%rdi), %xmm1
	movdqu	48(%rsi), %xmm3
	pcmpeqb	32(%rdi), %xmm2
	pcmpeqb	48(%rdi), %xmm3
	pminub	%xmm0, %xmm3
	pminub	%xmm1, %xmm3
	pminub	%xmm2, %xmm3
	pcmpeqb	%xmm4, %xmm3
	pmovmskb %xmm3, %eax
	testl	%eax, %eax
	je	L(loop)
	shl	$48, %rax
	pcmpeqb	%xmm4, %xmm0
	pcmpeqb	%xmm4, %xmm1
	pcmpeqb	%xmm4, %xmm2
	pmovmskb %xmm0, %r8
	pmovmskb %xmm1, %rcx
	pmovmskb %xmm2, %r9
	shl	$16, %ecx
	shl	$32, %r9
	or	%r8, %rax
	or	%r9, %rax
	or	%rcx, %rax
	jmp	L(different)

	.p2align 4
L(less_64_bytes):
	testq	%rdx, %rdx
	jne	L(handle_end)
	xorl	%eax, %eax
	ret


	.p2align 4
L(cross_page_start):
	cmp	$64, %rdx
	ja	L(back_header)

	.p2align 4
L(cross_page):
	test	%edx, %edx
	je	L(return_zero)
#ifdef AS_WMEMCMP
	mov	(%rdi), %eax
	mov	(%rsi), %ecx
	subl	%ecx, %eax
	jg	L(ret1)
	jl	L(ret_neg_1)
#else
	movzbl	(%rdi), %eax
	movzbl	(%rsi), %ecx
	subl	%ecx, %eax
	jne	L(return)
	cmp	$1, %edx
	je	L(return)
	movzbl	1(%rdi), %eax
	movzbl	1(%rsi), %ecx
	subl	%ecx, %eax
	jne	L(return)
	cmp	$2, %edx
	je	L(return)
	movzbl	2(%rdi), %eax
	movzbl	2(%rsi), %ecx
	subl	%ecx, %eax
	jne	L(return)
	cmp	$3, %edx
	je	L(return)
	movzbl	3(%rdi), %eax
	movzbl	3(%rsi), %ecx
	subl	%ecx, %eax
	jne	L(return)
#endif
	sub	$4, %edx
	add	$4, %rdi
	add	$4, %rsi
	jmp	L(cross_page)
L(return):
	ret
END(MEMCMP)

#undef	bcmp
weak_alias (memcmp, bcmp)
libc_hidden_builtin_def (memcmp)
