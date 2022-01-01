/* memcmp with SSE4.1, wmemcmp with SSE4.1
   Copyright (C) 2010-2022 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#if IS_IN (libc)

# include <sysdep.h>

# ifndef MEMCMP
#  define MEMCMP	__memcmp_sse4_1
# endif

#ifdef USE_AS_WMEMCMP
# define CMPEQ	pcmpeqd
# define CHAR_SIZE	4
#else
# define CMPEQ	pcmpeqb
# define CHAR_SIZE	1
#endif


/* Warning!
           wmemcmp has to use SIGNED comparison for elements.
           memcmp has to use UNSIGNED comparison for elemnts.
*/

	.section .text.sse4.1,"ax",@progbits
ENTRY (MEMCMP)
# ifdef USE_AS_WMEMCMP
	shl	$2, %RDX_LP
# elif defined __ILP32__
	/* Clear the upper 32 bits.  */
	mov	%edx, %edx
# endif
	cmp	$79, %RDX_LP
	ja	L(79bytesormore)

	cmp	$CHAR_SIZE, %RDX_LP
	jbe	L(firstbyte)

	/* N in (CHAR_SIZE, 79) bytes.  */
	cmpl	$32, %edx
	ja	L(more_32_bytes)

	cmpl	$16, %edx
	jae	L(16_to_32_bytes)

# ifndef USE_AS_WMEMCMP
	cmpl	$8, %edx
	jae	L(8_to_16_bytes)

	cmpl	$4, %edx
	jb	L(2_to_3_bytes)

	movl	(%rdi), %eax
	movl	(%rsi), %ecx

	bswap	%eax
	bswap	%ecx

	shlq	$32, %rax
	shlq	$32, %rcx

	movl	-4(%rdi, %rdx), %edi
	movl	-4(%rsi, %rdx), %esi

	bswap	%edi
	bswap	%esi

	orq	%rdi, %rax
	orq	%rsi, %rcx
	subq	%rcx, %rax
	cmovne	%edx, %eax
	sbbl	%ecx, %ecx
	orl	%ecx, %eax
	ret

	.p2align 4,, 8
L(2_to_3_bytes):
	movzwl	(%rdi), %eax
	movzwl	(%rsi), %ecx
	shll	$8, %eax
	shll	$8, %ecx
	bswap	%eax
	bswap	%ecx
	movzbl	-1(%rdi, %rdx), %edi
	movzbl	-1(%rsi, %rdx), %esi
	orl	%edi, %eax
	orl	%esi, %ecx
	subl	%ecx, %eax
	ret

	.p2align 4,, 8
L(8_to_16_bytes):
	movq	(%rdi), %rax
	movq	(%rsi), %rcx

	bswap	%rax
	bswap	%rcx

	subq	%rcx, %rax
	jne	L(8_to_16_bytes_done)

	movq	-8(%rdi, %rdx), %rax
	movq	-8(%rsi, %rdx), %rcx

	bswap	%rax
	bswap	%rcx

	subq	%rcx, %rax

L(8_to_16_bytes_done):
	cmovne	%edx, %eax
	sbbl	%ecx, %ecx
	orl	%ecx, %eax
	ret
# else
	xorl	%eax, %eax
	movl	(%rdi), %ecx
	cmpl	(%rsi), %ecx
	jne	L(8_to_16_bytes_done)
	movl	4(%rdi), %ecx
	cmpl	4(%rsi), %ecx
	jne	L(8_to_16_bytes_done)
	movl	-4(%rdi, %rdx), %ecx
	cmpl	-4(%rsi, %rdx), %ecx
	jne	L(8_to_16_bytes_done)
	ret
# endif

	.p2align 4,, 3
L(ret_zero):
	xorl	%eax, %eax
L(zero):
	ret

	.p2align 4,, 8
L(firstbyte):
	jb	L(ret_zero)
# ifdef USE_AS_WMEMCMP
	xorl	%eax, %eax
	movl	(%rdi), %ecx
	cmpl	(%rsi), %ecx
	je	L(zero)
L(8_to_16_bytes_done):
	setg	%al
	leal	-1(%rax, %rax), %eax
# else
	movzbl	(%rdi), %eax
	movzbl	(%rsi), %ecx
	sub	%ecx, %eax
# endif
	ret

	.p2align 4
L(vec_return_begin_48):
	addq	$16, %rdi
	addq	$16, %rsi
L(vec_return_begin_32):
	bsfl	%eax, %eax
# ifdef USE_AS_WMEMCMP
	movl	32(%rdi, %rax), %ecx
	xorl	%edx, %edx
	cmpl	32(%rsi, %rax), %ecx
	setg	%dl
	leal	-1(%rdx, %rdx), %eax
# else
	movzbl	32(%rsi, %rax), %ecx
	movzbl	32(%rdi, %rax), %eax
	subl	%ecx, %eax
# endif
	ret

	.p2align 4
L(vec_return_begin_16):
	addq	$16, %rdi
	addq	$16, %rsi
L(vec_return_begin):
	bsfl	%eax, %eax
# ifdef USE_AS_WMEMCMP
	movl	(%rdi, %rax), %ecx
	xorl	%edx, %edx
	cmpl	(%rsi, %rax), %ecx
	setg	%dl
	leal	-1(%rdx, %rdx), %eax
# else
	movzbl	(%rsi, %rax), %ecx
	movzbl	(%rdi, %rax), %eax
	subl	%ecx, %eax
# endif
	ret

	.p2align 4
L(vec_return_end_16):
	subl	$16, %edx
L(vec_return_end):
	bsfl	%eax, %eax
	addl	%edx, %eax
# ifdef USE_AS_WMEMCMP
	movl	-16(%rdi, %rax), %ecx
	xorl	%edx, %edx
	cmpl	-16(%rsi, %rax), %ecx
	setg	%dl
	leal	-1(%rdx, %rdx), %eax
# else
	movzbl	-16(%rsi, %rax), %ecx
	movzbl	-16(%rdi, %rax), %eax
	subl	%ecx, %eax
# endif
	ret

	.p2align 4,, 8
L(more_32_bytes):
	movdqu	(%rdi), %xmm0
	movdqu	(%rsi), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	16(%rdi), %xmm0
	movdqu	16(%rsi), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	cmpl	$64, %edx
	jbe	L(32_to_64_bytes)
	movdqu	32(%rdi), %xmm0
	movdqu	32(%rsi), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	.p2align 4,, 6
L(32_to_64_bytes):
	movdqu	-32(%rdi, %rdx), %xmm0
	movdqu	-32(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end_16)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret

	.p2align 4
L(16_to_32_bytes):
	movdqu	(%rdi), %xmm0
	movdqu	(%rsi), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret


	.p2align 4
L(79bytesormore):
	movdqu	(%rdi), %xmm0
	movdqu	(%rsi), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)


	mov	%rsi, %rcx
	and	$-16, %rsi
	add	$16, %rsi
	sub	%rsi, %rcx

	sub	%rcx, %rdi
	add	%rcx, %rdx
	test	$0xf, %rdi
	jz	L(2aligned)

	cmp	$128, %rdx
	ja	L(128bytesormore)

	.p2align 4,, 6
L(less128bytes):
	movdqu	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqu	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqu	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	cmp	$96, %rdx
	jb	L(32_to_64_bytes)

	addq	$64, %rdi
	addq	$64, %rsi
	subq	$64, %rdx

	.p2align 4,, 6
L(last_64_bytes):
	movdqu	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqu	-32(%rdi, %rdx), %xmm0
	movdqu	-32(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end_16)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret

	.p2align 4
L(128bytesormore):
	cmp	$256, %rdx
	ja	L(unaligned_loop)
L(less256bytes):
	movdqu	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqu	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqu	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	addq	$64, %rdi
	addq	$64, %rsi

	movdqu	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqu	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqu	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqu	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	addq	$-128, %rdx
	subq	$-64, %rsi
	subq	$-64, %rdi

	cmp	$64, %rdx
	ja	L(less128bytes)

	cmp	$32, %rdx
	ja	L(last_64_bytes)

	movdqu	-32(%rdi, %rdx), %xmm0
	movdqu	-32(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end_16)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret

	.p2align 4
L(unaligned_loop):
# ifdef DATA_CACHE_SIZE_HALF
	mov	$DATA_CACHE_SIZE_HALF, %R8_LP
# else
	mov	__x86_data_cache_size_half(%rip), %R8_LP
# endif
	movq	%r8, %r9
	addq	%r8, %r8
	addq	%r9, %r8
	cmpq	%r8, %rdx
	ja	L(L2_L3_cache_unaligned)
	sub	$64, %rdx
	.p2align 4
L(64bytesormore_loop):
	movdqu	(%rdi), %xmm0
	movdqu	16(%rdi), %xmm1
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3

	CMPEQ	(%rsi), %xmm0
	CMPEQ	16(%rsi), %xmm1
	CMPEQ	32(%rsi), %xmm2
	CMPEQ	48(%rsi), %xmm3

	pand	%xmm0, %xmm1
	pand	%xmm2, %xmm3
	pand	%xmm1, %xmm3

	pmovmskb %xmm3, %eax
	incw	%ax
	jnz	L(64bytesormore_loop_end)

	add	$64, %rsi
	add	$64, %rdi
	sub	$64, %rdx
	ja	L(64bytesormore_loop)

	.p2align 4,, 6
L(loop_tail):
	addq	%rdx, %rdi
	movdqu	(%rdi), %xmm0
	movdqu	16(%rdi), %xmm1
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3

	addq	%rdx, %rsi
	movdqu	(%rsi), %xmm4
	movdqu	16(%rsi), %xmm5
	movdqu	32(%rsi), %xmm6
	movdqu	48(%rsi), %xmm7

	CMPEQ	%xmm4, %xmm0
	CMPEQ	%xmm5, %xmm1
	CMPEQ	%xmm6, %xmm2
	CMPEQ	%xmm7, %xmm3

	pand	%xmm0, %xmm1
	pand	%xmm2, %xmm3
	pand	%xmm1, %xmm3

	pmovmskb %xmm3, %eax
	incw	%ax
	jnz	L(64bytesormore_loop_end)
	ret

L(L2_L3_cache_unaligned):
	subq	$64, %rdx
	.p2align 4
L(L2_L3_unaligned_128bytes_loop):
	prefetchnta 0x1c0(%rdi)
	prefetchnta 0x1c0(%rsi)

	movdqu	(%rdi), %xmm0
	movdqu	16(%rdi), %xmm1
	movdqu	32(%rdi), %xmm2
	movdqu	48(%rdi), %xmm3

	CMPEQ	(%rsi), %xmm0
	CMPEQ	16(%rsi), %xmm1
	CMPEQ	32(%rsi), %xmm2
	CMPEQ	48(%rsi), %xmm3

	pand	%xmm0, %xmm1
	pand	%xmm2, %xmm3
	pand	%xmm1, %xmm3

	pmovmskb %xmm3, %eax
	incw	%ax
	jnz	L(64bytesormore_loop_end)

	add	$64, %rsi
	add	$64, %rdi
	sub	$64, %rdx
	ja	L(L2_L3_unaligned_128bytes_loop)
	jmp	L(loop_tail)


	/* This case is for machines which are sensitive for unaligned
	 * instructions.  */
	.p2align 4
L(2aligned):
	cmp	$128, %rdx
	ja	L(128bytesormorein2aligned)
L(less128bytesin2aligned):
	movdqa	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqa	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqa	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqa	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	cmp	$96, %rdx
	jb	L(32_to_64_bytes)

	addq	$64, %rdi
	addq	$64, %rsi
	subq	$64, %rdx

	.p2align 4,, 6
L(aligned_last_64_bytes):
	movdqa	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqa	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqu	-32(%rdi, %rdx), %xmm0
	movdqu	-32(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end_16)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret

	.p2align 4
L(128bytesormorein2aligned):
	cmp	$256, %rdx
	ja	L(aligned_loop)
L(less256bytesin2alinged):
	movdqa	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqa	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqa	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqa	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	addq	$64, %rdi
	addq	$64, %rsi

	movdqa	(%rdi), %xmm1
	CMPEQ	(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin)

	movdqa	16(%rdi), %xmm1
	CMPEQ	16(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_16)

	movdqa	32(%rdi), %xmm1
	CMPEQ	32(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_32)

	movdqa	48(%rdi), %xmm1
	CMPEQ	48(%rsi), %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_begin_48)

	addq	$-128, %rdx
	subq	$-64, %rsi
	subq	$-64, %rdi

	cmp	$64, %rdx
	ja	L(less128bytesin2aligned)

	cmp	$32, %rdx
	ja	L(aligned_last_64_bytes)

	movdqu	-32(%rdi, %rdx), %xmm0
	movdqu	-32(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end_16)

	movdqu	-16(%rdi, %rdx), %xmm0
	movdqu	-16(%rsi, %rdx), %xmm1
	CMPEQ	%xmm0, %xmm1
	pmovmskb %xmm1, %eax
	incw	%ax
	jnz	L(vec_return_end)
	ret

	.p2align 4
L(aligned_loop):
# ifdef DATA_CACHE_SIZE_HALF
	mov	$DATA_CACHE_SIZE_HALF, %R8_LP
# else
	mov	__x86_data_cache_size_half(%rip), %R8_LP
# endif
	movq	%r8, %r9
	addq	%r8, %r8
	addq	%r9, %r8
	cmpq	%r8, %rdx
	ja	L(L2_L3_cache_aligned)

	sub	$64, %rdx
	.p2align 4
L(64bytesormore_loopin2aligned):
	movdqa	(%rdi), %xmm0
	movdqa	16(%rdi), %xmm1
	movdqa	32(%rdi), %xmm2
	movdqa	48(%rdi), %xmm3

	CMPEQ	(%rsi), %xmm0
	CMPEQ	16(%rsi), %xmm1
	CMPEQ	32(%rsi), %xmm2
	CMPEQ	48(%rsi), %xmm3

	pand	%xmm0, %xmm1
	pand	%xmm2, %xmm3
	pand	%xmm1, %xmm3

	pmovmskb %xmm3, %eax
	incw	%ax
	jnz	L(64bytesormore_loop_end)
	add	$64, %rsi
	add	$64, %rdi
	sub	$64, %rdx
	ja	L(64bytesormore_loopin2aligned)
	jmp	L(loop_tail)

L(L2_L3_cache_aligned):
	subq	$64, %rdx
	.p2align 4
L(L2_L3_aligned_128bytes_loop):
	prefetchnta 0x1c0(%rdi)
	prefetchnta 0x1c0(%rsi)
	movdqa	(%rdi), %xmm0
	movdqa	16(%rdi), %xmm1
	movdqa	32(%rdi), %xmm2
	movdqa	48(%rdi), %xmm3

	CMPEQ	(%rsi), %xmm0
	CMPEQ	16(%rsi), %xmm1
	CMPEQ	32(%rsi), %xmm2
	CMPEQ	48(%rsi), %xmm3

	pand	%xmm0, %xmm1
	pand	%xmm2, %xmm3
	pand	%xmm1, %xmm3

	pmovmskb %xmm3, %eax
	incw	%ax
	jnz	L(64bytesormore_loop_end)

	addq	$64, %rsi
	addq	$64, %rdi
	subq	$64, %rdx
	ja	L(L2_L3_aligned_128bytes_loop)
	jmp	L(loop_tail)

	.p2align 4
L(64bytesormore_loop_end):
	pmovmskb %xmm0, %ecx
	incw	%cx
	jnz	L(loop_end_ret)

	pmovmskb %xmm1, %ecx
	notw	%cx
	sall	$16, %ecx
	jnz	L(loop_end_ret)

	pmovmskb %xmm2, %ecx
	notw	%cx
	shlq	$32, %rcx
	jnz	L(loop_end_ret)

	addq	$48, %rdi
	addq	$48, %rsi
	movq	%rax, %rcx

	.p2align 4,, 6
L(loop_end_ret):
	bsfq	%rcx, %rcx
# ifdef USE_AS_WMEMCMP
	movl	(%rdi, %rcx), %eax
	xorl	%edx, %edx
	cmpl	(%rsi, %rcx), %eax
	setg	%dl
	leal	-1(%rdx, %rdx), %eax
# else
	movzbl	(%rdi, %rcx), %eax
	movzbl	(%rsi, %rcx), %ecx
	subl	%ecx, %eax
# endif
	ret
END (MEMCMP)
#endif
