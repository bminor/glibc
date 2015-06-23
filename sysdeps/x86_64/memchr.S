/* Copyright (C) 2015 Free Software Foundation, Inc.
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

/* fast SSE2 version with using 64 byte loop */

	.text
ENTRY(memchr)
	movd	%esi, %xmm2
	testq	%rdx, %rdx
	punpcklbw	%xmm2, %xmm2
	punpcklwd	%xmm2, %xmm2
	pshufd	$0, %xmm2, %xmm2
	je	L(return_null)
	movl	%edi, %eax
	andl	$4095, %eax
	cmpl	$4032, %eax
	jg	L(cross_page)
	movdqu	(%rdi), %xmm1
	pcmpeqb	%xmm2, %xmm1
	pmovmskb	%xmm1, %eax
	test	%eax, %eax
	je	L(next_48_bytes)
	bsf	%eax, %eax
	cmpq	%rax, %rdx
	jbe	L(return_null)
	addq	%rdi, %rax
		ret
.p2align	4,,10
.p2align	3
L(next_48_bytes):
	movdqu	16(%rdi), %xmm1
	movdqu	32(%rdi), %xmm3
	pcmpeqb	%xmm2, %xmm1
	pcmpeqb	%xmm2, %xmm3
	movdqu	48(%rdi), %xmm4
	pmovmskb	%xmm1, %esi
	pmovmskb	%xmm3, %ecx
	pcmpeqb	%xmm2, %xmm4
	pmovmskb	%xmm4, %eax
	salq	$32, %rcx
	sal	$16, %esi
	orq	%rsi, %rcx
	salq	$48, %rax
	orq	%rcx, %rax
	je	L(prepare_loop)
L(return):
	bsf	%rax, %rax
	cmpq	%rax, %rdx
	jbe	L(return_null)
	addq	%rdi, %rax
		ret

.p2align	4,,10
.p2align	3
L(return_null):
	xorl	%eax, %eax
		ret
.p2align	4,,10
.p2align	4
L(prepare_loop):
	movq	%rdi, %rcx
	andq	$-64, %rcx
	subq	%rcx, %rdi
	leaq	(%rdx, %rdi), %rsi
.p2align	4,,10
.p2align	3
L(loop):
	subq	$64, %rsi
	jbe	L(return_null)
		
	movdqa	64(%rcx), %xmm0
	movdqa	80(%rcx), %xmm1
	movdqa	96(%rcx), %xmm3
	movdqa	112(%rcx), %xmm4

	pcmpeqb	%xmm2, %xmm0
	pcmpeqb	%xmm2, %xmm1
	pcmpeqb	%xmm2, %xmm3
	pcmpeqb	%xmm2, %xmm4

	pmaxub	%xmm0, %xmm1
	pmaxub	%xmm1, %xmm3
	pmaxub	%xmm3, %xmm4
	addq	$64, %rcx
	pmovmskb %xmm4, %edx
	testl	%edx, %edx
	je	L(loop)
	pmovmskb %xmm3, %r8d
	pmovmskb %xmm1, %edi
	salq	$48, %rdx
	pmovmskb %xmm0, %eax
	salq	$32, %r8
	sal	$16, %edi
	or	%edi, %eax
	orq	%r8, %rax
	orq	%rax, %rdx
	bsfq	%rdx, %rax
	cmp	%rax, %rsi
	jbe	L(return_null)
	addq	%rcx, %rax
	ret

.p2align	4,,10
.p2align	3
L(cross_page):
	movq	%rdi, %rsi
	andq	$-64, %rsi
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm2, %xmm1
	pmovmskb %xmm1, %ecx
	movdqa	16(%rsi), %xmm1
	pcmpeqb	%xmm2, %xmm1
	pmovmskb %xmm1, %eax
	movdqa	32(%rsi), %xmm1
	pcmpeqb	%xmm2, %xmm1
	sal	$16, %eax
	movdqa	%xmm2,	%xmm0
	pcmpeqb	48(%rsi), %xmm0
	pmovmskb %xmm1, %r8d
	pmovmskb %xmm0, %r9d
	salq	$32, %r8
	salq	$48, %r9
	or	%ecx, %eax
	orq	%r9, %rax
	orq	%r8, %rax
	movq	%rdi, %rcx
	subq	%rsi, %rcx
	shrq	%cl, %rax
	testq	%rax, %rax
	jne	L(return)
	jmp	L(prepare_loop)
END(memchr)

strong_alias (memchr, __memchr)

libc_hidden_builtin_def(memchr)
