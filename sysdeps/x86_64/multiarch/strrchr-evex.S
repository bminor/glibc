/* strrchr/wcsrchr optimized with 256-bit EVEX instructions.
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

#include <isa-level.h>

#if ISA_SHOULD_BUILD (4)

# include <sysdep.h>

# ifndef STRRCHR
#  define STRRCHR	__strrchr_evex
# endif

# define VMOVU	vmovdqu64
# define VMOVA	vmovdqa64

# ifdef USE_AS_WCSRCHR
#  define SHIFT_REG	esi

#  define kunpck	kunpckbw
#  define kmov_2x	kmovd
#  define maskz_2x	ecx
#  define maskm_2x	eax
#  define CHAR_SIZE	4
#  define VPMIN	vpminud
#  define VPTESTN	vptestnmd
#  define VPBROADCAST	vpbroadcastd
#  define VPCMP	vpcmpd
# else
#  define SHIFT_REG	edi

#  define kunpck	kunpckdq
#  define kmov_2x	kmovq
#  define maskz_2x	rcx
#  define maskm_2x	rax

#  define CHAR_SIZE	1
#  define VPMIN	vpminub
#  define VPTESTN	vptestnmb
#  define VPBROADCAST	vpbroadcastb
#  define VPCMP	vpcmpb
# endif

# define XMMZERO	xmm16
# define YMMZERO	ymm16
# define YMMMATCH	ymm17
# define YMMSAVE	ymm18

# define YMM1	ymm19
# define YMM2	ymm20
# define YMM3	ymm21
# define YMM4	ymm22
# define YMM5	ymm23
# define YMM6	ymm24
# define YMM7	ymm25
# define YMM8	ymm26


# define VEC_SIZE	32
# define PAGE_SIZE	4096
	.section .text.evex, "ax", @progbits
ENTRY(STRRCHR)
	movl	%edi, %eax
	/* Broadcast CHAR to YMMMATCH.  */
	VPBROADCAST %esi, %YMMMATCH

	andl	$(PAGE_SIZE - 1), %eax
	cmpl	$(PAGE_SIZE - VEC_SIZE), %eax
	jg	L(cross_page_boundary)

L(page_cross_continue):
	VMOVU	(%rdi), %YMM1
	/* k0 has a 1 for each zero CHAR in YMM1.  */
	VPTESTN	%YMM1, %YMM1, %k0
	kmovd	%k0, %ecx
	testl	%ecx, %ecx
	jz	L(aligned_more)
	/* fallthrough: zero CHAR in first VEC.  */

	/* K1 has a 1 for each search CHAR match in YMM1.  */
	VPCMP	$0, %YMMMATCH, %YMM1, %k1
	kmovd	%k1, %eax
	/* Build mask up until first zero CHAR (used to mask of
	   potential search CHAR matches past the end of the string).
	 */
	blsmskl	%ecx, %ecx
	andl	%ecx, %eax
	jz	L(ret0)
	/* Get last match (the `andl` removed any out of bounds
	   matches).  */
	bsrl	%eax, %eax
# ifdef USE_AS_WCSRCHR
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
# else
	addq	%rdi, %rax
# endif
L(ret0):
	ret

	/* Returns for first vec x1/x2/x3 have hard coded backward
	   search path for earlier matches.  */
	.p2align 4,, 6
L(first_vec_x1):
	VPCMP	$0, %YMMMATCH, %YMM2, %k1
	kmovd	%k1, %eax
	blsmskl	%ecx, %ecx
	/* eax non-zero if search CHAR in range.  */
	andl	%ecx, %eax
	jnz	L(first_vec_x1_return)

	/* fallthrough: no match in YMM2 then need to check for earlier
	   matches (in YMM1).  */
	.p2align 4,, 4
L(first_vec_x0_test):
	VPCMP	$0, %YMMMATCH, %YMM1, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jz	L(ret1)
	bsrl	%eax, %eax
# ifdef USE_AS_WCSRCHR
	leaq	(%rsi, %rax, CHAR_SIZE), %rax
# else
	addq	%rsi, %rax
# endif
L(ret1):
	ret

	.p2align 4,, 10
L(first_vec_x1_or_x2):
	VPCMP	$0, %YMM3, %YMMMATCH, %k3
	VPCMP	$0, %YMM2, %YMMMATCH, %k2
	/* K2 and K3 have 1 for any search CHAR match. Test if any
	   matches between either of them. Otherwise check YMM1.  */
	kortestd %k2, %k3
	jz	L(first_vec_x0_test)

	/* Guranteed that YMM2 and YMM3 are within range so merge the
	   two bitmasks then get last result.  */
	kunpck	%k2, %k3, %k3
	kmovq	%k3, %rax
	bsrq	%rax, %rax
	leaq	(VEC_SIZE)(%r8, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4,, 6
L(first_vec_x3):
	VPCMP	$0, %YMMMATCH, %YMM4, %k1
	kmovd	%k1, %eax
	blsmskl	%ecx, %ecx
	/* If no search CHAR match in range check YMM1/YMM2/YMM3.  */
	andl	%ecx, %eax
	jz	L(first_vec_x1_or_x2)
	bsrl	%eax, %eax
	leaq	(VEC_SIZE * 3)(%rdi, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4,, 6
L(first_vec_x0_x1_test):
	VPCMP	$0, %YMMMATCH, %YMM2, %k1
	kmovd	%k1, %eax
	/* Check YMM2 for last match first. If no match try YMM1.  */
	testl	%eax, %eax
	jz	L(first_vec_x0_test)
	.p2align 4,, 4
L(first_vec_x1_return):
	bsrl	%eax, %eax
	leaq	(VEC_SIZE)(%rdi, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4,, 10
L(first_vec_x2):
	VPCMP	$0, %YMMMATCH, %YMM3, %k1
	kmovd	%k1, %eax
	blsmskl	%ecx, %ecx
	/* Check YMM3 for last match first. If no match try YMM2/YMM1.
	 */
	andl	%ecx, %eax
	jz	L(first_vec_x0_x1_test)
	bsrl	%eax, %eax
	leaq	(VEC_SIZE * 2)(%rdi, %rax, CHAR_SIZE), %rax
	ret


	.p2align 4
L(aligned_more):
	/* Need to keep original pointer incase YMM1 has last match.  */
	movq	%rdi, %rsi
	andq	$-VEC_SIZE, %rdi
	VMOVU	VEC_SIZE(%rdi), %YMM2
	VPTESTN	%YMM2, %YMM2, %k0
	kmovd	%k0, %ecx
	testl	%ecx, %ecx
	jnz	L(first_vec_x1)

	VMOVU	(VEC_SIZE * 2)(%rdi), %YMM3
	VPTESTN	%YMM3, %YMM3, %k0
	kmovd	%k0, %ecx
	testl	%ecx, %ecx
	jnz	L(first_vec_x2)

	VMOVU	(VEC_SIZE * 3)(%rdi), %YMM4
	VPTESTN	%YMM4, %YMM4, %k0
	kmovd	%k0, %ecx
	movq	%rdi, %r8
	testl	%ecx, %ecx
	jnz	L(first_vec_x3)

	andq	$-(VEC_SIZE * 2), %rdi
	.p2align 4
L(first_aligned_loop):
	/* Preserve YMM1, YMM2, YMM3, and YMM4 until we can gurantee
	   they don't store a match.  */
	VMOVA	(VEC_SIZE * 4)(%rdi), %YMM5
	VMOVA	(VEC_SIZE * 5)(%rdi), %YMM6

	VPCMP	$0, %YMM5, %YMMMATCH, %k2
	vpxord	%YMM6, %YMMMATCH, %YMM7

	VPMIN	%YMM5, %YMM6, %YMM8
	VPMIN	%YMM8, %YMM7, %YMM7

	VPTESTN	%YMM7, %YMM7, %k1
	subq	$(VEC_SIZE * -2), %rdi
	kortestd %k1, %k2
	jz	L(first_aligned_loop)

	VPCMP	$0, %YMM6, %YMMMATCH, %k3
	VPTESTN	%YMM8, %YMM8, %k1
	ktestd	%k1, %k1
	jz	L(second_aligned_loop_prep)

	kortestd %k2, %k3
	jnz	L(return_first_aligned_loop)

	.p2align 4,, 6
L(first_vec_x1_or_x2_or_x3):
	VPCMP	$0, %YMM4, %YMMMATCH, %k4
	kmovd	%k4, %eax
	testl	%eax, %eax
	jz	L(first_vec_x1_or_x2)
	bsrl	%eax, %eax
	leaq	(VEC_SIZE * 3)(%r8, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4,, 8
L(return_first_aligned_loop):
	VPTESTN	%YMM5, %YMM5, %k0
	kunpck	%k0, %k1, %k0
	kmov_2x	%k0, %maskz_2x

	blsmsk	%maskz_2x, %maskz_2x
	kunpck	%k2, %k3, %k3
	kmov_2x	%k3, %maskm_2x
	and	%maskz_2x, %maskm_2x
	jz	L(first_vec_x1_or_x2_or_x3)

	bsr	%maskm_2x, %maskm_2x
	leaq	(VEC_SIZE * 2)(%rdi, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4
	/* We can throw away the work done for the first 4x checks here
	   as we have a later match. This is the 'fast' path persay.
	 */
L(second_aligned_loop_prep):
L(second_aligned_loop_set_furthest_match):
	movq	%rdi, %rsi
	kunpck	%k2, %k3, %k4

	.p2align 4
L(second_aligned_loop):
	VMOVU	(VEC_SIZE * 4)(%rdi), %YMM1
	VMOVU	(VEC_SIZE * 5)(%rdi), %YMM2

	VPCMP	$0, %YMM1, %YMMMATCH, %k2
	vpxord	%YMM2, %YMMMATCH, %YMM3

	VPMIN	%YMM1, %YMM2, %YMM4
	VPMIN	%YMM3, %YMM4, %YMM3

	VPTESTN	%YMM3, %YMM3, %k1
	subq	$(VEC_SIZE * -2), %rdi
	kortestd %k1, %k2
	jz	L(second_aligned_loop)

	VPCMP	$0, %YMM2, %YMMMATCH, %k3
	VPTESTN	%YMM4, %YMM4, %k1
	ktestd	%k1, %k1
	jz	L(second_aligned_loop_set_furthest_match)

	kortestd %k2, %k3
	/* branch here because there is a significant advantage interms
	   of output dependency chance in using edx.  */
	jnz	L(return_new_match)
L(return_old_match):
	kmovq	%k4, %rax
	bsrq	%rax, %rax
	leaq	(VEC_SIZE * 2)(%rsi, %rax, CHAR_SIZE), %rax
	ret

L(return_new_match):
	VPTESTN	%YMM1, %YMM1, %k0
	kunpck	%k0, %k1, %k0
	kmov_2x	%k0, %maskz_2x

	blsmsk	%maskz_2x, %maskz_2x
	kunpck	%k2, %k3, %k3
	kmov_2x	%k3, %maskm_2x
	and	%maskz_2x, %maskm_2x
	jz	L(return_old_match)

	bsr	%maskm_2x, %maskm_2x
	leaq	(VEC_SIZE * 2)(%rdi, %rax, CHAR_SIZE), %rax
	ret

L(cross_page_boundary):
	/* eax contains all the page offset bits of src (rdi). `xor rdi,
	   rax` sets pointer will all page offset bits cleared so
	   offset of (PAGE_SIZE - VEC_SIZE) will get last aligned VEC
	   before page cross (guranteed to be safe to read). Doing this
	   as opposed to `movq %rdi, %rax; andq $-VEC_SIZE, %rax` saves
	   a bit of code size.  */
	xorq	%rdi, %rax
	VMOVU	(PAGE_SIZE - VEC_SIZE)(%rax), %YMM1
	VPTESTN	%YMM1, %YMM1, %k0
	kmovd	%k0, %ecx

	/* Shift out zero CHAR matches that are before the begining of
	   src (rdi).  */
# ifdef USE_AS_WCSRCHR
	movl	%edi, %esi
	andl	$(VEC_SIZE - 1), %esi
	shrl	$2, %esi
# endif
	shrxl	%SHIFT_REG, %ecx, %ecx

	testl	%ecx, %ecx
	jz	L(page_cross_continue)

	/* Found zero CHAR so need to test for search CHAR.  */
	VPCMP	$0, %YMMMATCH, %YMM1, %k1
	kmovd	%k1, %eax
	/* Shift out search CHAR matches that are before the begining of
	   src (rdi).  */
	shrxl	%SHIFT_REG, %eax, %eax

	/* Check if any search CHAR match in range.  */
	blsmskl	%ecx, %ecx
	andl	%ecx, %eax
	jz	L(ret3)
	bsrl	%eax, %eax
# ifdef USE_AS_WCSRCHR
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
# else
	addq	%rdi, %rax
# endif
L(ret3):
	ret

END(STRRCHR)
#endif
