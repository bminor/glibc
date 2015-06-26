/* strcspn (str, ss) -- Return the length of the initial segment of STR
			which contains no characters from SS.
   Copyright (C) 1994-2015 Free Software Foundation, Inc.
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

#ifdef AS_STRSPN
# define AS_STRCSPN
# define MATCH_ALL $18
#else
# define MATCH_ALL $2
#endif

ENTRY(__strpbrk_sse42)
	movq	%rdi, %rax
	andl	$4095, %eax
	cmp	$4032, %eax
	ja	L(cross_page)
	movq	%rsi, %rax
	andl	$4095, %eax
	cmp	$4080, %eax
	ja	L(cross_page)
	movdqu	(%rsi), %xmm4
	movdqu	(%rdi), %xmm1
	movdqu	16(%rdi), %xmm5
	movdqu	32(%rdi), %xmm6
	movdqu	48(%rdi), %xmm7

L(back_from_crosspage):
	pxor	%xmm3, %xmm3
	pxor	%xmm2, %xmm2

	pcmpeqb	%xmm4, %xmm2
	pmovmskb %xmm2, %eax
	testl	%eax, %eax
	je	L(call)
	pcmpistri MATCH_ALL, %xmm1, %xmm4
	jc	L(rx0)
	je	L(ret0)
	pcmpistri MATCH_ALL, %xmm5, %xmm4
	jc	L(rx16)
	je	L(ret16)
	pcmpistri MATCH_ALL, %xmm6, %xmm4
	jc	L(rx32)
	je	L(ret32)
	pcmpistri MATCH_ALL, %xmm7, %xmm4
	jc	L(rx48)
	je	L(ret48)

	movq	%rdi, %rax
	andq	$-16, %rax
	addq	$16, %rax
	.p2align 4,,10
	.p2align 3
L(loop):
	pcmpistri MATCH_ALL, (%rax), %xmm4
	lea	16(%rax), %rax
	jc	L(rx_loop)
	jne	L(loop)
#ifdef AS_STRCSPN
	movdqa -16(%rax), %xmm1
	pcmpistri $58, %xmm1, %xmm1
	lea	-16(%rcx, %rax), %rax
	sub	%rdi, %rax
#else
	xor	%eax, %eax
#endif
	ret
L(rx_loop):
	lea	-16(%rcx, %rax), %rax
#ifdef AS_STRCSPN
	sub	%rdi, %rax
#endif
	ret
	.p2align 4,,10
	.p2align 3
#ifndef AS_STRCSPN
L(ret0):
L(ret16):
L(ret32):
L(ret48):
	xorl	%eax, %eax
	ret
#endif
L(call):
#ifdef AS_STRCSPN
# ifdef AS_STRSPN
	jmp	__strspn_sse2
# else
	jmp	__strcspn_sse2
# endif
#else
	jmp	__strpbrk_sse2
#endif
	.p2align 4,,10
	.p2align 3
#ifdef AS_STRCSPN
L(ret0):
	pcmpistri $58, %xmm1, %xmm1
L(rx0):
	lea	0(%rcx), %rax
#else
L(rx0):
	leaq	(%rdi,%rcx), %rax
#endif
	ret
#ifdef AS_STRCSPN
L(ret16):
	pcmpistri $58, %xmm5, %xmm5
L(rx16):
	lea	16(%rcx), %rax
#else
L(rx16):
	leaq	16(%rdi,%rcx), %rax
#endif
	ret
#ifdef AS_STRCSPN
L(ret32):
	pcmpistri $58, %xmm6, %xmm6
L(rx32):
	lea	32(%rcx), %rax
#else
L(rx32):
	leaq	32(%rdi,%rcx), %rax
#endif
	ret
#ifdef AS_STRCSPN
L(ret48):
	pcmpistri $58, %xmm7, %xmm7
L(rx48):
	lea	48(%rcx), %rax
#else
L(rx48):
	leaq	48(%rdi,%rcx), %rax
#endif
	ret

	.p2align 4,,10
	.p2align 3
L(cross_page):
	movzbl	(%rdi), %ecx
	xorl	%eax, %eax
	leaq	-80(%rsp), %r8
	testb	%cl, %cl
	je	L(sloop_end)
	leaq	-80(%rsp), %r8
	xorl	%edx, %edx
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
L(sloop):
	movb	%cl, (%r8,%rdx)
	movzbl	1(%rdi,%rdx), %ecx
	addl	$1, %eax
	testb	%cl, %cl
	je	L(sloop_end)
	addq	$1, %rdx
	cmpl	$64, %eax
	jne	L(sloop)
L(sloop_end):
	movzbl	(%rsi), %ecx
	cltq
	movb	$0, -80(%rsp,%rax)
	movdqu	(%r8), %xmm1
	movdqu	16(%r8), %xmm5
	movdqu	32(%r8), %xmm6
	movdqu	48(%r8), %xmm7

	xorl	%eax, %eax
	testb	%cl, %cl
	je	L(aloop_end)
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
L(aloop):
	movb	%cl, (%r8,%rdx)
	movzbl	1(%rsi,%rdx), %ecx
	addl	$1, %eax
	testb	%cl, %cl
	je	L(aloop_end)
	addq	$1, %rdx
	cmpl	$16, %eax
	jne	L(aloop)
L(aloop_end):
	cltq
	movb	$0, -80(%rsp,%rax)
	movdqu	(%r8), %xmm4
	jmp	L(back_from_crosspage)
END(__strpbrk_sse42)
