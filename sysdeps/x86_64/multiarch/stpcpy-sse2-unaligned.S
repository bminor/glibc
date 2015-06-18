/* stpcpy with SSE2 and unaligned load
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
#ifndef STPCPY
# define STPCPY __stpcpy_sse2_unaligned
#endif

ENTRY(STPCPY)
	mov	%esi, %edx
#ifdef AS_STRCPY
	movq    %rdi, %rax
#endif
	pxor	%xmm4, %xmm4
	pxor	%xmm5, %xmm5
	andl	$4095, %edx
	cmp	$3968, %edx
	ja	L(cross_page)

	movdqu	(%rsi), %xmm0
	pcmpeqb	%xmm0, %xmm4
	pmovmskb %xmm4, %edx
	testl	%edx, %edx
	je	L(more16bytes)
	bsf	%edx, %ecx
#ifndef AS_STRCPY
	lea	(%rdi, %rcx), %rax
#endif
	cmp	$7, %ecx
	movq	(%rsi), %rdx
	jb	L(less_8_bytesb)
L(8bytes_from_cross):
	movq	-7(%rsi, %rcx), %rsi
	movq	%rdx, (%rdi)
#ifdef AS_STRCPY
	movq    %rsi, -7(%rdi, %rcx)
#else
	movq	%rsi, -7(%rax)
#endif
	ret

	.p2align 4
L(less_8_bytesb):
	cmp	$2, %ecx
	jbe	L(less_4_bytes)
L(4bytes_from_cross):
	mov	-3(%rsi, %rcx), %esi
	mov	%edx, (%rdi)
#ifdef AS_STRCPY
        mov     %esi, -3(%rdi, %rcx)
#else
	mov	%esi, -3(%rax)
#endif
	ret

.p2align 4
 L(less_4_bytes):
 /*
  Test branch vs this branchless that works for i 0,1,2
   d[i] = 0;
   d[i/2] = s[1];
   d[0] = s[0];
  */
#ifdef AS_STRCPY
 movb $0, (%rdi, %rcx)
#endif

 shr $1, %ecx
 mov %edx, %esi
 shr $8, %edx
 movb %dl, (%rdi, %rcx)
#ifndef AS_STRCPY
 movb $0, (%rax)
#endif
 movb %sil, (%rdi)
 ret





	.p2align 4
L(more16bytes):
	pxor	%xmm6, %xmm6
	movdqu	16(%rsi), %xmm1
	pxor	%xmm7, %xmm7
	pcmpeqb	%xmm1, %xmm5
	pmovmskb %xmm5, %edx
	testl	%edx, %edx
	je	L(more32bytes)
	bsf	%edx, %edx
#ifdef AS_STRCPY
        movdqu  1(%rsi, %rdx), %xmm1
        movdqu  %xmm0, (%rdi)
	movdqu  %xmm1, 1(%rdi, %rdx)
#else
	lea	16(%rdi, %rdx), %rax
	movdqu	1(%rsi, %rdx), %xmm1
	movdqu	%xmm0, (%rdi)
	movdqu	%xmm1, -15(%rax)
#endif
	ret

	.p2align 4
L(more32bytes):
	movdqu	32(%rsi), %xmm2
	movdqu	48(%rsi), %xmm3

	pcmpeqb	%xmm2, %xmm6
	pcmpeqb	%xmm3, %xmm7
	pmovmskb %xmm7, %edx
	shl	$16, %edx
	pmovmskb %xmm6, %ecx
	or	%ecx, %edx
	je	L(more64bytes)
	bsf	%edx, %edx
#ifndef AS_STRCPY
	lea	32(%rdi, %rdx), %rax
#endif
	movdqu	1(%rsi, %rdx), %xmm2
	movdqu	17(%rsi, %rdx), %xmm3
	movdqu	%xmm0, (%rdi)
	movdqu	%xmm1, 16(%rdi)
#ifdef AS_STRCPY
        movdqu  %xmm2, 1(%rdi, %rdx)
        movdqu  %xmm3, 17(%rdi, %rdx)
#else
	movdqu	%xmm2, -31(%rax)
	movdqu	%xmm3, -15(%rax)
#endif
	ret

	.p2align 4
L(more64bytes):
	movdqu	%xmm0, (%rdi)
	movdqu	%xmm1, 16(%rdi)
	movdqu	%xmm2, 32(%rdi)
	movdqu	%xmm3, 48(%rdi)
	movdqu	64(%rsi), %xmm0
	movdqu	80(%rsi), %xmm1
	movdqu	96(%rsi), %xmm2
	movdqu	112(%rsi), %xmm3

	pcmpeqb	%xmm0, %xmm4
	pcmpeqb	%xmm1, %xmm5
	pcmpeqb	%xmm2, %xmm6
	pcmpeqb	%xmm3, %xmm7
	pmovmskb %xmm4, %ecx
	pmovmskb %xmm5, %edx
	pmovmskb %xmm6, %r8d
	pmovmskb %xmm7, %r9d
	shl	$16, %edx
	or	%ecx, %edx
	shl	$32, %r8
	shl	$48, %r9
	or	%r8, %rdx
	or	%r9, %rdx
	test	%rdx, %rdx
	je	L(prepare_loop)
	bsf	%rdx, %rdx
#ifndef AS_STRCPY
	lea	64(%rdi, %rdx), %rax
#endif
	movdqu	1(%rsi, %rdx), %xmm0
	movdqu	17(%rsi, %rdx), %xmm1
	movdqu	33(%rsi, %rdx), %xmm2
	movdqu	49(%rsi, %rdx), %xmm3
#ifdef AS_STRCPY
        movdqu  %xmm0, 1(%rdi, %rdx)
        movdqu  %xmm1, 17(%rdi, %rdx)
        movdqu  %xmm2, 33(%rdi, %rdx)
        movdqu  %xmm3, 49(%rdi, %rdx)
#else
	movdqu	%xmm0, -63(%rax)
	movdqu	%xmm1, -47(%rax)
	movdqu	%xmm2, -31(%rax)
	movdqu	%xmm3, -15(%rax)
#endif
	ret


	.p2align 4
L(prepare_loop):
	movdqu	%xmm0, 64(%rdi)
	movdqu	%xmm1, 80(%rdi)
	movdqu	%xmm2, 96(%rdi)
	movdqu	%xmm3, 112(%rdi)

#ifdef USE_AVX2
	vpxor	%xmm5, %xmm5, %xmm5
#endif

	subq	%rsi, %rdi
	add	$64, %rsi
	andq	$-64, %rsi
	addq	%rsi, %rdi
	jmp	L(loop_entry)

#ifdef USE_AVX2
	.p2align 4
L(loop):
	vmovdqu	%ymm1, (%rdi)
	vmovdqu	%ymm3, 32(%rdi)
L(loop_entry):
	vmovdqa	96(%rsi), %ymm3
	vmovdqa	64(%rsi), %ymm1
	vpminub	%ymm3, %ymm1, %ymm2
	addq	$64, %rsi
	addq	$64, %rdi
	vpcmpeqb %ymm5, %ymm2, %ymm0
	vpmovmskb %ymm0, %edx
	test	%edx, %edx
	je	L(loop)
	salq	$32, %rdx
	vpcmpeqb %ymm5, %ymm1, %ymm4
	vpmovmskb %ymm4, %ecx
	or	%rcx, %rdx
	bsfq	%rdx, %rdx
#ifndef AS_STRCPY
	lea	(%rdi, %rdx), %rax
#endif
	vmovdqu	-63(%rsi, %rdx), %ymm0
	vmovdqu	-31(%rsi, %rdx), %ymm2
#ifdef AS_STRCPY
        vmovdqu  %ymm0, -63(%rdi, %rdx)
        vmovdqu  %ymm2, -31(%rdi, %rdx)
#else
	vmovdqu	%ymm0, -63(%rax)
	vmovdqu	%ymm2, -31(%rax)
#endif
	vzeroupper
	ret
#else
	.p2align 4
L(loop):
	movdqu	%xmm1, (%rdi)
	movdqu	%xmm2, 16(%rdi)
	movdqu	%xmm3, 32(%rdi)
	movdqu	%xmm4, 48(%rdi)
L(loop_entry):
	movdqa	96(%rsi), %xmm3
	movdqa	112(%rsi), %xmm4
	movdqa	%xmm3, %xmm0
	movdqa	80(%rsi), %xmm2
	pminub	%xmm4, %xmm0
	movdqa	64(%rsi), %xmm1
	pminub	%xmm2, %xmm0
	pminub	%xmm1, %xmm0
	addq	$64, %rsi
	addq	$64, %rdi
	pcmpeqb	%xmm5, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	je	L(loop)
	salq	$48, %rdx
	pcmpeqb	%xmm1, %xmm5
	pcmpeqb	%xmm2, %xmm6
	pmovmskb %xmm5, %ecx
#ifdef AS_STRCPY
	pmovmskb %xmm6, %r8d
	pcmpeqb	%xmm3, %xmm7
	pmovmskb %xmm7, %r9d
	sal	$16, %r8d
	or	%r8d, %ecx
#else
	pmovmskb %xmm6, %eax
	pcmpeqb	%xmm3, %xmm7
	pmovmskb %xmm7, %r9d
	sal	$16, %eax
	or	%eax, %ecx
#endif
	salq	$32, %r9
	orq	%rcx, %rdx
	orq	%r9, %rdx
	bsfq	%rdx, %rdx
#ifndef AS_STRCPY
	lea	(%rdi, %rdx), %rax
#endif
	movdqu	-63(%rsi, %rdx), %xmm0
	movdqu	-47(%rsi, %rdx), %xmm1
	movdqu	-31(%rsi, %rdx), %xmm2
	movdqu	-15(%rsi, %rdx), %xmm3
#ifdef AS_STRCPY
        movdqu  %xmm0, -63(%rdi, %rdx)
        movdqu  %xmm1, -47(%rdi, %rdx)
        movdqu  %xmm2, -31(%rdi, %rdx)
        movdqu  %xmm3, -15(%rdi, %rdx)
#else
	movdqu	%xmm0, -63(%rax)
	movdqu	%xmm1, -47(%rax)
	movdqu	%xmm2, -31(%rax)
	movdqu	%xmm3, -15(%rax)
#endif
	ret
#endif

	.p2align 4
L(cross_page):
	movq	%rsi, %rcx
	pxor	%xmm0, %xmm0
	and	$15, %ecx
	movq	%rsi, %r9
	movq	%rdi, %r10
	subq	%rcx, %rsi
	subq	%rcx, %rdi
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	shr	%cl, %edx
	shl	%cl, %edx
	test	%edx, %edx
	jne	L(less_32_cross)

	addq	$16, %rsi
	addq	$16, %rdi
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jne	L(less_32_cross)
	movdqu	%xmm1, (%rdi)

	movdqu	(%r9), %xmm0
	movdqu	%xmm0, (%r10)

	mov	$8, %rcx
L(cross_loop):
	addq	$16, %rsi
	addq	$16, %rdi
	pxor	%xmm0, %xmm0
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jne	L(return_cross)
	movdqu	%xmm1, (%rdi)
	sub	$1, %rcx
	ja	L(cross_loop)

#ifdef USE_AVX2
	vpxor	%xmm5, %xmm5, %xmm5
#else
	pxor	%xmm5, %xmm5
	pxor	%xmm6, %xmm6
	pxor	%xmm7, %xmm7
#endif
	lea	-64(%rsi), %rdx
	andq	$-64, %rdx
	addq	%rdx, %rdi
	subq	%rsi, %rdi
	movq	%rdx, %rsi
	jmp	L(loop_entry)

	.p2align 4
L(return_cross):
	bsf	%edx, %edx
#ifdef AS_STRCPY
        movdqu  -15(%rsi, %rdx), %xmm0
        movdqu  %xmm0, -15(%rdi, %rdx)
#else
	lea	(%rdi, %rdx), %rax
	movdqu	-15(%rsi, %rdx), %xmm0
	movdqu	%xmm0, -15(%rax)
#endif
	ret

	.p2align 4
L(less_32_cross):
	bsf	%rdx, %rdx
	lea	(%rdi, %rdx), %rcx
#ifndef AS_STRCPY
	mov	%rcx, %rax
#endif
	mov	%r9, %rsi
	mov	%r10, %rdi
	sub	%rdi, %rcx
	cmp	$15, %ecx
	jb	L(less_16_cross)
	movdqu	(%rsi), %xmm0
	movdqu	-15(%rsi, %rcx), %xmm1
	movdqu	%xmm0, (%rdi)
#ifdef AS_STRCPY
	movdqu  %xmm1, -15(%rdi, %rcx)
#else
	movdqu	%xmm1, -15(%rax)
#endif
	ret

L(less_16_cross):
	cmp	$7, %ecx
	jb	L(less_8_bytes_cross)
	movq	(%rsi), %rdx
	jmp	L(8bytes_from_cross)

L(less_8_bytes_cross):
	cmp	$2, %ecx
	jbe	L(3_bytes_cross)
	mov	(%rsi), %edx
	jmp	L(4bytes_from_cross)

L(3_bytes_cross):
	jb	L(1_2bytes_cross)
	movzwl	(%rsi), %edx
	jmp	L(_3_bytesb)

L(1_2bytes_cross):
	movb	(%rsi), %dl
	jmp	L(0_2bytes_from_cross)

	.p2align 4
L(less_4_bytesb):
	je	L(_3_bytesb)
L(0_2bytes_from_cross):
	movb	%dl, (%rdi)
#ifdef AS_STRCPY
	movb    $0, (%rdi, %rcx)
#else
	movb	$0, (%rax)
#endif
	ret

	.p2align 4
L(_3_bytesb):
	movw	%dx, (%rdi)
	movb	$0, 2(%rdi)
	ret

END(STPCPY)
