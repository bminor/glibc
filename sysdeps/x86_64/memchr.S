/* Copyright (C) 2011-2013 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* fast SSE2 version with using pmaxub and 64 byte loop */

#  ifdef __CHKP__
#   define RETURN \
	bndcu  (%rax), %bnd0; \
	ret
#  else
#   define RETURN ret
#  endif

	.text
ENTRY(memchr)

	movd	%rsi, %xmm1
	mov	%rdi, %rcx

	punpcklbw %xmm1, %xmm1
	test	%rdx, %rdx
	jz	L(return_null)
	punpcklbw %xmm1, %xmm1

	and	$63, %rcx
	pshufd	$0, %xmm1, %xmm1

#ifdef __CHKP__
        bndcl  	(%rdi), %bnd0
        bndcu  	(%rdi), %bnd0
#endif
	cmp	$48, %rcx
	ja	L(crosscache)

	movdqu	(%rdi), %xmm0
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %eax
	test	%eax, %eax

	jnz	L(matches_1)
	sub	$16, %rdx
	jbe	L(return_null)
	add	$16, %rdi
	and	$15, %rcx
	and	$-16, %rdi
	add	%rcx, %rdx
	sub	$64, %rdx
	jbe	L(exit_loop)
	jmp	L(loop_prolog)

	.p2align 4
L(crosscache):
	and	$15, %rcx
	and	$-16, %rdi
	movdqa	(%rdi), %xmm0

	pcmpeqb	%xmm1, %xmm0
/* Check if there is a match.  */
	pmovmskb %xmm0, %eax
/* Remove the leading bytes.  */
	sar	%cl, %eax
	test	%eax, %eax
	je	L(unaligned_no_match)
/* Check which byte is a match.  */
	bsf	%eax, %eax

	sub	%rax, %rdx
	jbe	L(return_null)
	add	%rdi, %rax
	add	%rcx, %rax
	RETURN

	.p2align 4
L(unaligned_no_match):
	add	%rcx, %rdx
	sub	$16, %rdx
	jbe	L(return_null)
	add	$16, %rdi
	sub	$64, %rdx
	jbe	L(exit_loop)

	.p2align 4
L(loop_prolog):
#ifdef __CHKP__
        bndcu  	(%rdi), %bnd0
#endif
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %eax
	test	%eax, %eax
	jnz	L(matches)

#ifdef __CHKP__
        bndcu  	16(%rdi), %bnd0
#endif
	movdqa	16(%rdi), %xmm2
	pcmpeqb	%xmm1, %xmm2
	pmovmskb %xmm2, %eax
	test	%eax, %eax
	jnz	L(matches16)

#ifdef __CHKP__
        bndcu  	32(%rdi), %bnd0
#endif
	movdqa	32(%rdi), %xmm3
	pcmpeqb	%xmm1, %xmm3
	pmovmskb %xmm3, %eax
	test	%eax, %eax
	jnz	L(matches32)

#ifdef __CHKP__
        bndcu  	48(%rdi), %bnd0
#endif
	movdqa	48(%rdi), %xmm4
	pcmpeqb	%xmm1, %xmm4
	add	$64, %rdi
	pmovmskb %xmm4, %eax
	test	%eax, %eax
	jnz	L(matches0)

	test	$0x3f, %rdi
	jz	L(align64_loop)

	sub	$64, %rdx
	jbe	L(exit_loop)

#ifdef __CHKP__
        bndcu  	(%rdi), %bnd0
#endif
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %eax
	test	%eax, %eax
	jnz	L(matches)

#ifdef __CHKP__
        bndcu  	16(%rdi), %bnd0
#endif
	movdqa	16(%rdi), %xmm2
	pcmpeqb	%xmm1, %xmm2
	pmovmskb %xmm2, %eax
	test	%eax, %eax
	jnz	L(matches16)

#ifdef __CHKP__
        bndcu  	32(%rdi), %bnd0
#endif
	movdqa	32(%rdi), %xmm3
	pcmpeqb	%xmm1, %xmm3
	pmovmskb %xmm3, %eax
	test	%eax, %eax
	jnz	L(matches32)

#ifdef __CHKP__
        bndcu  	48(%rdi), %bnd0
#endif
	movdqa	48(%rdi), %xmm3
	pcmpeqb	%xmm1, %xmm3
	pmovmskb %xmm3, %eax

	add	$64, %rdi
	test	%eax, %eax
	jnz	L(matches0)

	mov	%rdi, %rcx
	and	$-64, %rdi
	and	$63, %rcx
	add	%rcx, %rdx

	.p2align 4
L(align64_loop):
	sub	$64, %rdx
	jbe	L(exit_loop)
#ifdef __CHKP__
        bndcu  	(%rdi), %bnd0
#endif
	movdqa	(%rdi), %xmm0
	movdqa	16(%rdi), %xmm2
	movdqa	32(%rdi), %xmm3
	movdqa	48(%rdi), %xmm4

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm1, %xmm2
	pcmpeqb	%xmm1, %xmm3
	pcmpeqb	%xmm1, %xmm4

	pmaxub	%xmm0, %xmm3
	pmaxub	%xmm2, %xmm4
	pmaxub	%xmm3, %xmm4
	pmovmskb %xmm4, %eax

	add	$64, %rdi

	test	%eax, %eax
	jz	L(align64_loop)

	sub	$64, %rdi

	pmovmskb %xmm0, %eax
	test	%eax, %eax
	jnz	L(matches)

	pmovmskb %xmm2, %eax
	test	%eax, %eax
	jnz	L(matches16)

	movdqa	32(%rdi), %xmm3
	pcmpeqb	%xmm1, %xmm3

	pcmpeqb	48(%rdi), %xmm1
	pmovmskb %xmm3, %eax
	test	%eax, %eax
	jnz	L(matches32)

	pmovmskb %xmm1, %eax
	bsf	%eax, %eax
	lea	48(%rdi, %rax), %rax
	RETURN

	.p2align 4
L(exit_loop):
	add	$32, %rdx
	jle	L(exit_loop_32)

#ifdef __CHKP__
        bndcu  	(%rdi), %bnd0
#endif
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %eax
	test	%eax, %eax
	jnz	L(matches)

#ifdef __CHKP__
        bndcu   16(%rdi), %bnd0
#endif
	movdqa	16(%rdi), %xmm2
	pcmpeqb	%xmm1, %xmm2
	pmovmskb %xmm2, %eax
	test	%eax, %eax
	jnz	L(matches16)

#ifdef __CHKP__
        bndcu   32(%rdi), %bnd0
#endif
	movdqa	32(%rdi), %xmm3
	pcmpeqb	%xmm1, %xmm3
	pmovmskb %xmm3, %eax
	test	%eax, %eax
	jnz	L(matches32_1)
	sub	$16, %rdx
	jle	L(return_null)

#ifdef __CHKP__
        bndcu   48(%rdi), %bnd0
#endif
	pcmpeqb	48(%rdi), %xmm1
	pmovmskb %xmm1, %eax
	test	%eax, %eax
	jnz	L(matches48_1)
	xor	%rax, %rax
	ret

	.p2align 4
L(exit_loop_32):
	add	$32, %rdx
#ifdef __CHKP__
        bndcu   (%rdi), %bnd0
#endif
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %eax
	test	%eax, %eax
	jnz	L(matches_1)
	sub	$16, %rdx
	jbe	L(return_null)

#ifdef __CHKP__
        bndcu   16(%rdi), %bnd0
#endif
	pcmpeqb	16(%rdi), %xmm1
	pmovmskb %xmm1, %eax
	test	%eax, %eax
	jnz	L(matches16_1)
	xor	%rax, %rax
	ret

	.p2align 4
L(matches0):
	bsf	%eax, %eax
	lea	-16(%rax, %rdi), %rax
	RETURN

	.p2align 4
L(matches):
	bsf	%eax, %eax
	add	%rdi, %rax
	RETURN

	.p2align 4
L(matches16):
	bsf	%eax, %eax
	lea	16(%rax, %rdi), %rax
	RETURN

	.p2align 4
L(matches32):
	bsf	%eax, %eax
	lea	32(%rax, %rdi), %rax
	RETURN

	.p2align 4
L(matches_1):
	bsf	%eax, %eax
	sub	%rax, %rdx
	jbe	L(return_null)
	add	%rdi, %rax
	RETURN

	.p2align 4
L(matches16_1):
	bsf	%eax, %eax
	sub	%rax, %rdx
	jbe	L(return_null)
	lea	16(%rdi, %rax), %rax
	RETURN

	.p2align 4
L(matches32_1):
	bsf	%eax, %eax
	sub	%rax, %rdx
	jbe	L(return_null)
	lea	32(%rdi, %rax), %rax
	RETURN

	.p2align 4
L(matches48_1):
	bsf	%eax, %eax
	sub	%rax, %rdx
	jbe	L(return_null)
	lea	48(%rdi, %rax), %rax
	RETURN

	.p2align 4
L(return_null):
	xor	%rax, %rax
	ret
END(memchr)

strong_alias (memchr, __memchr)

libc_hidden_builtin_def(memchr)
