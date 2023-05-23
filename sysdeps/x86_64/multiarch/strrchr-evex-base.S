/* Placeholder function, not used by any processor at the moment.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.
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

/* UNUSED. Exists purely as reference implementation.  */

#include <isa-level.h>

#if ISA_SHOULD_BUILD (4)

# include <sysdep.h>

# ifdef USE_AS_WCSRCHR
#  define CHAR_SIZE	4
#  define VPBROADCAST   vpbroadcastd
#  define VPCMPEQ	vpcmpeqd
#  define VPMINU	vpminud
#  define VPTESTN	vptestnmd
# else
#  define CHAR_SIZE	1
#  define VPBROADCAST   vpbroadcastb
#  define VPCMPEQ	vpcmpeqb
#  define VPMINU	vpminub
#  define VPTESTN	vptestnmb
# endif

# define PAGE_SIZE	4096
# define CHAR_PER_VEC	(VEC_SIZE / CHAR_SIZE)

	.section SECTION(.text), "ax", @progbits
/* Aligning entry point to 64 byte, provides better performance for
   one vector length string.  */
ENTRY_P2ALIGN (STRRCHR, 6)

	/* Broadcast CHAR to VMM(0).  */
	VPBROADCAST %esi, %VMM(0)
	movl	%edi, %eax
	sall	$20, %eax
	cmpl	$((PAGE_SIZE - VEC_SIZE) << 20), %eax
	ja	L(page_cross)

L(page_cross_continue):
	/* Compare [w]char for null, mask bit will be set for match.  */
	VMOVU	(%rdi), %VMM(1)

	VPTESTN	%VMM(1), %VMM(1), %k1
	KMOV	%k1, %VRCX
	test	%VRCX, %VRCX
	jz	L(align_more)

	VPCMPEQ	%VMM(1), %VMM(0), %k0
	KMOV	%k0, %VRAX
	BLSMSK	%VRCX, %VRCX
	and	%VRCX, %VRAX
	jz	L(ret)

	BSR	%VRAX, %VRAX
# ifdef USE_AS_WCSRCHR
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
# else
	add	%rdi, %rax
# endif
L(ret):
	ret

L(vector_x2_end):
	VPCMPEQ	%VMM(2), %VMM(0), %k2
	KMOV	%k2, %VRAX
	BLSMSK	%VRCX, %VRCX
	and	%VRCX, %VRAX
	jz	L(vector_x1_ret)

	BSR	%VRAX, %VRAX
	leaq	(VEC_SIZE)(%rdi, %rax, CHAR_SIZE), %rax
	ret

	/* Check the first vector at very last to look for match.  */
L(vector_x1_ret):
	VPCMPEQ %VMM(1), %VMM(0), %k2
	KMOV	%k2, %VRAX
	test	%VRAX, %VRAX
	jz	L(ret)

	BSR	%VRAX, %VRAX
# ifdef USE_AS_WCSRCHR
	leaq	(%rsi, %rax, CHAR_SIZE), %rax
# else
	add	%rsi, %rax
# endif
	ret

L(align_more):
	/* Zero r8 to store match result.  */
	xorl	%r8d, %r8d
	/* Save pointer of first vector, in case if no match found.  */
	movq	%rdi, %rsi
	/* Align pointer to vector size.  */
	andq	$-VEC_SIZE, %rdi
	/* Loop unroll for 2 vector loop.  */
	VMOVA	(VEC_SIZE)(%rdi), %VMM(2)
	VPTESTN	%VMM(2), %VMM(2), %k0
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(vector_x2_end)

	/* Save pointer of second vector, in case if no match
	   found.  */
	movq	%rdi, %r9
	/* Align address to VEC_SIZE * 2 for loop.  */
	andq	$-(VEC_SIZE * 2), %rdi

	.p2align 4,,11
L(loop):
	/* 2 vector loop, as it provide better performance as compared
	   to 4 vector loop.  */
	VMOVA	(VEC_SIZE * 2)(%rdi), %VMM(3)
	VMOVA	(VEC_SIZE * 3)(%rdi), %VMM(4)
	VPCMPEQ	%VMM(3), %VMM(0), %k1
	VPCMPEQ	%VMM(4), %VMM(0), %k2
	VPMINU	%VMM(3), %VMM(4), %VMM(5)
	VPTESTN	%VMM(5), %VMM(5), %k0
	KOR	%k1, %k2, %k3
	subq	$-(VEC_SIZE * 2), %rdi
	/* If k0 and k3 zero, match and end of string not found.  */
	KORTEST	%k0, %k3
	jz	L(loop)

	/* If k0 is non zero, end of string found.  */
	KORTEST %k0, %k0
	jnz	L(endloop)

	lea	VEC_SIZE(%rdi), %r8
	/* A match found, it need to be stored in r8 before loop
	   continue.  */
	/* Check second vector first.  */
	KMOV	%k2, %VRDX
	test	%VRDX, %VRDX
	jnz	L(loop_vec_x2_match)

	KMOV	%k1, %VRDX
	/* Match is in first vector, rdi offset need to be subtracted
	  by VEC_SIZE.  */
	sub	$VEC_SIZE, %r8

	/* If second vector doesn't have match, first vector must
	   have match.  */
L(loop_vec_x2_match):
	BSR	%VRDX, %VRDX
# ifdef USE_AS_WCSRCHR
	sal	$2, %rdx
# endif
	add	%rdx, %r8
	jmp	L(loop)

L(endloop):
	/* Check if string end in first loop vector.  */
	VPTESTN	%VMM(3), %VMM(3), %k0
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(loop_vector_x1_end)

	/* Check if it has match in first loop vector.  */
	KMOV	%k1, %VRAX
	test	%VRAX, %VRAX
	jz	L(loop_vector_x2_end)

	BSR	%VRAX, %VRAX
	leaq	(%rdi, %rax, CHAR_SIZE), %r8

	/* String must end in second loop vector.  */
L(loop_vector_x2_end):
	VPTESTN	%VMM(4), %VMM(4), %k0
	KMOV	%k0, %VRCX
	KMOV	%k2, %VRAX
	BLSMSK	%VRCX, %VRCX
	/* Check if it has match in second loop vector.  */
	and	%VRCX, %VRAX
	jz	L(check_last_match)

	BSR	%VRAX, %VRAX
	leaq	(VEC_SIZE)(%rdi, %rax, CHAR_SIZE), %rax
	ret

	/* String end in first loop vector.  */
L(loop_vector_x1_end):
	KMOV	%k1, %VRAX
	BLSMSK	%VRCX, %VRCX
	/* Check if it has match in second loop vector.  */
	and	%VRCX, %VRAX
	jz	L(check_last_match)

	BSR	%VRAX, %VRAX
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
	ret

	/* No match in first and second loop vector.  */
L(check_last_match):
	/* Check if any match recorded in r8.  */
	test	%r8, %r8
	jz	L(vector_x2_ret)
	movq	%r8, %rax
	ret

	/* No match recorded in r8. Check the second saved vector
	   in beginning.  */
L(vector_x2_ret):
	VPCMPEQ %VMM(2), %VMM(0), %k2
	KMOV	%k2, %VRAX
	test	%VRAX, %VRAX
	jz	L(vector_x1_ret)

	/* Match found in the second saved vector.  */
	BSR	%VRAX, %VRAX
	leaq	(VEC_SIZE)(%r9, %rax, CHAR_SIZE), %rax
	ret

L(page_cross):
	mov	%rdi, %rax
	movl	%edi, %ecx

# ifdef USE_AS_WCSRCHR
	/* Calculate number of compare result bits to be skipped for
	   wide string alignment adjustment.  */
	andl	$(VEC_SIZE - 1), %ecx
	sarl	$2, %ecx
# endif
	/* ecx contains number of w[char] to be skipped as a result
	   of address alignment.  */
	andq    $-VEC_SIZE, %rax
	VMOVA	(%rax), %VMM(1)
	VPTESTN	%VMM(1), %VMM(1), %k1
	KMOV	%k1, %VRAX
	SHR     %cl, %VRAX
	jz	L(page_cross_continue)
	VPCMPEQ	%VMM(1), %VMM(0), %k0
	KMOV	%k0, %VRDX
	SHR     %cl, %VRDX
	BLSMSK	%VRAX, %VRAX
	and	%VRDX, %VRAX
	jz	L(ret)
	BSR	%VRAX, %VRAX
# ifdef USE_AS_WCSRCHR
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
# else
	add	%rdi, %rax
# endif

	ret
END (STRRCHR)
#endif
