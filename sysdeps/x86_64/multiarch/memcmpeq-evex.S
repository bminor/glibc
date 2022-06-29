/* __memcmpeq optimized with EVEX.
   Copyright (C) 2017-2022 Free Software Foundation, Inc.
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

/* __memcmpeq is implemented as:
   1. Use ymm vector compares when possible. The only case where
      vector compares is not possible for when size < VEC_SIZE
      and loading from either s1 or s2 would cause a page cross.
   2. Use xmm vector compare when size >= 8 bytes.
   3. Optimistically compare up to first 4 * VEC_SIZE one at a
      to check for early mismatches. Only do this if its guranteed the
      work is not wasted.
   4. If size is 8 * VEC_SIZE or less, unroll the loop.
   5. Compare 4 * VEC_SIZE at a time with the aligned first memory
      area.
   6. Use 2 vector compares when size is 2 * VEC_SIZE or less.
   7. Use 4 vector compares when size is 4 * VEC_SIZE or less.
   8. Use 8 vector compares when size is 8 * VEC_SIZE or less.  */

# include <sysdep.h>

# ifndef MEMCMPEQ
#  define MEMCMPEQ	__memcmpeq_evex
# endif

# define VMOVU_MASK	vmovdqu8
# define VMOVU	vmovdqu64
# define VPCMP	vpcmpub
# define VPTEST	vptestmb

# define VEC_SIZE	32
# define PAGE_SIZE	4096

# define YMM0		ymm16
# define YMM1		ymm17
# define YMM2		ymm18
# define YMM3		ymm19
# define YMM4		ymm20
# define YMM5		ymm21
# define YMM6		ymm22


	.section .text.evex, "ax", @progbits
ENTRY_P2ALIGN (MEMCMPEQ, 6)
# ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%edx, %edx
# endif
	cmp	$VEC_SIZE, %RDX_LP
	/* Fall through for [0, VEC_SIZE] as its the hottest.  */
	ja	L(more_1x_vec)

	/* Create mask of bytes that are guranteed to be valid because
	   of length (edx). Using masked movs allows us to skip checks for
	   page crosses/zero size.  */
	movl	$-1, %ecx
	bzhil	%edx, %ecx, %ecx
	kmovd	%ecx, %k2

	/* Use masked loads as VEC_SIZE could page cross where length
	   (edx) would not.  */
	VMOVU_MASK (%rsi), %YMM2{%k2}
	VPCMP	$4,(%rdi), %YMM2, %k1{%k2}
	kmovd	%k1, %eax
	ret


L(last_1x_vec):
	VMOVU	-(VEC_SIZE * 1)(%rsi, %rdx), %YMM1
	VPCMP	$4, -(VEC_SIZE * 1)(%rdi, %rdx), %YMM1, %k1
	kmovd	%k1, %eax
L(return_neq0):
	ret



	.p2align 4
L(more_1x_vec):
	/* From VEC + 1 to 2 * VEC.  */
	VMOVU	(%rsi), %YMM1
	/* Use compare not equals to directly check for mismatch.  */
	VPCMP	$4,(%rdi), %YMM1, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(return_neq0)

	cmpq	$(VEC_SIZE * 2), %rdx
	jbe	L(last_1x_vec)

	/* Check second VEC no matter what.  */
	VMOVU	VEC_SIZE(%rsi), %YMM2
	VPCMP	$4, VEC_SIZE(%rdi), %YMM2, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(return_neq0)

	/* Less than 4 * VEC.  */
	cmpq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_2x_vec)

	/* Check third and fourth VEC no matter what.  */
	VMOVU	(VEC_SIZE * 2)(%rsi), %YMM3
	VPCMP	$4,(VEC_SIZE * 2)(%rdi), %YMM3, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(return_neq0)

	VMOVU	(VEC_SIZE * 3)(%rsi), %YMM4
	VPCMP	$4,(VEC_SIZE * 3)(%rdi), %YMM4, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(return_neq0)

	/* Go to 4x VEC loop.  */
	cmpq	$(VEC_SIZE * 8), %rdx
	ja	L(more_8x_vec)

	/* Handle remainder of size = 4 * VEC + 1 to 8 * VEC without any
	   branches.  */

	VMOVU	-(VEC_SIZE * 4)(%rsi, %rdx), %YMM1
	VMOVU	-(VEC_SIZE * 3)(%rsi, %rdx), %YMM2
	addq	%rdx, %rdi

	/* Wait to load from s1 until addressed adjust due to
	   unlamination.  */

	/* vpxor will be all 0s if s1 and s2 are equal. Otherwise it
	   will have some 1s.  */
	vpxorq	-(VEC_SIZE * 4)(%rdi), %YMM1, %YMM1
	/* Ternary logic to xor -(VEC_SIZE * 3)(%rdi) with YMM2 while
	   oring with YMM1. Result is stored in YMM1.  */
	vpternlogd $0xde, -(VEC_SIZE * 3)(%rdi), %YMM1, %YMM2

	VMOVU	-(VEC_SIZE * 2)(%rsi, %rdx), %YMM3
	vpxorq	-(VEC_SIZE * 2)(%rdi), %YMM3, %YMM3
	/* Or together YMM1, YMM2, and YMM3 into YMM3.  */
	VMOVU	-(VEC_SIZE)(%rsi, %rdx), %YMM4
	vpxorq	-(VEC_SIZE)(%rdi), %YMM4, %YMM4

	/* Or together YMM2, YMM3, and YMM4 into YMM4.  */
	vpternlogd $0xfe, %YMM2, %YMM3, %YMM4

	/* Compare YMM4 with 0. If any 1s s1 and s2 don't match.  */
	VPTEST	%YMM4, %YMM4, %k1
	kmovd	%k1, %eax
	ret

	.p2align 4
L(more_8x_vec):
	/* Set end of s1 in rdx.  */
	leaq	-(VEC_SIZE * 4)(%rdi, %rdx), %rdx
	/* rsi stores s2 - s1. This allows loop to only update one
	   pointer.  */
	subq	%rdi, %rsi
	/* Align s1 pointer.  */
	andq	$-VEC_SIZE, %rdi
	/* Adjust because first 4x vec where check already.  */
	subq	$-(VEC_SIZE * 4), %rdi
	.p2align 4
L(loop_4x_vec):
	VMOVU	(%rsi, %rdi), %YMM1
	vpxorq	(%rdi), %YMM1, %YMM1

	VMOVU	VEC_SIZE(%rsi, %rdi), %YMM2
	vpternlogd $0xde,(VEC_SIZE)(%rdi), %YMM1, %YMM2

	VMOVU	(VEC_SIZE * 2)(%rsi, %rdi), %YMM3
	vpxorq	(VEC_SIZE * 2)(%rdi), %YMM3, %YMM3

	VMOVU	(VEC_SIZE * 3)(%rsi, %rdi), %YMM4
	vpxorq	(VEC_SIZE * 3)(%rdi), %YMM4, %YMM4

	vpternlogd $0xfe, %YMM2, %YMM3, %YMM4
	VPTEST	%YMM4, %YMM4, %k1
	kmovd	%k1, %eax
	testl	%eax, %eax
	jnz	L(return_neq2)
	subq	$-(VEC_SIZE * 4), %rdi
	cmpq	%rdx, %rdi
	jb	L(loop_4x_vec)

	subq	%rdx, %rdi
	VMOVU	(VEC_SIZE * 3)(%rsi, %rdx), %YMM4
	vpxorq	(VEC_SIZE * 3)(%rdx), %YMM4, %YMM4
	/* rdi has 4 * VEC_SIZE - remaining length.  */
	cmpl	$(VEC_SIZE * 3), %edi
	jae	L(8x_last_1x_vec)
	/* Load regardless of branch.  */
	VMOVU	(VEC_SIZE * 2)(%rsi, %rdx), %YMM3
	/* Ternary logic to xor (VEC_SIZE * 2)(%rdx) with YMM3 while
	   oring with YMM4. Result is stored in YMM4.  */
	vpternlogd $0xf6,(VEC_SIZE * 2)(%rdx), %YMM3, %YMM4
	cmpl	$(VEC_SIZE * 2), %edi
	jae	L(8x_last_2x_vec)

	VMOVU	VEC_SIZE(%rsi, %rdx), %YMM2
	vpxorq	VEC_SIZE(%rdx), %YMM2, %YMM2

	VMOVU	(%rsi, %rdx), %YMM1
	vpxorq	(%rdx), %YMM1, %YMM1

	vpternlogd $0xfe, %YMM1, %YMM2, %YMM4
L(8x_last_1x_vec):
L(8x_last_2x_vec):
	VPTEST	%YMM4, %YMM4, %k1
	kmovd	%k1, %eax
L(return_neq2):
	ret

	.p2align 4,, 8
L(last_2x_vec):
	VMOVU	-(VEC_SIZE * 2)(%rsi, %rdx), %YMM1
	vpxorq	-(VEC_SIZE * 2)(%rdi, %rdx), %YMM1, %YMM1
	VMOVU	-(VEC_SIZE * 1)(%rsi, %rdx), %YMM2
	vpternlogd $0xde, -(VEC_SIZE * 1)(%rdi, %rdx), %YMM1, %YMM2
	VPTEST	%YMM2, %YMM2, %k1
	kmovd	%k1, %eax
	ret

    /* 1 Bytes from next cache line. */
END (MEMCMPEQ)
#endif
