/* strcmp/wcscmp/strncmp/wcsncmp optimized with 256-bit EVEX instructions.
   Copyright (C) 2021-2022 Free Software Foundation, Inc.
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

# ifndef STRCMP
#  define STRCMP	__strcmp_evex
# endif

# define PAGE_SIZE	4096

/* VEC_SIZE = Number of bytes in a ymm register */
# define VEC_SIZE	32

/* Shift for dividing by (VEC_SIZE * 4).  */
# define DIVIDE_BY_VEC_4_SHIFT	7
# if (VEC_SIZE * 4) != (1 << DIVIDE_BY_VEC_4_SHIFT)
#  error (VEC_SIZE * 4) != (1 << DIVIDE_BY_VEC_4_SHIFT)
# endif

# define VMOVU		vmovdqu64
# define VMOVA		vmovdqa64

# ifdef USE_AS_WCSCMP
/* Compare packed dwords.  */
#  define VPCMP		vpcmpd
#  define VPMINU	vpminud
#  define VPTESTM	vptestmd
#  define SHIFT_REG32	r8d
#  define SHIFT_REG64	r8
/* 1 dword char == 4 bytes.  */
#  define SIZE_OF_CHAR	4
# else
/* Compare packed bytes.  */
#  define VPCMP		vpcmpb
#  define VPMINU	vpminub
#  define VPTESTM	vptestmb
#  define SHIFT_REG32	ecx
#  define SHIFT_REG64	rcx
/* 1 byte char == 1 byte.  */
#  define SIZE_OF_CHAR	1
# endif

# define XMMZERO	xmm16
# define XMM0		xmm17
# define XMM1		xmm18

# define YMMZERO	ymm16
# define YMM0		ymm17
# define YMM1		ymm18
# define YMM2		ymm19
# define YMM3		ymm20
# define YMM4		ymm21
# define YMM5		ymm22
# define YMM6		ymm23
# define YMM7		ymm24
# define YMM8		ymm25
# define YMM9		ymm26
# define YMM10		ymm27

/* Warning!
           wcscmp/wcsncmp have to use SIGNED comparison for elements.
           strcmp/strncmp have to use UNSIGNED comparison for elements.
*/

/* The main idea of the string comparison (byte or dword) using 256-bit
   EVEX instructions consists of comparing (VPCMP) two ymm vectors. The
   latter can be on either packed bytes or dwords depending on
   USE_AS_WCSCMP. In order to check the null CHAR, algorithm keeps the
   matched bytes/dwords, requiring 5 EVEX instructions (3 VPCMP and 2
   KORD). In general, the costs of comparing VEC_SIZE bytes (32-bytes)
   are 3 VPCMP and 2 KORD instructions, together with VMOVU and ktestd
   instructions.  Main loop (away from from page boundary) compares 4
   vectors are a time, effectively comparing 4 x VEC_SIZE bytes (128
   bytes) on each loop.

   The routine strncmp/wcsncmp (enabled by defining USE_AS_STRNCMP) logic
   is the same as strcmp, except that an a maximum offset is tracked.  If
   the maximum offset is reached before a difference is found, zero is
   returned.  */

	.section .text.evex,"ax",@progbits
ENTRY (STRCMP)
# ifdef USE_AS_STRNCMP
	/* Check for simple cases (0 or 1) in offset.  */
	cmp	$1, %RDX_LP
	je	L(char0)
	jb	L(zero)
#  ifdef USE_AS_WCSCMP
#  ifndef __ILP32__
	movq	%rdx, %rcx
	/* Check if length could overflow when multiplied by
	   sizeof(wchar_t). Checking top 8 bits will cover all potential
	   overflow cases as well as redirect cases where its impossible to
	   length to bound a valid memory region. In these cases just use
	   'wcscmp'.  */
	shrq	$56, %rcx
	jnz	__wcscmp_evex
#  endif
	/* Convert units: from wide to byte char.  */
	shl	$2, %RDX_LP
#  endif
	/* Register %r11 tracks the maximum offset.  */
	mov	%RDX_LP, %R11_LP
# endif
	movl	%edi, %eax
	xorl	%edx, %edx
	/* Make %XMMZERO (%YMMZERO) all zeros in this function.  */
	vpxorq	%XMMZERO, %XMMZERO, %XMMZERO
	orl	%esi, %eax
	andl	$(PAGE_SIZE - 1), %eax
	cmpl	$(PAGE_SIZE - (VEC_SIZE * 4)), %eax
	jg	L(cross_page)
	/* Start comparing 4 vectors.  */
	VMOVU	(%rdi), %YMM0

	/* Each bit set in K2 represents a non-null CHAR in YMM0.  */
	VPTESTM	%YMM0, %YMM0, %k2

	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (%rsi).  */
	VPCMP	$0, (%rsi), %YMM0, %k1{%k2}

	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	je	L(next_3_vectors)
	tzcntl	%ecx, %edx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edx
# endif
# ifdef USE_AS_STRNCMP
	/* Return 0 if the mismatched index (%rdx) is after the maximum
	   offset (%r11).   */
	cmpq	%r11, %rdx
	jae	L(zero)
# endif
# ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi, %rdx), %ecx
	cmpl	(%rsi, %rdx), %ecx
	je	L(return)
L(wcscmp_return):
	setl	%al
	negl	%eax
	orl	$1, %eax
L(return):
# else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	subl	%edx, %eax
# endif
	ret

L(return_vec_size):
	tzcntl	%ecx, %edx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edx
# endif
# ifdef USE_AS_STRNCMP
	/* Return 0 if the mismatched index (%rdx + VEC_SIZE) is after
	   the maximum offset (%r11).  */
	addq	$VEC_SIZE, %rdx
	cmpq	%r11, %rdx
	jae	L(zero)
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi, %rdx), %ecx
	cmpl	(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	VEC_SIZE(%rdi, %rdx), %ecx
	cmpl	VEC_SIZE(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	VEC_SIZE(%rdi, %rdx), %eax
	movzbl	VEC_SIZE(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

L(return_2_vec_size):
	tzcntl	%ecx, %edx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edx
# endif
# ifdef USE_AS_STRNCMP
	/* Return 0 if the mismatched index (%rdx + 2 * VEC_SIZE) is
	   after the maximum offset (%r11).  */
	addq	$(VEC_SIZE * 2), %rdx
	cmpq	%r11, %rdx
	jae	L(zero)
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi, %rdx), %ecx
	cmpl	(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(VEC_SIZE * 2)(%rdi, %rdx), %ecx
	cmpl	(VEC_SIZE * 2)(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(VEC_SIZE * 2)(%rdi, %rdx), %eax
	movzbl	(VEC_SIZE * 2)(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

L(return_3_vec_size):
	tzcntl	%ecx, %edx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edx
# endif
# ifdef USE_AS_STRNCMP
	/* Return 0 if the mismatched index (%rdx + 3 * VEC_SIZE) is
	   after the maximum offset (%r11).  */
	addq	$(VEC_SIZE * 3), %rdx
	cmpq	%r11, %rdx
	jae	L(zero)
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi, %rdx), %ecx
	cmpl	(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(VEC_SIZE * 3)(%rdi, %rdx), %ecx
	cmpl	(VEC_SIZE * 3)(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(VEC_SIZE * 3)(%rdi, %rdx), %eax
	movzbl	(VEC_SIZE * 3)(%rsi, %rdx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(next_3_vectors):
	VMOVU	VEC_SIZE(%rdi), %YMM0
	/* Each bit set in K2 represents a non-null CHAR in YMM0.  */
	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at VEC_SIZE(%rsi).  */
	VPCMP	$0, VEC_SIZE(%rsi), %YMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	jne	L(return_vec_size)

	VMOVU	(VEC_SIZE * 2)(%rdi), %YMM0
	/* Each bit set in K2 represents a non-null CHAR in YMM0.  */
	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (VEC_SIZE * 2)(%rsi).  */
	VPCMP	$0, (VEC_SIZE * 2)(%rsi), %YMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	jne	L(return_2_vec_size)

	VMOVU	(VEC_SIZE * 3)(%rdi), %YMM0
	/* Each bit set in K2 represents a non-null CHAR in YMM0.  */
	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (VEC_SIZE * 2)(%rsi).  */
	VPCMP	$0, (VEC_SIZE * 3)(%rsi), %YMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	jne	L(return_3_vec_size)
L(main_loop_header):
	leaq	(VEC_SIZE * 4)(%rdi), %rdx
	movl	$PAGE_SIZE, %ecx
	/* Align load via RAX.  */
	andq	$-(VEC_SIZE * 4), %rdx
	subq	%rdi, %rdx
	leaq	(%rdi, %rdx), %rax
# ifdef USE_AS_STRNCMP
	/* Starting from this point, the maximum offset, or simply the
	   'offset', DECREASES by the same amount when base pointers are
	   moved forward.  Return 0 when:
	     1) On match: offset <= the matched vector index.
	     2) On mistmach, offset is before the mistmatched index.
	 */
	subq	%rdx, %r11
	jbe	L(zero)
# endif
	addq	%rsi, %rdx
	movq	%rdx, %rsi
	andl	$(PAGE_SIZE - 1), %esi
	/* Number of bytes before page crossing.  */
	subq	%rsi, %rcx
	/* Number of VEC_SIZE * 4 blocks before page crossing.  */
	shrq	$DIVIDE_BY_VEC_4_SHIFT, %rcx
	/* ESI: Number of VEC_SIZE * 4 blocks before page crossing.   */
	movl	%ecx, %esi
	jmp	L(loop_start)

	.p2align 4
L(loop):
# ifdef USE_AS_STRNCMP
	/* Base pointers are moved forward by 4 * VEC_SIZE.  Decrease
	   the maximum offset (%r11) by the same amount.  */
	subq	$(VEC_SIZE * 4), %r11
	jbe	L(zero)
# endif
	addq	$(VEC_SIZE * 4), %rax
	addq	$(VEC_SIZE * 4), %rdx
L(loop_start):
	testl	%esi, %esi
	leal	-1(%esi), %esi
	je	L(loop_cross_page)
L(back_to_loop):
	/* Main loop, comparing 4 vectors are a time.  */
	VMOVA	(%rax), %YMM0
	VMOVA	VEC_SIZE(%rax), %YMM2
	VMOVA	(VEC_SIZE * 2)(%rax), %YMM4
	VMOVA	(VEC_SIZE * 3)(%rax), %YMM6

	VPMINU	%YMM0, %YMM2, %YMM8
	VPMINU	%YMM4, %YMM6, %YMM9

	/* A zero CHAR in YMM8 means that there is a null CHAR.  */
	VPMINU	%YMM8, %YMM9, %YMM8

	/* Each bit set in K1 represents a non-null CHAR in YMM8.  */
	VPTESTM	%YMM8, %YMM8, %k1

	/* (YMM ^ YMM): A non-zero CHAR represents a mismatch.  */
	vpxorq	(%rdx), %YMM0, %YMM1
	vpxorq	VEC_SIZE(%rdx), %YMM2, %YMM3
	vpxorq	(VEC_SIZE * 2)(%rdx), %YMM4, %YMM5
	vpxorq	(VEC_SIZE * 3)(%rdx), %YMM6, %YMM7

	vporq	%YMM1, %YMM3, %YMM9
	vporq	%YMM5, %YMM7, %YMM10

	/* A non-zero CHAR in YMM9 represents a mismatch.  */
	vporq	%YMM9, %YMM10, %YMM9

	/* Each bit cleared in K0 represents a mismatch or a null CHAR.  */
	VPCMP	$0, %YMMZERO, %YMM9, %k0{%k1}
	kmovd   %k0, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	je	 L(loop)

	/* Each bit set in K1 represents a non-null CHAR in YMM0.  */
	VPTESTM	%YMM0, %YMM0, %k1
	/* Each bit cleared in K0 represents a mismatch or a null CHAR
	   in YMM0 and (%rdx).  */
	VPCMP	$0, %YMMZERO, %YMM1, %k0{%k1}
	kmovd	%k0, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	je	L(test_vec)
	tzcntl	%ecx, %ecx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %ecx
# endif
# ifdef USE_AS_STRNCMP
	cmpq	%rcx, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %edi
	cmpl	(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %edi
	cmpl	(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(test_vec):
# ifdef USE_AS_STRNCMP
	/* The first vector matched.  Return 0 if the maximum offset
	   (%r11) <= VEC_SIZE.  */
	cmpq	$VEC_SIZE, %r11
	jbe	L(zero)
# endif
	/* Each bit set in K1 represents a non-null CHAR in YMM2.  */
	VPTESTM	%YMM2, %YMM2, %k1
	/* Each bit cleared in K0 represents a mismatch or a null CHAR
	   in YMM2 and VEC_SIZE(%rdx).  */
	VPCMP	$0, %YMMZERO, %YMM3, %k0{%k1}
	kmovd	%k0, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	je	L(test_2_vec)
	tzcntl	%ecx, %edi
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edi
# endif
# ifdef USE_AS_STRNCMP
	addq	$VEC_SIZE, %rdi
	cmpq	%rdi, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rdi), %ecx
	cmpl	(%rdx, %rdi), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rdi), %eax
	movzbl	(%rdx, %rdi), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	VEC_SIZE(%rsi, %rdi), %ecx
	cmpl	VEC_SIZE(%rdx, %rdi), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	VEC_SIZE(%rax, %rdi), %eax
	movzbl	VEC_SIZE(%rdx, %rdi), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(test_2_vec):
# ifdef USE_AS_STRNCMP
	/* The first 2 vectors matched.  Return 0 if the maximum offset
	   (%r11) <= 2 * VEC_SIZE.  */
	cmpq	$(VEC_SIZE * 2), %r11
	jbe	L(zero)
# endif
	/* Each bit set in K1 represents a non-null CHAR in YMM4.  */
	VPTESTM	%YMM4, %YMM4, %k1
	/* Each bit cleared in K0 represents a mismatch or a null CHAR
	   in YMM4 and (VEC_SIZE * 2)(%rdx).  */
	VPCMP	$0, %YMMZERO, %YMM5, %k0{%k1}
	kmovd	%k0, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	je	L(test_3_vec)
	tzcntl	%ecx, %edi
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edi
# endif
# ifdef USE_AS_STRNCMP
	addq	$(VEC_SIZE * 2), %rdi
	cmpq	%rdi, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rdi), %ecx
	cmpl	(%rdx, %rdi), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rdi), %eax
	movzbl	(%rdx, %rdi), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(VEC_SIZE * 2)(%rsi, %rdi), %ecx
	cmpl	(VEC_SIZE * 2)(%rdx, %rdi), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(VEC_SIZE * 2)(%rax, %rdi), %eax
	movzbl	(VEC_SIZE * 2)(%rdx, %rdi), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(test_3_vec):
# ifdef USE_AS_STRNCMP
	/* The first 3 vectors matched.  Return 0 if the maximum offset
	   (%r11) <= 3 * VEC_SIZE.  */
	cmpq	$(VEC_SIZE * 3), %r11
	jbe	L(zero)
# endif
	/* Each bit set in K1 represents a non-null CHAR in YMM6.  */
	VPTESTM	%YMM6, %YMM6, %k1
	/* Each bit cleared in K0 represents a mismatch or a null CHAR
	   in YMM6 and (VEC_SIZE * 3)(%rdx).  */
	VPCMP	$0, %YMMZERO, %YMM7, %k0{%k1}
	kmovd	%k0, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	tzcntl	%ecx, %ecx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %ecx
# endif
# ifdef USE_AS_STRNCMP
	addq	$(VEC_SIZE * 3), %rcx
	cmpq	%rcx, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %esi
	cmpl	(%rdx, %rcx), %esi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(VEC_SIZE * 3)(%rsi, %rcx), %esi
	cmpl	(VEC_SIZE * 3)(%rdx, %rcx), %esi
	jne	L(wcscmp_return)
#  else
	movzbl	(VEC_SIZE * 3)(%rax, %rcx), %eax
	movzbl	(VEC_SIZE * 3)(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(loop_cross_page):
	xorl	%r10d, %r10d
	movq	%rdx, %rcx
	/* Align load via RDX.  We load the extra ECX bytes which should
	   be ignored.  */
	andl	$((VEC_SIZE * 4) - 1), %ecx
	/* R10 is -RCX.  */
	subq	%rcx, %r10

	/* This works only if VEC_SIZE * 2 == 64. */
# if (VEC_SIZE * 2) != 64
#  error (VEC_SIZE * 2) != 64
# endif

	/* Check if the first VEC_SIZE * 2 bytes should be ignored.  */
	cmpl	$(VEC_SIZE * 2), %ecx
	jge	L(loop_cross_page_2_vec)

	VMOVU	(%rax, %r10), %YMM2
	VMOVU	VEC_SIZE(%rax, %r10), %YMM3

	/* Each bit set in K2 represents a non-null CHAR in YMM2.  */
	VPTESTM	%YMM2, %YMM2, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM2 and 32 bytes at (%rdx, %r10).  */
	VPCMP	$0, (%rdx, %r10), %YMM2, %k1{%k2}
	kmovd	%k1, %r9d
	/* Don't use subl since it is the lower 16/32 bits of RDI
	   below.  */
	notl	%r9d
# ifdef USE_AS_WCSCMP
	/* Only last 8 bits are valid.  */
	andl	$0xff, %r9d
# endif

	/* Each bit set in K4 represents a non-null CHAR in YMM3.  */
	VPTESTM	%YMM3, %YMM3, %k4
	/* Each bit cleared in K3 represents a mismatch or a null CHAR
	   in YMM3 and 32 bytes at VEC_SIZE(%rdx, %r10).  */
	VPCMP	$0, VEC_SIZE(%rdx, %r10), %YMM3, %k3{%k4}
	kmovd	%k3, %edi
    /* Must use notl %edi here as lower bits are for CHAR
	   comparisons potentially out of range thus can be 0 without
	   indicating mismatch.  */
	notl	%edi
# ifdef USE_AS_WCSCMP
	/* Don't use subl since it is the upper 8 bits of EDI below.  */
	andl	$0xff, %edi
# endif

# ifdef USE_AS_WCSCMP
	/* NB: Each bit in EDI/R9D represents 4-byte element.  */
	sall	$8, %edi
	/* NB: Divide shift count by 4 since each bit in K1 represent 4
	   bytes.  */
	movl	%ecx, %SHIFT_REG32
	sarl	$2, %SHIFT_REG32

	/* Each bit in EDI represents a null CHAR or a mismatch.  */
	orl	%r9d, %edi
# else
	salq	$32, %rdi

	/* Each bit in RDI represents a null CHAR or a mismatch.  */
	orq	%r9, %rdi
# endif

	/* Since ECX < VEC_SIZE * 2, simply skip the first ECX bytes.  */
	shrxq	%SHIFT_REG64, %rdi, %rdi
	testq	%rdi, %rdi
	je	L(loop_cross_page_2_vec)
	tzcntq	%rdi, %rcx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %ecx
# endif
# ifdef USE_AS_STRNCMP
	cmpq	%rcx, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %edi
	cmpl	(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %edi
	cmpl	(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

	.p2align 4
L(loop_cross_page_2_vec):
	/* The first VEC_SIZE * 2 bytes match or are ignored.  */
	VMOVU	(VEC_SIZE * 2)(%rax, %r10), %YMM0
	VMOVU	(VEC_SIZE * 3)(%rax, %r10), %YMM1

	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (VEC_SIZE * 2)(%rdx, %r10).  */
	VPCMP	$0, (VEC_SIZE * 2)(%rdx, %r10), %YMM0, %k1{%k2}
	kmovd	%k1, %r9d
	/* Don't use subl since it is the lower 16/32 bits of RDI
	   below.  */
	notl	%r9d
# ifdef USE_AS_WCSCMP
	/* Only last 8 bits are valid.  */
	andl	$0xff, %r9d
# endif

	VPTESTM	%YMM1, %YMM1, %k4
	/* Each bit cleared in K3 represents a mismatch or a null CHAR
	   in YMM1 and 32 bytes at (VEC_SIZE * 3)(%rdx, %r10).  */
	VPCMP	$0, (VEC_SIZE * 3)(%rdx, %r10), %YMM1, %k3{%k4}
	kmovd	%k3, %edi
	/* Must use notl %edi here as lower bits are for CHAR
	   comparisons potentially out of range thus can be 0 without
	   indicating mismatch.  */
	notl	%edi
# ifdef USE_AS_WCSCMP
	/* Don't use subl since it is the upper 8 bits of EDI below.  */
	andl	$0xff, %edi
# endif

# ifdef USE_AS_WCSCMP
	/* NB: Each bit in EDI/R9D represents 4-byte element.  */
	sall	$8, %edi

	/* Each bit in EDI represents a null CHAR or a mismatch.  */
	orl	%r9d, %edi
# else
	salq	$32, %rdi

	/* Each bit in RDI represents a null CHAR or a mismatch.  */
	orq	%r9, %rdi
# endif

	xorl	%r8d, %r8d
	/* If ECX > VEC_SIZE * 2, skip ECX - (VEC_SIZE * 2) bytes.  */
	subl	$(VEC_SIZE * 2), %ecx
	jle	1f
	/* R8 has number of bytes skipped.  */
	movl	%ecx, %r8d
# ifdef USE_AS_WCSCMP
	/* NB: Divide shift count by 4 since each bit in RDI represent 4
	   bytes.  */
	sarl	$2, %ecx
	/* Skip ECX bytes.  */
	shrl	%cl, %edi
# else
	/* Skip ECX bytes.  */
	shrq	%cl, %rdi
# endif
1:
	/* Before jumping back to the loop, set ESI to the number of
	   VEC_SIZE * 4 blocks before page crossing.  */
	movl	$(PAGE_SIZE / (VEC_SIZE * 4) - 1), %esi

	testq	%rdi, %rdi
# ifdef USE_AS_STRNCMP
	/* At this point, if %rdi value is 0, it already tested
	   VEC_SIZE*4+%r10 byte starting from %rax. This label
	   checks whether strncmp maximum offset reached or not.  */
	je	L(string_nbyte_offset_check)
# else
	je	L(back_to_loop)
# endif
	tzcntq	%rdi, %rcx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %ecx
# endif
	addq	%r10, %rcx
	/* Adjust for number of bytes skipped.  */
	addq	%r8, %rcx
# ifdef USE_AS_STRNCMP
	addq	$(VEC_SIZE * 2), %rcx
	subq	%rcx, %r11
	jbe	L(zero)
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(%rsi, %rcx), %edi
	cmpl	(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(%rax, %rcx), %eax
	movzbl	(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# else
#  ifdef USE_AS_WCSCMP
	movq	%rax, %rsi
	xorl	%eax, %eax
	movl	(VEC_SIZE * 2)(%rsi, %rcx), %edi
	cmpl	(VEC_SIZE * 2)(%rdx, %rcx), %edi
	jne	L(wcscmp_return)
#  else
	movzbl	(VEC_SIZE * 2)(%rax, %rcx), %eax
	movzbl	(VEC_SIZE * 2)(%rdx, %rcx), %edx
	subl	%edx, %eax
#  endif
# endif
	ret

# ifdef USE_AS_STRNCMP
L(string_nbyte_offset_check):
	leaq	(VEC_SIZE * 4)(%r10), %r10
	cmpq	%r10, %r11
	jbe	L(zero)
	jmp	L(back_to_loop)
# endif

	.p2align 4
L(cross_page_loop):
	/* Check one byte/dword at a time.  */
# ifdef USE_AS_WCSCMP
	cmpl	%ecx, %eax
# else
	subl	%ecx, %eax
# endif
	jne	L(different)
	addl	$SIZE_OF_CHAR, %edx
	cmpl	$(VEC_SIZE * 4), %edx
	je	L(main_loop_header)
# ifdef USE_AS_STRNCMP
	cmpq	%r11, %rdx
	jae	L(zero)
# endif
# ifdef USE_AS_WCSCMP
	movl	(%rdi, %rdx), %eax
	movl	(%rsi, %rdx), %ecx
# else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %ecx
# endif
	/* Check null CHAR.  */
	testl	%eax, %eax
	jne	L(cross_page_loop)
	/* Since %eax == 0, subtract is OK for both SIGNED and UNSIGNED
	   comparisons.  */
	subl	%ecx, %eax
# ifndef USE_AS_WCSCMP
L(different):
# endif
	ret

# ifdef USE_AS_WCSCMP
	.p2align 4
L(different):
	/* Use movl to avoid modifying EFLAGS.  */
	movl	$0, %eax
	setl	%al
	negl	%eax
	orl	$1, %eax
	ret
# endif

# ifdef USE_AS_STRNCMP
	.p2align 4
L(zero):
	xorl	%eax, %eax
	ret

	.p2align 4
L(char0):
#  ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi), %ecx
	cmpl	(%rsi), %ecx
	jne	L(wcscmp_return)
#  else
	movzbl	(%rsi), %ecx
	movzbl	(%rdi), %eax
	subl	%ecx, %eax
#  endif
	ret
# endif

	.p2align 4
L(last_vector):
	addq	%rdx, %rdi
	addq	%rdx, %rsi
# ifdef USE_AS_STRNCMP
	subq	%rdx, %r11
# endif
	tzcntl	%ecx, %edx
# ifdef USE_AS_WCSCMP
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %edx
# endif
# ifdef USE_AS_STRNCMP
	cmpq	%r11, %rdx
	jae	L(zero)
# endif
# ifdef USE_AS_WCSCMP
	xorl	%eax, %eax
	movl	(%rdi, %rdx), %ecx
	cmpl	(%rsi, %rdx), %ecx
	jne	L(wcscmp_return)
# else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	subl	%edx, %eax
# endif
	ret

	/* Comparing on page boundary region requires special treatment:
	   It must done one vector at the time, starting with the wider
	   ymm vector if possible, if not, with xmm. If fetching 16 bytes
	   (xmm) still passes the boundary, byte comparison must be done.
	 */
	.p2align 4
L(cross_page):
	/* Try one ymm vector at a time.  */
	cmpl	$(PAGE_SIZE - VEC_SIZE), %eax
	jg	L(cross_page_1_vector)
L(loop_1_vector):
	VMOVU	(%rdi, %rdx), %YMM0

	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (%rsi, %rdx).  */
	VPCMP	$0, (%rsi, %rdx), %YMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xff, %ecx
# else
	incl	%ecx
# endif
	jne	L(last_vector)

	addl	$VEC_SIZE, %edx

	addl	$VEC_SIZE, %eax
# ifdef USE_AS_STRNCMP
	/* Return 0 if the current offset (%rdx) >= the maximum offset
	   (%r11).  */
	cmpq	%r11, %rdx
	jae	L(zero)
# endif
	cmpl	$(PAGE_SIZE - VEC_SIZE), %eax
	jle	L(loop_1_vector)
L(cross_page_1_vector):
	/* Less than 32 bytes to check, try one xmm vector.  */
	cmpl	$(PAGE_SIZE - 16), %eax
	jg	L(cross_page_1_xmm)
	VMOVU	(%rdi, %rdx), %XMM0

	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in XMM0 and 16 bytes at (%rsi, %rdx).  */
	VPCMP	$0, (%rsi, %rdx), %XMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xf, %ecx
# else
	subl	$0xffff, %ecx
# endif
	jne	L(last_vector)

	addl	$16, %edx
# ifndef USE_AS_WCSCMP
	addl	$16, %eax
# endif
# ifdef USE_AS_STRNCMP
	/* Return 0 if the current offset (%rdx) >= the maximum offset
	   (%r11).  */
	cmpq	%r11, %rdx
	jae	L(zero)
# endif

L(cross_page_1_xmm):
# ifndef USE_AS_WCSCMP
	/* Less than 16 bytes to check, try 8 byte vector.  NB: No need
	   for wcscmp nor wcsncmp since wide char is 4 bytes.   */
	cmpl	$(PAGE_SIZE - 8), %eax
	jg	L(cross_page_8bytes)
	vmovq	(%rdi, %rdx), %XMM0
	vmovq	(%rsi, %rdx), %XMM1

	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in XMM0 and XMM1.  */
	VPCMP	$0, %XMM1, %XMM0, %k1{%k2}
	kmovb	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0x3, %ecx
# else
	subl	$0xff, %ecx
# endif
	jne	L(last_vector)

	addl	$8, %edx
	addl	$8, %eax
#  ifdef USE_AS_STRNCMP
	/* Return 0 if the current offset (%rdx) >= the maximum offset
	   (%r11).  */
	cmpq	%r11, %rdx
	jae	L(zero)
#  endif

L(cross_page_8bytes):
	/* Less than 8 bytes to check, try 4 byte vector.  */
	cmpl	$(PAGE_SIZE - 4), %eax
	jg	L(cross_page_4bytes)
	vmovd	(%rdi, %rdx), %XMM0
	vmovd	(%rsi, %rdx), %XMM1

	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in XMM0 and XMM1.  */
	VPCMP	$0, %XMM1, %XMM0, %k1{%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0x1, %ecx
# else
	subl	$0xf, %ecx
# endif
	jne	L(last_vector)

	addl	$4, %edx
#  ifdef USE_AS_STRNCMP
	/* Return 0 if the current offset (%rdx) >= the maximum offset
	   (%r11).  */
	cmpq	%r11, %rdx
	jae	L(zero)
#  endif

L(cross_page_4bytes):
# endif
	/* Less than 4 bytes to check, try one byte/dword at a time.  */
# ifdef USE_AS_STRNCMP
	cmpq	%r11, %rdx
	jae	L(zero)
# endif
# ifdef USE_AS_WCSCMP
	movl	(%rdi, %rdx), %eax
	movl	(%rsi, %rdx), %ecx
# else
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %ecx
# endif
	testl	%eax, %eax
	jne	L(cross_page_loop)
	subl	%ecx, %eax
	ret
END (STRCMP)
#endif
