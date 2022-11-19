/* Wrapper implementations of vector math functions.
   Copyright (C) 2014-2022 Free Software Foundation, Inc.
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

/* SSE2 ISA version as wrapper to scalar.  */
.macro WRAPPER_IMPL_SSE2 callee
	push	%rbx
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbx, 0)
	subq	$16, %rsp
	cfi_adjust_cfa_offset (16)
	movaps	%xmm0, (%rsp)
	call	JUMPTARGET(\callee)
	movss	%xmm0, (%rsp)
	movss	4(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	movss	%xmm0, 4(%rsp)
	movss	8(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	movd	%xmm0, %ebx
	movss	12(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	movd	%ebx, %xmm1
	unpcklps %xmm0, %xmm1
	movsd	(%rsp), %xmm0
	unpcklpd %xmm1, %xmm0
	addq	$16, %rsp
	cfi_adjust_cfa_offset (-16)
	popq	%rbx
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbx)
	ret
.endm

/* 2 argument SSE2 ISA version as wrapper to scalar.  */
.macro WRAPPER_IMPL_SSE2_ff callee
	push	%rbx
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbx, 0)
	subq	$32, %rsp
	cfi_adjust_cfa_offset (40)
	movaps	%xmm0, (%rsp)
	movaps	%xmm1, 16(%rsp)
	call	JUMPTARGET(\callee)
	movss	20(%rsp), %xmm1
	movss	%xmm0, 0(%rsp)
	movss	4(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	movss	24(%rsp), %xmm1
	movss	%xmm0, 4(%rsp)
	movss	8(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	movss	28(%rsp), %xmm1
	movd	%xmm0, %ebx
	movss	12(%rsp), %xmm0
	call	JUMPTARGET(\callee)
	/* merge 4x results into xmm0.  */
	movd	%ebx, %xmm1
	unpcklps %xmm0, %xmm1
	movsd	(%rsp), %xmm0
	unpcklpd %xmm1, %xmm0
	addq	$32, %rsp
	cfi_adjust_cfa_offset (-32)
	popq	%rbx
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbx)
	ret
.endm

/* 3 argument SSE2 ISA version as wrapper to scalar.  */
.macro WRAPPER_IMPL_SSE2_fFF callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	pushq	%rbx
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbx, 0)
	movq	%rdi, %rbp
	movq	%rsi, %rbx
	subq	$24, %rsp
	cfi_adjust_cfa_offset (24)
	movaps	%xmm0, (%rsp)
	call	JUMPTARGET(\callee)
	movss	4(%rsp), %xmm0
	leaq	4(%rbp), %rdi
	leaq	4(%rbx), %rsi
	call	JUMPTARGET(\callee)
	movss	8(%rsp), %xmm0
	leaq	8(%rbp), %rdi
	leaq	8(%rbx), %rsi
	call	JUMPTARGET(\callee)
	movss	12(%rsp), %xmm0
	leaq	12(%rbp), %rdi
	leaq	12(%rbx), %rsi
	call	JUMPTARGET(\callee)
	addq	$24, %rsp
	cfi_adjust_cfa_offset (-24)
	popq	%rbx
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbx)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	cfi_def_cfa_register (%rbp)
	andq	$-32, %rsp
	subq	$32, %rsp
	vmovaps	%ymm0, (%rsp)
	vzeroupper
	call	HIDDEN_JUMPTARGET(\callee)
	vmovaps	%xmm0, (%rsp)
	vmovaps	16(%rsp), %xmm0
	call	HIDDEN_JUMPTARGET(\callee)
	/* combine xmm0 (return of second call) with result of first
	   call (saved on stack). Might be worth exploring logic that
	   uses `vpblend` and reads in ymm1 using -16(rsp).  */
	vmovaps	(%rsp), %xmm1
	vinsertf128 $1, %xmm0, %ymm1, %ymm0
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* 2 argument AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX_ff callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	cfi_def_cfa_register (%rbp)
	andq	$-32, %rsp
	subq	$64, %rsp
	vmovaps	%ymm0, (%rsp)
	vmovaps	%ymm1, 32(%rsp)
	vzeroupper
	call	HIDDEN_JUMPTARGET(\callee)
	vmovaps	48(%rsp), %xmm1
	vmovaps	%xmm0, (%rsp)
	vmovaps	16(%rsp), %xmm0
	call	HIDDEN_JUMPTARGET(\callee)
	/* combine xmm0 (return of second call) with result of first
	   call (saved on stack). Might be worth exploring logic that
	   uses `vpblend` and reads in ymm1 using -16(rsp).  */
	vmovaps	(%rsp), %xmm1
	vinsertf128 $1, %xmm0, %ymm1, %ymm0
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* 3 argument AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX_fFF callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	andq	$-32, %rsp
	subq	$32, %rsp
	vmovaps	%ymm0, (%rsp)
	pushq	%rbx
	pushq	%r14
	movq	%rdi, %rbx
	movq	%rsi, %r14
	vzeroupper
	call	HIDDEN_JUMPTARGET(\callee)
	vmovaps	32(%rsp), %xmm0
	leaq	16(%rbx), %rdi
	leaq	16(%r14), %rsi
	call	HIDDEN_JUMPTARGET(\callee)
	popq	%r14
	popq	%rbx
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* AVX512 ISA version as wrapper to AVX2 ISA version.  */
.macro WRAPPER_IMPL_AVX512 callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	cfi_def_cfa_register (%rbp)
	andq	$-64, %rsp
	subq	$64, %rsp
	vmovups	%zmm0, (%rsp)
	call	HIDDEN_JUMPTARGET(\callee)
	vmovupd	%ymm0, (%rsp)
	vmovupd	32(%rsp), %ymm0
	call	HIDDEN_JUMPTARGET(\callee)
	/* combine ymm0 (return of second call) with result of first
	   call (saved on stack).  */
	vmovaps	(%rsp), %ymm1
	vinserti64x4 $0x1, %ymm0, %zmm1, %zmm0
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* 2 argument AVX512 ISA version as wrapper to AVX2 ISA version.  */
.macro WRAPPER_IMPL_AVX512_ff callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	cfi_def_cfa_register (%rbp)
	andq	$-64, %rsp
	addq	$-128, %rsp
	vmovups	%zmm0, (%rsp)
	vmovups	%zmm1, 64(%rsp)
	/* ymm0 and ymm1 are already set.  */
	call	HIDDEN_JUMPTARGET(\callee)
	vmovups	96(%rsp), %ymm1
	vmovaps	%ymm0, (%rsp)
	vmovups	32(%rsp), %ymm0
	call	HIDDEN_JUMPTARGET(\callee)
	/* combine ymm0 (return of second call) with result of first
	   call (saved on stack).  */
	vmovaps	(%rsp), %ymm1
	vinserti64x4 $0x1, %ymm0, %zmm1, %zmm0
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm

/* 3 argument AVX512 ISA version as wrapper to AVX2 ISA version.  */
.macro WRAPPER_IMPL_AVX512_fFF callee
	pushq	%rbp
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (%rbp, 0)
	movq	%rsp, %rbp
	cfi_def_cfa_register (%rbp)
	andq	$-64, %rsp
	subq	$64, %rsp
	vmovaps	%zmm0, (%rsp)
	pushq	%rbx
	pushq	%r14
	movq	%rdi, %rbx
	movq	%rsi, %r14
	/* ymm0 is already set.  */
	call	HIDDEN_JUMPTARGET(\callee)
	vmovaps	48(%rsp), %ymm0
	leaq	32(%rbx), %rdi
	leaq	32(%r14), %rsi
	call	HIDDEN_JUMPTARGET(\callee)
	popq	%r14
	popq	%rbx
	movq	%rbp, %rsp
	cfi_def_cfa_register (%rsp)
	popq	%rbp
	cfi_adjust_cfa_offset (-8)
	cfi_restore (%rbp)
	ret
.endm
