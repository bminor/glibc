/* strchr/strchrnul optimized with 256-bit EVEX instructions.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

# ifndef STRCHR
#  define STRCHR	__strchr_evex
# endif

# define VMOVU		vmovdqu64
# define VMOVA		vmovdqa64

# ifdef USE_AS_WCSCHR
#  define VPBROADCAST	vpbroadcastd
#  define VPCMP		vpcmpd
#  define VPMINU	vpminud
#  define CHAR_REG	esi
#  define SHIFT_REG	r8d
# else
#  define VPBROADCAST	vpbroadcastb
#  define VPCMP		vpcmpb
#  define VPMINU	vpminub
#  define CHAR_REG	sil
#  define SHIFT_REG	ecx
# endif

# define XMMZERO	xmm16

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

# define VEC_SIZE 32
# define PAGE_SIZE 4096

	.section .text.evex,"ax",@progbits
ENTRY (STRCHR)
	movl	%edi, %ecx
# ifndef USE_AS_STRCHRNUL
	xorl	%edx, %edx
# endif

	/* Broadcast CHAR to YMM0.	*/
	VPBROADCAST %esi, %YMM0

	vpxorq	%XMMZERO, %XMMZERO, %XMMZERO

	/* Check if we cross page boundary with one vector load.  */
	andl	$(PAGE_SIZE - 1), %ecx
	cmpl	$(PAGE_SIZE - VEC_SIZE), %ecx
	ja  L(cross_page_boundary)

	/* Check the first VEC_SIZE bytes. Search for both CHAR and the
	   null bytes.  */
	VMOVU	(%rdi), %YMM1

	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	ktestd	%k0, %k0
	jz	L(more_vecs)
	kmovd	%k0, %eax
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(%rdi, %rax, 4), %rax
# else
	addq	%rdi, %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

	.p2align 4
L(more_vecs):
	/* Align data for aligned loads in the loop.  */
	andq	$-VEC_SIZE, %rdi
L(aligned_more):

	/* Check the next 4 * VEC_SIZE.	 Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.	*/
	VMOVA	VEC_SIZE(%rdi), %YMM1
	addq	$VEC_SIZE, %rdi

	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VMOVA	VEC_SIZE(%rdi), %YMM1
	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VMOVA	(VEC_SIZE * 2)(%rdi), %YMM1
	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)

	VMOVA	(VEC_SIZE * 3)(%rdi), %YMM1
	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	ktestd	%k0, %k0
	jz	L(prep_loop_4x)

	kmovd	%k0, %eax
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(VEC_SIZE * 3)(%rdi, %rax, 4), %rax
# else
	leaq	(VEC_SIZE * 3)(%rdi, %rax), %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

	.p2align 4
L(first_vec_x0):
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(%rdi, %rax, 4), %rax
# else
	addq	%rdi, %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

	.p2align 4
L(first_vec_x1):
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	VEC_SIZE(%rdi, %rax, 4), %rax
# else
	leaq	VEC_SIZE(%rdi, %rax), %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

	.p2align 4
L(first_vec_x2):
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(VEC_SIZE * 2)(%rdi, %rax, 4), %rax
# else
	leaq	(VEC_SIZE * 2)(%rdi, %rax), %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

L(prep_loop_4x):
	/* Align data to 4 * VEC_SIZE.	*/
	andq	$-(VEC_SIZE * 4), %rdi

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	VMOVA	(VEC_SIZE * 4)(%rdi), %YMM1
	VMOVA	(VEC_SIZE * 5)(%rdi), %YMM2
	VMOVA	(VEC_SIZE * 6)(%rdi), %YMM3
	VMOVA	(VEC_SIZE * 7)(%rdi), %YMM4

	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM5
	vpxorq	%YMM2, %YMM0, %YMM6
	vpxorq	%YMM3, %YMM0, %YMM7
	vpxorq	%YMM4, %YMM0, %YMM8

	VPMINU	%YMM5, %YMM1, %YMM5
	VPMINU	%YMM6, %YMM2, %YMM6
	VPMINU	%YMM7, %YMM3, %YMM7
	VPMINU	%YMM8, %YMM4, %YMM8

	VPMINU	%YMM5, %YMM6, %YMM1
	VPMINU	%YMM7, %YMM8, %YMM2

	VPMINU	%YMM1, %YMM2, %YMM1

	/* Each bit in K0 represents a CHAR or a null byte.  */
	VPCMP	$0, %YMMZERO, %YMM1, %k0

	addq	$(VEC_SIZE * 4), %rdi

	ktestd	%k0, %k0
	jz	L(loop_4x_vec)

	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM5, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	/* Each bit in K1 represents a CHAR or a null byte in YMM2.  */
	VPCMP	$0, %YMMZERO, %YMM6, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	/* Each bit in K2 represents a CHAR or a null byte in YMM3.  */
	VPCMP	$0, %YMMZERO, %YMM7, %k2
	/* Each bit in K3 represents a CHAR or a null byte in YMM4.  */
	VPCMP	$0, %YMMZERO, %YMM8, %k3

# ifdef USE_AS_WCSCHR
	/* NB: Each bit in K2/K3 represents 4-byte element.  */
	kshiftlw $8, %k3, %k1
# else
	kshiftlq $32, %k3, %k1
# endif

	/* Each bit in K1 represents a NULL or a mismatch.  */
	korq	%k1, %k2, %k1
	kmovq	%k1, %rax

	tzcntq  %rax, %rax
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(VEC_SIZE * 2)(%rdi, %rax, 4), %rax
# else
	leaq	(VEC_SIZE * 2)(%rdi, %rax), %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

	/* Cold case for crossing page with first load.	 */
	.p2align 4
L(cross_page_boundary):
	andq	$-VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx

	VMOVA	(%rdi), %YMM1

	/* Leaves only CHARS matching esi as 0.  */
	vpxorq	%YMM1, %YMM0, %YMM2
	VPMINU	%YMM2, %YMM1, %YMM2
	/* Each bit in K0 represents a CHAR or a null byte in YMM1.  */
	VPCMP	$0, %YMMZERO, %YMM2, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax

# ifdef USE_AS_WCSCHR
	/* NB: Divide shift count by 4 since each bit in K1 represent 4
	   bytes.  */
	movl	%ecx, %SHIFT_REG
	sarl    $2, %SHIFT_REG
# endif

	/* Remove the leading bits.	 */
	sarxl	%SHIFT_REG, %eax, %eax
	testl	%eax, %eax

	jz	L(aligned_more)
	tzcntl	%eax, %eax
	addq	%rcx, %rdi
# ifdef USE_AS_WCSCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(%rdi, %rax, 4), %rax
# else
	addq	%rdi, %rax
# endif
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	ret

END (STRCHR)
# endif
