/* strrchr/wcsrchr optimized with 256-bit EVEX instructions.
   Copyright (C) 2021-2023 Free Software Foundation, Inc.
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

# include "x86-evex256-vecs.h"

# ifdef USE_AS_WCSRCHR
#  define SHIFT_REG	rsi
#  define kunpck_2x	kunpckbw
#  define kmov_2x	kmovd
#  define maskz_2x	ecx
#  define maskm_2x	eax
#  define CHAR_SIZE	4
#  define VPMIN	vpminud
#  define VPTESTN	vptestnmd
#  define VPTEST	vptestmd
#  define VPBROADCAST	vpbroadcastd
#  define VPCMPEQ	vpcmpeqd
#  define VPCMP	vpcmpd

#  define USE_WIDE_CHAR
# else
#  define SHIFT_REG	rdi
#  define kunpck_2x	kunpckdq
#  define kmov_2x	kmovq
#  define maskz_2x	rcx
#  define maskm_2x	rax

#  define CHAR_SIZE	1
#  define VPMIN	vpminub
#  define VPTESTN	vptestnmb
#  define VPTEST	vptestmb
#  define VPBROADCAST	vpbroadcastb
#  define VPCMPEQ	vpcmpeqb
#  define VPCMP	vpcmpb
# endif

# include "reg-macros.h"

# define VMATCH	VMM(0)
# define CHAR_PER_VEC	(VEC_SIZE / CHAR_SIZE)
# define PAGE_SIZE	4096

	.section SECTION(.text), "ax", @progbits
ENTRY_P2ALIGN(STRRCHR, 6)
	movl	%edi, %eax
	/* Broadcast CHAR to VMATCH.  */
	VPBROADCAST %esi, %VMATCH

	andl	$(PAGE_SIZE - 1), %eax
	cmpl	$(PAGE_SIZE - VEC_SIZE), %eax
	jg	L(cross_page_boundary)
L(page_cross_continue):
	VMOVU	(%rdi), %VMM(1)
	/* k0 has a 1 for each zero CHAR in VEC(1).  */
	VPTESTN	%VMM(1), %VMM(1), %k0
	KMOV	%k0, %VRSI
	test	%VRSI, %VRSI
	jz	L(aligned_more)
	/* fallthrough: zero CHAR in first VEC.  */
	/* K1 has a 1 for each search CHAR match in VEC(1).  */
	VPCMPEQ	%VMATCH, %VMM(1), %k1
	KMOV	%k1, %VRAX
	/* Build mask up until first zero CHAR (used to mask of
	   potential search CHAR matches past the end of the string).
	 */
	blsmsk	%VRSI, %VRSI
	and	%VRSI, %VRAX
	jz	L(ret0)
	/* Get last match (the `and` removed any out of bounds matches).
	 */
	bsr	%VRAX, %VRAX
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
	VPCMPEQ	%VMATCH, %VMM(2), %k1
	KMOV	%k1, %VRAX
	blsmsk	%VRCX, %VRCX
	/* eax non-zero if search CHAR in range.  */
	and	%VRCX, %VRAX
	jnz	L(first_vec_x1_return)

	/* fallthrough: no match in VEC(2) then need to check for
	   earlier matches (in VEC(1)).  */
	.p2align 4,, 4
L(first_vec_x0_test):
	VPCMPEQ	%VMATCH, %VMM(1), %k1
	KMOV	%k1, %VRAX
	test	%VRAX, %VRAX
	jz	L(ret1)
	bsr	%VRAX, %VRAX
# ifdef USE_AS_WCSRCHR
	leaq	(%rsi, %rax, CHAR_SIZE), %rax
# else
	addq	%rsi, %rax
# endif
L(ret1):
	ret

	.p2align 4,, 10
L(first_vec_x1_or_x2):
	VPCMPEQ	%VMM(3), %VMATCH, %k3
	VPCMPEQ	%VMM(2), %VMATCH, %k2
	/* K2 and K3 have 1 for any search CHAR match. Test if any
	   matches between either of them. Otherwise check VEC(1).  */
	KORTEST %k2, %k3
	jz	L(first_vec_x0_test)

	/* Guaranteed that VEC(2) and VEC(3) are within range so merge
	   the two bitmasks then get last result.  */
	kunpck_2x %k2, %k3, %k3
	kmov_2x	%k3, %maskm_2x
	bsr	%maskm_2x, %maskm_2x
	leaq	(VEC_SIZE * 1)(%r8, %rax, CHAR_SIZE), %rax
	ret

	.p2align 4,, 7
L(first_vec_x3):
	VPCMPEQ	%VMATCH, %VMM(4), %k1
	KMOV	%k1, %VRAX
	blsmsk	%VRCX, %VRCX
	/* If no search CHAR match in range check VEC(1)/VEC(2)/VEC(3).
	 */
	and	%VRCX, %VRAX
	jz	L(first_vec_x1_or_x2)
	bsr	%VRAX, %VRAX
	leaq	(VEC_SIZE * 3)(%rdi, %rax, CHAR_SIZE), %rax
	ret


	.p2align 4,, 6
L(first_vec_x0_x1_test):
	VPCMPEQ	%VMATCH, %VMM(2), %k1
	KMOV	%k1, %VRAX
	/* Check VEC(2) for last match first. If no match try VEC(1).
	 */
	test	%VRAX, %VRAX
	jz	L(first_vec_x0_test)
	.p2align 4,, 4
L(first_vec_x1_return):
	bsr	%VRAX, %VRAX
	leaq	(VEC_SIZE)(%rdi, %rax, CHAR_SIZE), %rax
	ret


	.p2align 4,, 10
L(first_vec_x2):
	VPCMPEQ	%VMATCH, %VMM(3), %k1
	KMOV	%k1, %VRAX
	blsmsk	%VRCX, %VRCX
	/* Check VEC(3) for last match first. If no match try
	   VEC(2)/VEC(1).  */
	and	%VRCX, %VRAX
	jz	L(first_vec_x0_x1_test)
	bsr	%VRAX, %VRAX
	leaq	(VEC_SIZE * 2)(%rdi, %rax, CHAR_SIZE), %rax
	ret


	.p2align 4,, 12
L(aligned_more):
	/* Need to keep original pointer in case VEC(1) has last match.
	 */
	movq	%rdi, %rsi
	andq	$-VEC_SIZE, %rdi

	VMOVU	VEC_SIZE(%rdi), %VMM(2)
	VPTESTN	%VMM(2), %VMM(2), %k0
	KMOV	%k0, %VRCX

	test	%VRCX, %VRCX
	jnz	L(first_vec_x1)

	VMOVU	(VEC_SIZE * 2)(%rdi), %VMM(3)
	VPTESTN	%VMM(3), %VMM(3), %k0
	KMOV	%k0, %VRCX

	test	%VRCX, %VRCX
	jnz	L(first_vec_x2)

	VMOVU	(VEC_SIZE * 3)(%rdi), %VMM(4)
	VPTESTN	%VMM(4), %VMM(4), %k0
	KMOV	%k0, %VRCX
	movq	%rdi, %r8
	test	%VRCX, %VRCX
	jnz	L(first_vec_x3)

	andq	$-(VEC_SIZE * 2), %rdi
	.p2align 4,, 10
L(first_aligned_loop):
	/* Preserve VEC(1), VEC(2), VEC(3), and VEC(4) until we can
	   guarantee they don't store a match.  */
	VMOVA	(VEC_SIZE * 4)(%rdi), %VMM(5)
	VMOVA	(VEC_SIZE * 5)(%rdi), %VMM(6)

	VPCMPEQ	%VMM(5), %VMATCH, %k2
	vpxord	%VMM(6), %VMATCH, %VMM(7)

	VPMIN	%VMM(5), %VMM(6), %VMM(8)
	VPMIN	%VMM(8), %VMM(7), %VMM(7)

	VPTESTN	%VMM(7), %VMM(7), %k1
	subq	$(VEC_SIZE * -2), %rdi
	KORTEST %k1, %k2
	jz	L(first_aligned_loop)

	VPCMPEQ	%VMM(6), %VMATCH, %k3
	VPTESTN	%VMM(8), %VMM(8), %k1

	/* If k1 is zero, then we found a CHAR match but no null-term.
	   We can now safely throw out VEC1-4.  */
	KTEST	%k1, %k1
	jz	L(second_aligned_loop_prep)

	KORTEST %k2, %k3
	jnz	L(return_first_aligned_loop)


	.p2align 4,, 6
L(first_vec_x1_or_x2_or_x3):
	VPCMPEQ	%VMM(4), %VMATCH, %k4
	KMOV	%k4, %VRAX
	bsr	%VRAX, %VRAX
	jz	L(first_vec_x1_or_x2)
	leaq	(VEC_SIZE * 3)(%r8, %rax, CHAR_SIZE), %rax
	ret


	.p2align 4,, 8
L(return_first_aligned_loop):
	VPTESTN	%VMM(5), %VMM(5), %k0

	/* Combined results from VEC5/6.  */
	kunpck_2x %k0, %k1, %k0
	kmov_2x	%k0, %maskz_2x

	blsmsk	%maskz_2x, %maskz_2x
	kunpck_2x %k2, %k3, %k3
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
	/* Ideally we would safe k2/k3 but `kmov/kunpck` take uops on
	   port0 and have noticeable overhead in the loop.  */
	VMOVA	%VMM(5), %VMM(7)
	VMOVA	%VMM(6), %VMM(8)
	.p2align 4
L(second_aligned_loop):
	VMOVU	(VEC_SIZE * 4)(%rdi), %VMM(5)
	VMOVU	(VEC_SIZE * 5)(%rdi), %VMM(6)
	VPCMPEQ	%VMM(5), %VMATCH, %k2
	vpxord	%VMM(6), %VMATCH, %VMM(3)

	VPMIN	%VMM(5), %VMM(6), %VMM(4)
	VPMIN	%VMM(3), %VMM(4), %VMM(3)

	VPTESTN	%VMM(3), %VMM(3), %k1
	subq	$(VEC_SIZE * -2), %rdi
	KORTEST %k1, %k2
	jz	L(second_aligned_loop)
	VPCMPEQ	%VMM(6), %VMATCH, %k3
	VPTESTN	%VMM(4), %VMM(4), %k1
	KTEST	%k1, %k1
	jz	L(second_aligned_loop_set_furthest_match)

	/* branch here because we know we have a match in VEC7/8 but
	   might not in VEC5/6 so the latter is expected to be less
	   likely.  */
	KORTEST %k2, %k3
	jnz	L(return_new_match)

L(return_old_match):
	VPCMPEQ	%VMM(8), %VMATCH, %k0
	KMOV	%k0, %VRCX
	bsr	%VRCX, %VRCX
	jnz	L(return_old_match_ret)

	VPCMPEQ	%VMM(7), %VMATCH, %k0
	KMOV	%k0, %VRCX
	bsr	%VRCX, %VRCX
	subq	$VEC_SIZE, %rsi
L(return_old_match_ret):
	leaq	(VEC_SIZE * 3)(%rsi, %rcx, CHAR_SIZE), %rax
	ret

	.p2align 4,, 10
L(return_new_match):
	VPTESTN	%VMM(5), %VMM(5), %k0

	/* Combined results from VEC5/6.  */
	kunpck_2x %k0, %k1, %k0
	kmov_2x	%k0, %maskz_2x

	blsmsk	%maskz_2x, %maskz_2x
	kunpck_2x %k2, %k3, %k3
	kmov_2x	%k3, %maskm_2x

	/* Match at end was out-of-bounds so use last known match.  */
	and	%maskz_2x, %maskm_2x
	jz	L(return_old_match)

	bsr	%maskm_2x, %maskm_2x
	leaq	(VEC_SIZE * 2)(%rdi, %rax, CHAR_SIZE), %rax
	ret

L(cross_page_boundary):
	/* eax contains all the page offset bits of src (rdi). `xor rdi,
	   rax` sets pointer will all page offset bits cleared so
	   offset of (PAGE_SIZE - VEC_SIZE) will get last aligned VEC
	   before page cross (guaranteed to be safe to read). Doing this
	   as opposed to `movq %rdi, %rax; andq $-VEC_SIZE, %rax` saves
	   a bit of code size.  */
	xorq	%rdi, %rax
	VMOVU	(PAGE_SIZE - VEC_SIZE)(%rax), %VMM(1)
	VPTESTN	%VMM(1), %VMM(1), %k0
	KMOV	%k0, %VRCX

	/* Shift out zero CHAR matches that are before the beginning of
	   src (rdi).  */
# ifdef USE_AS_WCSRCHR
	movl	%edi, %esi
	andl	$(VEC_SIZE - 1), %esi
	shrl	$2, %esi
# endif
	shrx	%VGPR(SHIFT_REG), %VRCX, %VRCX

	test	%VRCX, %VRCX
	jz	L(page_cross_continue)

	/* Found zero CHAR so need to test for search CHAR.  */
	VPCMP	$0, %VMATCH, %VMM(1), %k1
	KMOV	%k1, %VRAX
	/* Shift out search CHAR matches that are before the beginning of
	   src (rdi).  */
	shrx	%VGPR(SHIFT_REG), %VRAX, %VRAX

	/* Check if any search CHAR match in range.  */
	blsmsk	%VRCX, %VRCX
	and	%VRCX, %VRAX
	jz	L(ret3)
	bsr	%VRAX, %VRAX
# ifdef USE_AS_WCSRCHR
	leaq	(%rdi, %rax, CHAR_SIZE), %rax
# else
	addq	%rdi, %rax
# endif
L(ret3):
	ret
END(STRRCHR)
#endif
