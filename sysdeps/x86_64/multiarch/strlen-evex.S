/* strlen/strnlen/wcslen/wcsnlen optimized with 256-bit EVEX instructions.
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

# ifndef STRLEN
#  define STRLEN	__strlen_evex
# endif

# define VMOVA		vmovdqa64

# ifdef USE_AS_WCSLEN
#  define VPCMP		vpcmpd
#  define VPMINU	vpminud
#  define SHIFT_REG	r9d
# else
#  define VPCMP		vpcmpb
#  define VPMINU	vpminub
#  define SHIFT_REG	ecx
# endif

# define XMMZERO	xmm16
# define YMMZERO	ymm16
# define YMM1		ymm17
# define YMM2		ymm18
# define YMM3		ymm19
# define YMM4		ymm20
# define YMM5		ymm21
# define YMM6		ymm22

# define VEC_SIZE 32

	.section .text.evex,"ax",@progbits
ENTRY (STRLEN)
# ifdef USE_AS_STRNLEN
	/* Check for zero length.  */
	test	%RSI_LP, %RSI_LP
	jz	L(zero)
#  ifdef USE_AS_WCSLEN
	shl	$2, %RSI_LP
#  elif defined __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%esi, %esi
#  endif
	mov	%RSI_LP, %R8_LP
# endif
	movl	%edi, %ecx
	movq	%rdi, %rdx
	vpxorq	%XMMZERO, %XMMZERO, %XMMZERO

	/* Check if we may cross page boundary with one vector load.  */
	andl	$(2 * VEC_SIZE - 1), %ecx
	cmpl	$VEC_SIZE, %ecx
	ja	L(cros_page_boundary)

	/* Check the first VEC_SIZE bytes.  Each bit in K0 represents a
	   null byte.  */
	VPCMP	$0, (%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax

# ifdef USE_AS_STRNLEN
	jnz	L(first_vec_x0_check)
	/* Adjust length and check the end of data.  */
	subq	$VEC_SIZE, %rsi
	jbe	L(max)
# else
	jnz	L(first_vec_x0)
# endif

	/* Align data for aligned loads in the loop.  */
	addq	$VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi

# ifdef USE_AS_STRNLEN
	/* Adjust length.  */
	addq	%rcx, %rsi

	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif
	jmp	L(more_4x_vec)

	.p2align 4
L(cros_page_boundary):
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi

# ifdef USE_AS_WCSLEN
	/* NB: Divide shift count by 4 since each bit in K0 represent 4
	   bytes.  */
	movl	%ecx, %SHIFT_REG
	sarl	$2, %SHIFT_REG
# endif
	VPCMP	$0, (%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax

	/* Remove the leading bytes.  */
	sarxl	%SHIFT_REG, %eax, %eax
	testl	%eax, %eax
	jz	L(aligned_more)
	tzcntl	%eax, %eax
# ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
# ifdef USE_AS_STRNLEN
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
# endif
	addq	%rdi, %rax
	addq	%rcx, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	ret

	.p2align 4
L(aligned_more):
# ifdef USE_AS_STRNLEN
        /* "rcx" is less than VEC_SIZE.  Calculate "rdx + rcx - VEC_SIZE"
	    with "rdx - (VEC_SIZE - rcx)" instead of "(rdx + rcx) - VEC_SIZE"
	    to void possible addition overflow.  */
	negq	%rcx
	addq	$VEC_SIZE, %rcx

	/* Check the end of data.  */
	subq	%rcx, %rsi
	jbe	L(max)
# endif

	addq	$VEC_SIZE, %rdi

# ifdef USE_AS_STRNLEN
	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif

L(more_4x_vec):
	/* Check the first 4 * VEC_SIZE.  Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.  */
	VPCMP	$0, (%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMP	$0, (VEC_SIZE * 2)(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)

	VPCMP	$0, (VEC_SIZE * 3)(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x3)

	addq	$(VEC_SIZE * 4), %rdi

# ifdef USE_AS_STRNLEN
	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif

	/* Align data to 4 * VEC_SIZE.  */
	movq	%rdi, %rcx
	andl	$(4 * VEC_SIZE - 1), %ecx
	andq	$-(4 * VEC_SIZE), %rdi

# ifdef USE_AS_STRNLEN
	/* Adjust length.  */
	addq	%rcx, %rsi
# endif

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	VMOVA	(%rdi), %YMM1
	VMOVA	VEC_SIZE(%rdi), %YMM2
	VMOVA	(VEC_SIZE * 2)(%rdi), %YMM3
	VMOVA	(VEC_SIZE * 3)(%rdi), %YMM4

	VPMINU	%YMM1, %YMM2, %YMM5
	VPMINU	%YMM3, %YMM4, %YMM6

	VPMINU	%YMM5, %YMM6, %YMM5
	VPCMP	$0, %YMM5, %YMMZERO, %k0
	ktestd	%k0, %k0
	jnz	L(4x_vec_end)

	addq	$(VEC_SIZE * 4), %rdi

# ifndef USE_AS_STRNLEN
	jmp	L(loop_4x_vec)
# else
	subq	$(VEC_SIZE * 4), %rsi
	ja	L(loop_4x_vec)

L(last_4x_vec_or_less):
	/* Less than 4 * VEC and aligned to VEC_SIZE.  */
	addl	$(VEC_SIZE * 2), %esi
	jle	L(last_2x_vec)

	VPCMP	$0, (%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMP	$0, (VEC_SIZE * 2)(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2_check)
	subl	$VEC_SIZE, %esi
	jle	L(max)

	VPCMP	$0, (VEC_SIZE * 3)(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x3_check)
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(last_2x_vec):
	addl	$(VEC_SIZE * 2), %esi

	VPCMP	$0, (%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0_check)
	subl	$VEC_SIZE, %esi
	jle	L(max)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1_check)
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(first_vec_x0_check):
	tzcntl	%eax, %eax
#  ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
#  endif
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(first_vec_x1_check):
	tzcntl	%eax, %eax
#  ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
#  endif
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(first_vec_x2_check):
	tzcntl	%eax, %eax
#  ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
#  endif
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(first_vec_x3_check):
	tzcntl	%eax, %eax
#  ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
#  endif
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(max):
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	ret

	.p2align 4
L(zero):
	xorl	%eax, %eax
	ret
# endif

	.p2align 4
L(first_vec_x0):
	tzcntl	%eax, %eax
# ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	ret

	.p2align 4
L(first_vec_x1):
	tzcntl	%eax, %eax
# ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	ret

	.p2align 4
L(first_vec_x2):
	tzcntl	%eax, %eax
# ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	ret

	.p2align 4
L(4x_vec_end):
	VPCMP	$0, %YMM1, %YMMZERO, %k0
	kmovd	%k0, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)
	VPCMP	$0, %YMM2, %YMMZERO, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)
	VPCMP	$0, %YMM3, %YMMZERO, %k2
	kmovd	%k2, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)
	VPCMP	$0, %YMM4, %YMMZERO, %k3
	kmovd	%k3, %eax
L(first_vec_x3):
	tzcntl	%eax, %eax
# ifdef USE_AS_WCSLEN
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	ret

END (STRLEN)
#endif
