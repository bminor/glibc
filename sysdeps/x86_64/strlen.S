/* SSE2 version of strlen.
   Copyright (C) 2015 Free Software Foundation, Inc.
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



.text
ENTRY(strlen)
#ifdef AS_STRNLEN
	mov	%rsi, %r8
	xor	%edx, %edx
	test	%rsi, %rsi
	je	L(return_zero)
	cmp	$64,  %rsi
	jae	L(dont_set)
	bts	%rsi, %rdx
L(dont_set):
#endif
	pxor	%xmm0, %xmm0
	mov	%edi, %ecx
	and	$4095, %ecx
	cmp	$4032, %ecx
	ja	L(cross_page)
	movdqu	(%rdi), %xmm4
	pcmpeqb	%xmm0, %xmm4
	pmovmskb %xmm4, %ecx
#ifdef AS_STRNLEN
	or	%dx, %cx
#else
	test	%ecx, %ecx
#endif
	je	L(next48_bytes)
	bsf	%ecx, %eax
	ret

#ifdef AS_STRNLEN
L(return_zero):
	xor	%eax, %eax
	ret
L(return_noread):
	add	$64, %rax
	sub	%rdi, %rax
	ret
#endif
	.p2align 4
L(next48_bytes):
	movdqu	16(%rdi), %xmm1
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3
	pcmpeqb  %xmm0, %xmm1
	pcmpeqb  %xmm0, %xmm2
	pcmpeqb  %xmm0, %xmm3
#ifdef AS_STRNLEN
	pmovmskb %xmm1, %ecx
	sal	$16, %ecx
	or	%rcx, %rdx
#else
	pmovmskb %xmm1, %edx
	sal	$16, %edx
#endif
	pmovmskb %xmm2, %esi
	pmovmskb %xmm3, %ecx
	sal	$16, %ecx
	or	%esi, %ecx
	salq	$32, %rcx
	orq	%rcx, %rdx
	je	L(loop_init)
	bsfq	%rdx, %rax
	ret

	.p2align 4
L(cross_page):

	movq	%rdi, %rax
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
	pxor	%xmm3, %xmm3
#ifdef AS_STRNLEN
	mov	%rdx, %r9
#endif
	andq	$-64, %rax
	pcmpeqb	(%rax), %xmm0
	pcmpeqb	16(%rax), %xmm1
	pcmpeqb	32(%rax), %xmm2
	pcmpeqb	48(%rax), %xmm3
	pmovmskb %xmm0, %esi
	pxor	%xmm0, %xmm0
	pmovmskb %xmm1, %edx
	pmovmskb %xmm2, %r10d
	pmovmskb %xmm3, %ecx
	sal	$16, %edx
	sal	$16, %ecx
	or	%esi, %edx
	or	%r10, %rcx
	salq	$32, %rcx
	orq	%rcx, %rdx
	mov	%edi, %ecx
#ifdef AS_STRNLEN
	salq	%cl, %r9
	or	%r9, %rdx
#endif
	sarq	%cl, %rdx
	test	%rdx, %rdx
	je	L(loop_init)
	bsfq	%rdx, %rax
	ret
	.p2align 4
L(loop_init):
	movq	%rdi, %rax
	andq	$-64, %rax
#ifdef AS_STRNLEN
	add	%rdi, %r8
	sub	%rax, %r8
	cmp	$64, %r8
	je	L(return_noread)
#endif
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
#ifdef USE_AVX2
	vpxor	%xmm0, %xmm0, %xmm0
#endif
	.p2align 4
L(loop):
#ifdef USE_AVX2
	vmovdqa	64(%rax), %ymm1
	vpminub	96(%rax), %ymm1, %ymm2
	vpcmpeqb %ymm0, %ymm2, %ymm2
	vpmovmskb %ymm2, %edx
#else
	movdqa	64(%rax), %xmm5
	pminub	80(%rax), %xmm5
	pminub	96(%rax), %xmm5
	pminub	112(%rax), %xmm5
	pcmpeqb	%xmm0, %xmm5
	pmovmskb %xmm5, %edx
#endif

#ifdef AS_STRNLEN
	sub	$64, %r8
	testl	%edx, %edx
	jne	L(exit64)
	cmp	$64, %r8
	jbe	L(exit64_zero)
#else
	testl	%edx, %edx
	jne	L(exit64)
#endif

	subq	$-128, %rax
#ifdef USE_AVX2
	vmovdqa	(%rax), %ymm1
	vpminub	32(%rax), %ymm1, %ymm2
	vpcmpeqb %ymm0, %ymm2, %ymm2
	vpmovmskb %ymm2, %edx
#else
	movdqa	(%rax), %xmm5
	pminub	16(%rax), %xmm5
	pminub	32(%rax), %xmm5
	pminub	48(%rax), %xmm5
	pcmpeqb	%xmm0, %xmm5
	pmovmskb %xmm5, %edx
#endif
#ifdef AS_STRNLEN
	sub	$64, %r8
	testl	%edx, %edx
	jne	L(exit0)
	cmp	$64, %r8
	jbe	L(exit0_zero)
#else
	testl	%edx, %edx
	jne	L(exit0)
#endif
	jmp	L(loop)

#ifdef AS_STRNLEN
	.p2align 4
L(exit64_zero):
	addq	$64, %rax
L(exit0_zero):
	add	%r8, %rax
	sub	%rdi, %rax
	ret
#endif
	.p2align 4


L(exit64):
	addq	$64, %rax
L(exit0):
#ifdef USE_AVX2
	sal	$32, %rdx
#else
	sal	$48, %rdx
#endif
#ifdef AS_STRNLEN
	cmp	$64, %r8
	jae	L(dont_set2)
	bts	%r8, %rdx
	L(dont_set2):
#endif
#ifdef USE_AVX2
	subq    %rdi, %rax
	vpcmpeqb %ymm0, %ymm1, %ymm1
	vpmovmskb %ymm1, %ecx
	vzeroupper
	or	%rcx, %rdx
#else
	pcmpeqb	(%rax), %xmm0
	pcmpeqb	16(%rax), %xmm1
	pcmpeqb	32(%rax), %xmm2
	subq	%rdi, %rax
	pmovmskb %xmm0, %esi
	pmovmskb %xmm1, %ecx
	pmovmskb %xmm2, %r8d
	sal	$16, %ecx
	or	%esi, %ecx
	salq	$32, %r8
	orq	%r8, %rcx
	orq	%rcx, %rdx
#endif
	bsfq	%rdx, %rdx
	addq	%rdx, %rax
	ret
END(strlen)
libc_hidden_builtin_def (strlen)
