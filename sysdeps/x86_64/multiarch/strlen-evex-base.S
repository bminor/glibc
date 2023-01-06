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

# ifdef USE_AS_WCSLEN
#  define VPCMPEQ	vpcmpeqd
#  define VPTESTN	vptestnmd
#  define VPMINU	vpminud
#  define CHAR_SIZE	4
# else
#  define VPCMPEQ	vpcmpeqb
#  define VPTESTN	vptestnmb
#  define VPMINU	vpminub
#  define CHAR_SIZE	1
# endif

# define PAGE_SIZE	4096
# define CHAR_PER_VEC	(VEC_SIZE / CHAR_SIZE)

	.section SECTION(.text),"ax",@progbits
/* Aligning entry point to 64 byte, provides better performance for
   one vector length string.  */
ENTRY_P2ALIGN (STRLEN, 6)
# ifdef USE_AS_STRNLEN
	/* Check zero length.  */
	test	%RSI_LP, %RSI_LP
	jz	L(ret_max)
#  ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%esi, %esi
#  endif
# endif

	movl	%edi, %eax
	vpxorq	%VMM_128(0), %VMM_128(0), %VMM_128(0)
	sall	$20, %eax
	cmpl	$((PAGE_SIZE - VEC_SIZE) << 20), %eax
	ja	L(page_cross)

	/* Compare [w]char for null, mask bit will be set for match.  */
	VPCMPEQ	(%rdi), %VMM(0), %k0
# ifdef USE_AS_STRNLEN
	KMOV	%k0, %VRCX
	/* Store max length in rax.  */
	mov	%rsi, %rax
	/* If rcx is 0, rax will have max length.  We can not use VRCX
	   and VRAX here for evex256 because, upper 32 bits may be
	   undefined for ecx and eax.  */
	bsfq	%rcx, %rax
	cmp	$CHAR_PER_VEC, %rax
	ja	L(align_more)
	cmpq	%rax, %rsi
	cmovb	%esi, %eax
# else
	KMOV	%k0, %VRAX
	test	%VRAX, %VRAX
	jz	L(align_more)
	bsf	%VRAX, %VRAX
# endif
	ret

	/* At this point vector max length reached.  */
# ifdef USE_AS_STRNLEN
	.p2align 4,,3
L(ret_max):
	movq	%rsi, %rax
	ret
# endif

L(align_more):
	mov	%rdi, %rax
	/* Align rax to VEC_SIZE.  */
	andq	$-VEC_SIZE, %rax
# ifdef USE_AS_STRNLEN
	movq	%rdi, %rdx
	subq	%rax, %rdx
#  ifdef USE_AS_WCSLEN
	shr	$2, %VRDX
#  endif
	/* At this point rdx contains [w]chars already compared.  */
	leaq	-CHAR_PER_VEC(%rsi, %rdx), %rdx
	/* At this point rdx contains number of w[char] needs to go.
	   Now onwards rdx will keep decrementing with each compare.  */
# endif

	/* Loop unroll 4 times for 4 vector loop.  */
	VPCMPEQ	VEC_SIZE(%rax), %VMM(0), %k0
	subq	$-VEC_SIZE, %rax
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x1)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMPEQ	VEC_SIZE(%rax), %VMM(0), %k0
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x2)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMPEQ	(VEC_SIZE * 2)(%rax), %VMM(0), %k0
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x3)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMPEQ	(VEC_SIZE * 3)(%rax), %VMM(0), %k0
	KMOV	%k0, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x4)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
	/* Save pointer before 4 x VEC_SIZE alignment.  */
	movq	%rax, %rcx
# endif

	/* Align address to VEC_SIZE * 4 for loop.  */
	andq	$-(VEC_SIZE * 4), %rax

# ifdef USE_AS_STRNLEN
	subq	%rax, %rcx
#  ifdef USE_AS_WCSLEN
	shr	$2, %VRCX
#  endif
	/* rcx contains number of [w]char will be recompared due to
	   alignment fixes.  rdx must be incremented by rcx to offset
	   alignment adjustment.  */
	addq	%rcx, %rdx
	/* Need jump as we don't want to add/subtract rdx for first
	   iteration of 4 x VEC_SIZE aligned loop.  */
# endif

	.p2align 4,,11
L(loop):
	/* VPMINU and VPCMP combination provide better performance as
	   compared to alternative combinations.  */
	VMOVA	(VEC_SIZE * 4)(%rax), %VMM(1)
	VPMINU	(VEC_SIZE * 5)(%rax), %VMM(1), %VMM(2)
	VMOVA	(VEC_SIZE * 6)(%rax), %VMM(3)
	VPMINU	(VEC_SIZE * 7)(%rax), %VMM(3), %VMM(4)

	VPTESTN	%VMM(2), %VMM(2), %k0
	VPTESTN	%VMM(4), %VMM(4), %k1

	subq	$-(VEC_SIZE * 4), %rax
	KORTEST	%k0, %k1

# ifndef USE_AS_STRNLEN
	jz      L(loop)
# else
	jnz	L(loopend)
	subq	$(CHAR_PER_VEC * 4), %rdx
	ja	L(loop)
	mov	%rsi, %rax
	ret
# endif

L(loopend):

	VPTESTN	%VMM(1), %VMM(1), %k2
	KMOV	%k2, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x1)

	KMOV	%k0, %VRCX
	/* At this point, if k0 is non zero, null char must be in the
	   second vector.  */
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x2)

	VPTESTN	%VMM(3), %VMM(3), %k3
	KMOV	%k3, %VRCX
	test	%VRCX, %VRCX
	jnz	L(ret_vec_x3)
	/* At this point null [w]char must be in the fourth vector so no
	   need to check.  */
	KMOV	%k1, %VRCX

	/* Fourth, third, second vector terminating are pretty much
	   same, implemented this way to avoid branching and reuse code
	   from pre loop exit condition.  */
L(ret_vec_x4):
	bsf	%VRCX, %VRCX
	subq	%rdi, %rax
# ifdef USE_AS_WCSLEN
	subq	$-(VEC_SIZE * 3), %rax
	shrq	$2, %rax
	addq	%rcx, %rax
# else
	leaq	(VEC_SIZE * 3)(%rcx, %rax), %rax
# endif
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%rsi, %rax
# endif
	ret

L(ret_vec_x3):
	bsf	%VRCX, %VRCX
	subq	%rdi, %rax
# ifdef USE_AS_WCSLEN
	subq	$-(VEC_SIZE * 2), %rax
	shrq	$2, %rax
	addq	%rcx, %rax
# else
	leaq	(VEC_SIZE * 2)(%rcx, %rax), %rax
# endif
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%rsi, %rax
# endif
	ret

L(ret_vec_x2):
	subq	$-VEC_SIZE, %rax
L(ret_vec_x1):
	bsf	%VRCX, %VRCX
	subq	%rdi, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	addq	%rcx, %rax
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%rsi, %rax
# endif
	ret

L(page_cross):
	mov	%rdi, %rax
	movl	%edi, %ecx
	andl	$(VEC_SIZE - 1), %ecx
# ifdef USE_AS_WCSLEN
	sarl	$2, %ecx
# endif
	/* ecx contains number of w[char] to be skipped as a result
	   of address alignment.  */
	andq	$-VEC_SIZE, %rax
	VPCMPEQ	(%rax), %VMM(0), %k0
	KMOV	%k0, %VRDX
	/* Ignore number of character for alignment adjustment.  */
	shr	%cl, %VRDX
# ifdef USE_AS_STRNLEN
	jnz	L(page_cross_end)
	movl    $CHAR_PER_VEC, %eax
	sub     %ecx, %eax
	cmp	%rax, %rsi
	ja	L(align_more)
# else
	jz	L(align_more)
# endif

L(page_cross_end):
	bsf	%VRDX, %VRAX
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%esi, %eax
# endif
	ret

END (STRLEN)
#endif
