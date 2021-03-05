/* memchr/wmemchr optimized with 256-bit EVEX instructions.
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

# ifndef MEMCHR
#  define MEMCHR	__memchr_evex
# endif

# ifdef USE_AS_WMEMCHR
#  define VPBROADCAST	vpbroadcastd
#  define VPCMP		vpcmpd
#  define SHIFT_REG	r8d
# else
#  define VPBROADCAST	vpbroadcastb
#  define VPCMP		vpcmpb
#  define SHIFT_REG	ecx
# endif

# define XMMMATCH	xmm16
# define YMMMATCH	ymm16
# define YMM1		ymm17
# define YMM2		ymm18
# define YMM3		ymm19
# define YMM4		ymm20
# define YMM5		ymm21
# define YMM6		ymm22

# define VEC_SIZE 32

	.section .text.evex,"ax",@progbits
ENTRY (MEMCHR)
# ifndef USE_AS_RAWMEMCHR
	/* Check for zero length.  */
	test	%RDX_LP, %RDX_LP
	jz	L(zero)
# endif
	movl	%edi, %ecx
# ifdef USE_AS_WMEMCHR
	shl	$2, %RDX_LP
# else
#  ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%edx, %edx
#  endif
# endif
	/* Broadcast CHAR to YMMMATCH.  */
	VPBROADCAST %esi, %YMMMATCH
	/* Check if we may cross page boundary with one vector load.  */
	andl	$(2 * VEC_SIZE - 1), %ecx
	cmpl	$VEC_SIZE, %ecx
	ja	L(cros_page_boundary)

	/* Check the first VEC_SIZE bytes.  */
	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax

# ifndef USE_AS_RAWMEMCHR
	jnz	L(first_vec_x0_check)
	/* Adjust length and check the end of data.  */
	subq	$VEC_SIZE, %rdx
	jbe	L(zero)
# else
	jnz	L(first_vec_x0)
# endif

	/* Align data for aligned loads in the loop.  */
	addq	$VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi

# ifndef USE_AS_RAWMEMCHR
	/* Adjust length.  */
	addq	%rcx, %rdx

	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif
	jmp	L(more_4x_vec)

	.p2align 4
L(cros_page_boundary):
	andl	$(VEC_SIZE - 1), %ecx
# ifdef USE_AS_WMEMCHR
	/* NB: Divide shift count by 4 since each bit in K1 represent 4
	   bytes.  */
	movl	%ecx, %SHIFT_REG
	sarl	$2, %SHIFT_REG
# endif
	andq	$-VEC_SIZE, %rdi
	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	/* Remove the leading bytes.  */
	sarxl	%SHIFT_REG, %eax, %eax
	testl	%eax, %eax
	jz	L(aligned_more)
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
# ifndef USE_AS_RAWMEMCHR
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
# endif
	addq	%rdi, %rax
	addq	%rcx, %rax
	ret

	.p2align 4
L(aligned_more):
# ifndef USE_AS_RAWMEMCHR
        /* Calculate "rdx + rcx - VEC_SIZE" with "rdx - (VEC_SIZE - rcx)"
	   instead of "(rdx + rcx) - VEC_SIZE" to void possible addition
	   overflow.  */
	negq	%rcx
	addq	$VEC_SIZE, %rcx

	/* Check the end of data.  */
	subq	%rcx, %rdx
	jbe	L(zero)
# endif

	addq	$VEC_SIZE, %rdi

# ifndef USE_AS_RAWMEMCHR
	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif

L(more_4x_vec):
	/* Check the first 4 * VEC_SIZE.  Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.  */
	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMP	$0, (VEC_SIZE * 2)(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)

	VPCMP	$0, (VEC_SIZE * 3)(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x3)

	addq	$(VEC_SIZE * 4), %rdi

# ifndef USE_AS_RAWMEMCHR
	subq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_4x_vec_or_less)
# endif

	/* Align data to 4 * VEC_SIZE.  */
	movq	%rdi, %rcx
	andl	$(4 * VEC_SIZE - 1), %ecx
	andq	$-(4 * VEC_SIZE), %rdi

# ifndef USE_AS_RAWMEMCHR
	/* Adjust length.  */
	addq	%rcx, %rdx
# endif

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	VPCMP	$0, VEC_SIZE(%rdi), %YMMMATCH, %k2
	kord	%k1, %k2, %k5
	VPCMP	$0, (VEC_SIZE * 2)(%rdi), %YMMMATCH, %k3
	VPCMP	$0, (VEC_SIZE * 3)(%rdi), %YMMMATCH, %k4

	kord	%k3, %k4, %k6
	kortestd %k5, %k6
	jnz	L(4x_vec_end)

	addq	$(VEC_SIZE * 4), %rdi

# ifdef USE_AS_RAWMEMCHR
	jmp	L(loop_4x_vec)
# else
	subq	$(VEC_SIZE * 4), %rdx
	ja	L(loop_4x_vec)

L(last_4x_vec_or_less):
	/* Less than 4 * VEC and aligned to VEC_SIZE.  */
	addl	$(VEC_SIZE * 2), %edx
	jle	L(last_2x_vec)

	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMP	$0, (VEC_SIZE * 2)(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x2_check)
	subl	$VEC_SIZE, %edx
	jle	L(zero)

	VPCMP	$0, (VEC_SIZE * 3)(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x3_check)
	xorl	%eax, %eax
	ret

	.p2align 4
L(last_2x_vec):
	addl	$(VEC_SIZE * 2), %edx
	VPCMP	$0, (%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x0_check)
	subl	$VEC_SIZE, %edx
	jle	L(zero)

	VPCMP	$0, VEC_SIZE(%rdi), %YMMMATCH, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1_check)
	xorl	%eax, %eax
	ret

	.p2align 4
L(first_vec_x0_check):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
	addq	%rdi, %rax
	ret

	.p2align 4
L(first_vec_x1_check):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
	ret

	.p2align 4
L(first_vec_x2_check):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
	ret

	.p2align 4
L(first_vec_x3_check):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	sall	$2, %eax
# endif
	/* Check the end of data.  */
	cmpq	%rax, %rdx
	jbe	L(zero)
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
	ret

	.p2align 4
L(zero):
	xorl	%eax, %eax
	ret
# endif

	.p2align 4
L(first_vec_x0):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(%rdi, %rax, 4), %rax
# else
	addq	%rdi, %rax
# endif
	ret

	.p2align 4
L(first_vec_x1):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	VEC_SIZE(%rdi, %rax, 4), %rax
# else
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
# endif
	ret

	.p2align 4
L(first_vec_x2):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(VEC_SIZE * 2)(%rdi, %rax, 4), %rax
# else
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
# endif
	ret

	.p2align 4
L(4x_vec_end):
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)
	kmovd	%k2, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)
	kmovd	%k3, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)
	kmovd	%k4, %eax
	testl	%eax, %eax
L(first_vec_x3):
	tzcntl	%eax, %eax
# ifdef USE_AS_WMEMCHR
	/* NB: Multiply wchar_t count by 4 to get the number of bytes.  */
	leaq	(VEC_SIZE * 3)(%rdi, %rax, 4), %rax
# else
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
# endif
	ret

END (MEMCHR)
#endif
