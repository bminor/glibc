/* wcscpy with SSSE3
   Copyright (C) 2011-2013 Free Software Foundation, Inc.
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

#ifndef NOT_IN_libc
# include <sysdep.h>

	.section .text.ssse3,"ax",@progbits
ENTRY (__wcscpy_ssse3)

	mov	%rsi, %rcx
	mov	%rdi, %rdx
# ifdef __CHKP__
	bndcl  	(%rdi), %bnd0
	bndcl  	(%rsi), %bnd1
	bndcu  	(%rsi), %bnd1
# endif

	cmpl	$0, (%rcx)
	jz	L(Exit4)
# ifdef __CHKP__
	bndcu  	4(%rcx), %bnd1
# endif
	cmpl	$0, 4(%rcx)
	jz	L(Exit8)
# ifdef __CHKP__
	bndcu  	8(%rcx), %bnd1
# endif
	cmpl	$0, 8(%rcx)
	jz	L(Exit12)
# ifdef __CHKP__
	bndcu  	12(%rcx), %bnd1
# endif
	cmpl	$0, 12(%rcx)
	jz	L(Exit16)

	lea	16(%rcx), %rsi
	and	$-16, %rsi

	pxor	%xmm0, %xmm0
	mov	(%rcx), %r9
# ifdef __CHKP__
	bndcu  	7(%rdx), %bnd0
# endif
	mov	%r9, (%rdx)

# ifdef __CHKP__
	bndcu  	(%rsi), %bnd1
# endif
	pcmpeqd	(%rsi), %xmm0
	mov	8(%rcx), %r9
# ifdef __CHKP__
	bndcu  	15(%rdx), %bnd0
# endif
	mov	%r9, 8(%rdx)

	pmovmskb %xmm0, %rax
	sub	%rcx, %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

	mov	%rdx, %rax
	lea	16(%rdx), %rdx
	and	$-16, %rdx
	sub	%rdx, %rax
	sub	%rax, %rcx
	mov	%rcx, %rax
	and	$0xf, %rax
	mov	$0, %rsi

/* case: rcx_offset == rdx_offset */

	jz	L(Align16Both)

	cmp	$4, %rax
	je	L(Shl4)
	cmp	$8, %rax
	je	L(Shl8)
	jmp	L(Shl12)

L(Align16Both):
# ifdef __CHKP__
	bndcu  	16(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	(%rcx), %xmm1
	movaps	16(%rcx), %xmm2
	movaps	%xmm1, (%rdx)
	pcmpeqd	%xmm2, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	16(%rcx, %rsi), %xmm3
	movaps	%xmm2, (%rdx, %rsi)
	pcmpeqd	%xmm3, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	16(%rcx, %rsi), %xmm4
	movaps	%xmm3, (%rdx, %rsi)
	pcmpeqd	%xmm4, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	16(%rcx, %rsi), %xmm1
	movaps	%xmm4, (%rdx, %rsi)
	pcmpeqd	%xmm1, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	16(%rcx, %rsi), %xmm2
	movaps	%xmm1, (%rdx, %rsi)
	pcmpeqd	%xmm2, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	16(%rcx, %rsi), %xmm3
	movaps	%xmm2, (%rdx, %rsi)
	pcmpeqd	%xmm3, %xmm0
	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi

	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	16(%rcx, %rsi), %bnd1
	bndcu  	15(%rdx, %rsi), %bnd0
# endif
	movaps	%xmm3, (%rdx, %rsi)
	mov	%rcx, %rax
	lea	16(%rcx, %rsi), %rcx
	and	$-0x40, %rcx
	sub	%rcx, %rax
	sub	%rax, %rdx

	mov	$-0x40, %rsi

	.p2align 4
L(Aligned64Loop):
# ifdef __CHKP__
	bndcu  	(%rcx), %bnd1
	bndcu  	63(%rdx), %bnd0
# endif
	movaps	(%rcx), %xmm2
	movaps	%xmm2, %xmm4
	movaps	16(%rcx), %xmm5
	movaps	32(%rcx), %xmm3
	movaps	%xmm3, %xmm6
	movaps	48(%rcx), %xmm7
	pminub	%xmm5, %xmm2
	pminub	%xmm7, %xmm3
	pminub	%xmm2, %xmm3
	pcmpeqd	%xmm0, %xmm3
	pmovmskb %xmm3, %rax
	lea	64(%rdx), %rdx
	lea	64(%rcx), %rcx
	test	%rax, %rax
	jnz	L(Aligned64Leave)
	movaps	%xmm4, -64(%rdx)
	movaps	%xmm5, -48(%rdx)
	movaps	%xmm6, -32(%rdx)
	movaps	%xmm7, -16(%rdx)
	jmp	L(Aligned64Loop)

L(Aligned64Leave):
	pcmpeqd	%xmm4, %xmm0
	pmovmskb %xmm0, %rax
	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

	pcmpeqd	%xmm5, %xmm0

	pmovmskb %xmm0, %rax
# ifdef __CHKP__
	bndcu  	-49(%rdx), %bnd0
# endif
	movaps	%xmm4, -64(%rdx)
	test	%rax, %rax
	lea	16(%rsi), %rsi
	jnz	L(CopyFrom1To16Bytes)

	pcmpeqd	%xmm6, %xmm0

	pmovmskb %xmm0, %rax
# ifdef __CHKP__
	bndcu  	-33(%rdx), %bnd0
# endif
	movaps	%xmm5, -48(%rdx)
	test	%rax, %rax
	lea	16(%rsi), %rsi
	jnz	L(CopyFrom1To16Bytes)

# ifdef __CHKP__
	bndcu  	-17(%rdx), %bnd0
# endif
	movaps	%xmm6, -32(%rdx)
	pcmpeqd	%xmm7, %xmm0

	pmovmskb %xmm0, %rax
	lea	16(%rsi), %rsi
	test	%rax, %rax
	jnz	L(CopyFrom1To16Bytes)

	mov	$-0x40, %rsi
# ifdef __CHKP__
	bndcu  	-1(%rdx), %bnd0
# endif
	movaps	%xmm7, -16(%rdx)
	jmp	L(Aligned64Loop)

	.p2align 4
L(Shl4):
# ifdef __CHKP__
	bndcu  	12(%rcx), %bnd1
# endif
	movaps	-4(%rcx), %xmm1
	movaps	12(%rcx), %xmm2
L(Shl4Start):
	pcmpeqd	%xmm2, %xmm0
	pmovmskb %xmm0, %rax
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl4LoopExit)

	palignr	$4, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	28(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	28(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm1

	test	%rax, %rax
	jnz	L(Shl4LoopExit)

	palignr	$4, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	28(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	28(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl4LoopExit)

	palignr	$4, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	28(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	28(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx

	test	%rax, %rax
	jnz	L(Shl4LoopExit)

	palignr	$4, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	lea	28(%rcx), %rcx
	lea	16(%rdx), %rdx

	mov	%rcx, %rax
	and	$-0x40, %rcx
	sub	%rcx, %rax
	lea	-12(%rcx), %rcx
	sub	%rax, %rdx

	movaps	-4(%rcx), %xmm1

	.p2align 4
L(Shl4LoopStart):
# ifdef __CHKP__
	bndcu  	12(%rcx), %bnd1
# endif
	movaps	12(%rcx), %xmm2
	movaps	28(%rcx), %xmm3
	movaps	%xmm3, %xmm6
	movaps	44(%rcx), %xmm4
	movaps	%xmm4, %xmm7
	movaps	60(%rcx), %xmm5
	pminub	%xmm2, %xmm6
	pminub	%xmm5, %xmm7
	pminub	%xmm6, %xmm7
	pcmpeqd	%xmm0, %xmm7
	pmovmskb %xmm7, %rax
	movaps	%xmm5, %xmm7
	palignr	$4, %xmm4, %xmm5
	test	%rax, %rax
	palignr	$4, %xmm3, %xmm4
	jnz	L(Shl4Start)

	palignr	$4, %xmm2, %xmm3
	lea	64(%rcx), %rcx
	palignr	$4, %xmm1, %xmm2
	movaps	%xmm7, %xmm1
# ifdef __CHKP__
	bndcu  	63(%rdx), %bnd0
# endif
	movaps	%xmm5, 48(%rdx)
	movaps	%xmm4, 32(%rdx)
	movaps	%xmm3, 16(%rdx)
	movaps	%xmm2, (%rdx)
	lea	64(%rdx), %rdx
	jmp	L(Shl4LoopStart)

L(Shl4LoopExit):
# ifdef __CHKP__
	bndcu  	-4(%rcx), %bnd1
	bndcu  	11(%rdx), %bnd0
# endif
	movdqu	-4(%rcx), %xmm1
	mov	$12, %rsi
	movdqu	%xmm1, -4(%rdx)
	jmp	L(CopyFrom1To16Bytes)

	.p2align 4
L(Shl8):
# ifdef __CHKP__
	bndcu  	8(%rcx), %bnd1
# endif
	movaps	-8(%rcx), %xmm1
	movaps	8(%rcx), %xmm2
L(Shl8Start):
	pcmpeqd	%xmm2, %xmm0
	pmovmskb %xmm0, %rax
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl8LoopExit)

	palignr	$8, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	24(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	24(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm1

	test	%rax, %rax
	jnz	L(Shl8LoopExit)

	palignr	$8, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	24(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	24(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl8LoopExit)

	palignr	$8, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	24(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	24(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx

	test	%rax, %rax
	jnz	L(Shl8LoopExit)

	palignr	$8, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	24(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	lea	24(%rcx), %rcx
	lea	16(%rdx), %rdx

	mov	%rcx, %rax
	and	$-0x40, %rcx
	sub	%rcx, %rax
	lea	-8(%rcx), %rcx
	sub	%rax, %rdx

	movaps	-8(%rcx), %xmm1

	.p2align 4
L(Shl8LoopStart):
# ifdef __CHKP__
	bndcu  	8(%rcx), %bnd1
# endif
	movaps	8(%rcx), %xmm2
	movaps	24(%rcx), %xmm3
	movaps	%xmm3, %xmm6
	movaps	40(%rcx), %xmm4
	movaps	%xmm4, %xmm7
	movaps	56(%rcx), %xmm5
	pminub	%xmm2, %xmm6
	pminub	%xmm5, %xmm7
	pminub	%xmm6, %xmm7
	pcmpeqd	%xmm0, %xmm7
	pmovmskb %xmm7, %rax
	movaps	%xmm5, %xmm7
	palignr	$8, %xmm4, %xmm5
	test	%rax, %rax
	palignr	$8, %xmm3, %xmm4
	jnz	L(Shl8Start)

	palignr	$8, %xmm2, %xmm3
	lea	64(%rcx), %rcx
	palignr	$8, %xmm1, %xmm2
	movaps	%xmm7, %xmm1
# ifdef __CHKP__
	bndcu  	63(%rdx), %bnd0
# endif
	movaps	%xmm5, 48(%rdx)
	movaps	%xmm4, 32(%rdx)
	movaps	%xmm3, 16(%rdx)
	movaps	%xmm2, (%rdx)
	lea	64(%rdx), %rdx
	jmp	L(Shl8LoopStart)

L(Shl8LoopExit):
# ifdef __CHKP__
	bndcu  	(%rcx), %bnd1
	bndcu  	7(%rdx), %bnd0
# endif
	mov	(%rcx), %r9
	mov	$8, %rsi
	mov	%r9, (%rdx)
	jmp	L(CopyFrom1To16Bytes)

	.p2align 4
L(Shl12):
# ifdef __CHKP__
	bndcu  	4(%rcx), %bnd1
# endif
	movaps	-12(%rcx), %xmm1
	movaps	4(%rcx), %xmm2
L(Shl12Start):
	pcmpeqd	%xmm2, %xmm0
	pmovmskb %xmm0, %rax
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl12LoopExit)

	palignr	$12, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	20(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	20(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm1

	test	%rax, %rax
	jnz	L(Shl12LoopExit)

	palignr	$12, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	20(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	20(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx
	movaps	%xmm2, %xmm3

	test	%rax, %rax
	jnz	L(Shl12LoopExit)

	palignr	$12, %xmm1, %xmm2
# ifdef __CHKP__
	bndcu  	20(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	movaps	20(%rcx), %xmm2

	pcmpeqd	%xmm2, %xmm0
	lea	16(%rdx), %rdx
	pmovmskb %xmm0, %rax
	lea	16(%rcx), %rcx

	test	%rax, %rax
	jnz	L(Shl12LoopExit)

	palignr	$12, %xmm3, %xmm2
# ifdef __CHKP__
	bndcu  	20(%rcx), %bnd1
	bndcu  	15(%rdx), %bnd0
# endif
	movaps	%xmm2, (%rdx)
	lea	20(%rcx), %rcx
	lea	16(%rdx), %rdx

	mov	%rcx, %rax
	and	$-0x40, %rcx
	sub	%rcx, %rax
	lea	-4(%rcx), %rcx
	sub	%rax, %rdx

	movaps	-12(%rcx), %xmm1

	.p2align 4
L(Shl12LoopStart):
# ifdef __CHKP__
	bndcu  	4(%rcx), %bnd1
# endif
	movaps	4(%rcx), %xmm2
	movaps	20(%rcx), %xmm3
	movaps	%xmm3, %xmm6
	movaps	36(%rcx), %xmm4
	movaps	%xmm4, %xmm7
	movaps	52(%rcx), %xmm5
	pminub	%xmm2, %xmm6
	pminub	%xmm5, %xmm7
	pminub	%xmm6, %xmm7
	pcmpeqd	%xmm0, %xmm7
	pmovmskb %xmm7, %rax
	movaps	%xmm5, %xmm7
	palignr	$12, %xmm4, %xmm5
	test	%rax, %rax
	palignr	$12, %xmm3, %xmm4
	jnz	L(Shl12Start)
	palignr	$12, %xmm2, %xmm3
	lea	64(%rcx), %rcx
	palignr	$12, %xmm1, %xmm2
	movaps	%xmm7, %xmm1
# ifdef __CHKP__
	bndcu  	63(%rdx), %bnd0
# endif
	movaps	%xmm5, 48(%rdx)
	movaps	%xmm4, 32(%rdx)
	movaps	%xmm3, 16(%rdx)
	movaps	%xmm2, (%rdx)
	lea	64(%rdx), %rdx
	jmp	L(Shl12LoopStart)

L(Shl12LoopExit):
# ifdef __CHKP__
	bndcu  	(%rcx), %bnd1
	bndcu  	3(%rdx), %bnd0
# endif
	mov	(%rcx), %r9d
	mov	$4, %rsi
	mov	%r9d, (%rdx)
	jmp	L(CopyFrom1To16Bytes)

	.p2align 4
L(CopyFrom1To16Bytes):
	add	%rsi, %rdx
	add	%rsi, %rcx

	test	%al, %al
	jz	L(ExitHigh)
	test	$0x01, %al
	jnz	L(Exit4)

	mov	(%rcx), %rax
# ifdef __CHKP__
	bndcu  	7(%rdx), %bnd0
# endif
	mov	%rax, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(ExitHigh):
	test	$0x01, %ah
	jnz	L(Exit12)

	mov	(%rcx), %rax
# ifdef __CHKP__
	bndcu  	15(%rdx), %bnd0
# endif
	mov	%rax, (%rdx)
	mov	8(%rcx), %rax
	mov	%rax, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(Exit4):
	movl	(%rcx), %eax
# ifdef __CHKP__
	bndcu  	3(%rdx), %bnd0
# endif
	movl	%eax, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(Exit8):
	mov	(%rcx), %rax
# ifdef __CHKP__
	bndcu  	7(%rdx), %bnd0
# endif
	mov	%rax, (%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(Exit12):
	mov	(%rcx), %rax
# ifdef __CHKP__
	bndcu  	11(%rdx), %bnd0
# endif
	mov	%rax, (%rdx)
	mov	8(%rcx), %eax
	mov	%eax, 8(%rdx)
	mov	%rdi, %rax
	ret

	.p2align 4
L(Exit16):
	mov	(%rcx), %rax
# ifdef __CHKP__
	bndcu  	15(%rdx), %bnd0
# endif
	mov	%rax, (%rdx)
	mov	8(%rcx), %rax
	mov	%rax, 8(%rdx)
	mov	%rdi, %rax
	ret

END(__wcscpy_ssse3)
#endif
