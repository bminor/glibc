/* Placeholder function, not used by any processor at the moment.
   Copyright (C) 2022 Free Software Foundation, Inc.
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
#  define VPCMP		vpcmpd
#  define VPTESTN	vptestnmd
#  define VPMINU	vpminud
#  define CHAR_SIZE	4
# else
#  define VPCMP		vpcmpb
#  define VPTESTN	vptestnmb
#  define VPMINU	vpminub
#  define CHAR_SIZE	1
# endif

# define XMM0		xmm16
# define PAGE_SIZE	4096
# define CHAR_PER_VEC	(VEC_SIZE / CHAR_SIZE)

# if VEC_SIZE == 64
#  define KMOV		kmovq
#  define KORTEST	kortestq
#  define RAX		rax
#  define RCX		rcx
#  define RDX		rdx
#  define SHR		shrq
#  define TEXTSUFFIX	evex512
#  define VMM0		zmm16
#  define VMM1		zmm17
#  define VMM2		zmm18
#  define VMM3		zmm19
#  define VMM4		zmm20
#  define VMOVA		vmovdqa64
# elif VEC_SIZE == 32
/* Currently Unused.  */
#  define KMOV		kmovd
#  define KORTEST	kortestd
#  define RAX		eax
#  define RCX		ecx
#  define RDX		edx
#  define SHR		shrl
#  define TEXTSUFFIX	evex256
#  define VMM0		ymm16
#  define VMM1		ymm17
#  define VMM2		ymm18
#  define VMM3		ymm19
#  define VMM4		ymm20
#  define VMOVA		vmovdqa32
# endif

	.section .text.TEXTSUFFIX, "ax", @progbits
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
	vpxorq	%XMM0, %XMM0, %XMM0
	andl	$(PAGE_SIZE - 1), %eax
	cmpl	$(PAGE_SIZE - VEC_SIZE), %eax
	ja	L(page_cross)

	/* Compare [w]char for null, mask bit will be set for match.  */
	VPCMP	$0, (%rdi), %VMM0, %k0
	KMOV	%k0, %RAX
	test	%RAX, %RAX
	jz	L(align_more)

	bsf	%RAX, %RAX
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%rsi, %rax
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
	leaq	VEC_SIZE(%rdi), %rax
	/* Align rax to VEC_SIZE.  */
	andq	$-VEC_SIZE, %rax
# ifdef USE_AS_STRNLEN
	movq	%rax, %rdx
	subq	%rdi, %rdx
#  ifdef USE_AS_WCSLEN
	SHR	$2, %RDX
#  endif
	/* At this point rdx contains [w]chars already compared.  */
	subq	%rsi, %rdx
	jae	L(ret_max)
	negq	%rdx
	/* At this point rdx contains number of w[char] needs to go.
	   Now onwards rdx will keep decrementing with each compare.  */
# endif

	/* Loop unroll 4 times for 4 vector loop.  */
	VPCMP	$0, (%rax), %VMM0, %k0
	KMOV	%k0, %RCX
	test	%RCX, %RCX
	jnz	L(ret_vec_x1)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMP	$0, VEC_SIZE(%rax), %VMM0, %k0
	KMOV	%k0, %RCX
	test	%RCX, %RCX
	jnz	L(ret_vec_x2)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMP	$0, (VEC_SIZE * 2)(%rax), %VMM0, %k0
	KMOV	%k0, %RCX
	test	%RCX, %RCX
	jnz	L(ret_vec_x3)

# ifdef USE_AS_STRNLEN
	subq	$CHAR_PER_VEC, %rdx
	jbe	L(ret_max)
# endif

	VPCMP	$0, (VEC_SIZE * 3)(%rax), %VMM0, %k0
	KMOV	%k0, %RCX
	test	%RCX, %RCX
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
	SHR	$2, %RCX
#  endif
	/* rcx contains number of [w]char will be recompared due to
	   alignment fixes.  rdx must be incremented by rcx to offset
	   alignment adjustment.  */
	addq	%rcx, %rdx
	/* Need jump as we don't want to add/subtract rdx for first
	   iteration of 4 x VEC_SIZE aligned loop.  */
	jmp	L(loop_entry)
# endif

	.p2align 4,,11
L(loop):
# ifdef USE_AS_STRNLEN
	subq	$(CHAR_PER_VEC * 4), %rdx
	jbe	L(ret_max)
L(loop_entry):
# endif
	/* VPMINU and VPCMP combination provide better performance as
	   compared to alternative combinations.  */
	VMOVA	(VEC_SIZE * 4)(%rax), %VMM1
	VPMINU	(VEC_SIZE * 5)(%rax), %VMM1, %VMM2
	VMOVA	(VEC_SIZE * 6)(%rax), %VMM3
	VPMINU	(VEC_SIZE * 7)(%rax), %VMM3, %VMM4

	VPTESTN	%VMM2, %VMM2, %k0
	VPTESTN	%VMM4, %VMM4, %k1

	subq	$-(VEC_SIZE * 4), %rax
	KORTEST	%k0, %k1
	jz	L(loop)

	VPTESTN	%VMM1, %VMM1, %k2
	KMOV	%k2, %RCX
	test	%RCX, %RCX
	jnz	L(ret_vec_x1)

	KMOV	%k0, %RCX
	/* At this point, if k0 is non zero, null char must be in the
	   second vector.  */
	test	%RCX, %RCX
	jnz	L(ret_vec_x2)

	VPTESTN	%VMM3, %VMM3, %k3
	KMOV	%k3, %RCX
	test	%RCX, %RCX
	jnz	L(ret_vec_x3)
	/* At this point null [w]char must be in the fourth vector so no
	   need to check.  */
	KMOV	%k1, %RCX

	/* Fourth, third, second vector terminating are pretty much
	   same, implemented this way to avoid branching and reuse code
	   from pre loop exit condition.  */
L(ret_vec_x4):
	bsf	%RCX, %RCX
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
	bsf	%RCX, %RCX
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
	bsf	%RCX, %RCX
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
	movl	%eax, %ecx
# ifdef USE_AS_WCSLEN
	andl	$(VEC_SIZE - 1), %ecx
	sarl	$2, %ecx
# endif
	/* ecx contains number of w[char] to be skipped as a result
	   of address alignment.  */
	xorq	%rdi, %rax
	VPCMP	$0, (PAGE_SIZE - VEC_SIZE)(%rax), %VMM0, %k0
	KMOV	%k0, %RAX
	/* Ignore number of character for alignment adjustment.  */
	SHR	%cl, %RAX
	jz	L(align_more)

	bsf	%RAX, %RAX
# ifdef USE_AS_STRNLEN
	cmpq	%rsi, %rax
	cmovnb	%rsi, %rax
# endif
	ret

END (STRLEN)
#endif
