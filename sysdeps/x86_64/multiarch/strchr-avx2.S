/* strchr/strchrnul optimized with AVX2.
   Copyright (C) 2017-2021 Free Software Foundation, Inc.
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

# ifndef STRCHR
#  define STRCHR	__strchr_avx2
# endif

# ifdef USE_AS_WCSCHR
#  define VPBROADCAST	vpbroadcastd
#  define VPCMPEQ	vpcmpeqd
#  define VPMINU	vpminud
#  define CHAR_REG	esi
# else
#  define VPBROADCAST	vpbroadcastb
#  define VPCMPEQ	vpcmpeqb
#  define VPMINU	vpminub
#  define CHAR_REG	sil
# endif

# ifndef VZEROUPPER
#  define VZEROUPPER	vzeroupper
# endif

# ifndef SECTION
#  define SECTION(p)	p##.avx
# endif

# define VEC_SIZE 32
# define PAGE_SIZE 4096

	.section SECTION(.text),"ax",@progbits
ENTRY (STRCHR)
	movl	%edi, %ecx
# ifndef USE_AS_STRCHRNUL
	xorl	%edx, %edx
# endif

	/* Broadcast CHAR to YMM0.	*/
	vmovd	%esi, %xmm0
	vpxor	%xmm9, %xmm9, %xmm9
	VPBROADCAST %xmm0, %ymm0

	/* Check if we cross page boundary with one vector load.  */
	andl	$(PAGE_SIZE - 1), %ecx
	cmpl	$(PAGE_SIZE - VEC_SIZE), %ecx
	ja  L(cross_page_boundary)

	/* Check the first VEC_SIZE bytes.	Search for both CHAR and the
	   null byte.  */
	vmovdqu	(%rdi), %ymm8
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jz	L(more_vecs)
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
	addq	%rdi, %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
L(return_vzeroupper):
	ZERO_UPPER_VEC_REGISTERS_RETURN

	.p2align 4
L(more_vecs):
	/* Align data for aligned loads in the loop.  */
	andq	$-VEC_SIZE, %rdi
L(aligned_more):

	/* Check the next 4 * VEC_SIZE.	 Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.	*/
	vmovdqa	VEC_SIZE(%rdi), %ymm8
	addq	$VEC_SIZE, %rdi
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	vmovdqa	VEC_SIZE(%rdi), %ymm8
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	vmovdqa	(VEC_SIZE * 2)(%rdi), %ymm8
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)

	vmovdqa	(VEC_SIZE * 3)(%rdi), %ymm8
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jz	L(prep_loop_4x)

	tzcntl	%eax, %eax
	leaq	(VEC_SIZE * 3)(%rdi, %rax), %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

	.p2align 4
L(first_vec_x0):
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
	addq	%rdi, %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

	.p2align 4
L(first_vec_x1):
	tzcntl	%eax, %eax
	leaq	VEC_SIZE(%rdi, %rax), %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

	.p2align 4
L(first_vec_x2):
	tzcntl	%eax, %eax
	/* Found CHAR or the null byte.	 */
	leaq	(VEC_SIZE * 2)(%rdi, %rax), %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

L(prep_loop_4x):
	/* Align data to 4 * VEC_SIZE.	*/
	andq	$-(VEC_SIZE * 4), %rdi

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	vmovdqa	(VEC_SIZE * 4)(%rdi), %ymm5
	vmovdqa	(VEC_SIZE * 5)(%rdi), %ymm6
	vmovdqa	(VEC_SIZE * 6)(%rdi), %ymm7
	vmovdqa	(VEC_SIZE * 7)(%rdi), %ymm8

	/* Leaves only CHARS matching esi as 0.	 */
	vpxor	%ymm5, %ymm0, %ymm1
	vpxor	%ymm6, %ymm0, %ymm2
	vpxor	%ymm7, %ymm0, %ymm3
	vpxor	%ymm8, %ymm0, %ymm4

	VPMINU	%ymm1, %ymm5, %ymm1
	VPMINU	%ymm2, %ymm6, %ymm2
	VPMINU	%ymm3, %ymm7, %ymm3
	VPMINU	%ymm4, %ymm8, %ymm4

	VPMINU	%ymm1, %ymm2, %ymm5
	VPMINU	%ymm3, %ymm4, %ymm6

	VPMINU	%ymm5, %ymm6, %ymm5

	VPCMPEQ %ymm5, %ymm9, %ymm5
	vpmovmskb %ymm5, %eax

	addq	$(VEC_SIZE * 4), %rdi
	testl	%eax, %eax
	jz  L(loop_4x_vec)

	VPCMPEQ %ymm1, %ymm9, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMPEQ %ymm2, %ymm9, %ymm2
	vpmovmskb %ymm2, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMPEQ %ymm3, %ymm9, %ymm3
	VPCMPEQ %ymm4, %ymm9, %ymm4
	vpmovmskb %ymm3, %ecx
	vpmovmskb %ymm4, %eax
	salq	$32, %rax
	orq %rcx, %rax
	tzcntq  %rax, %rax
	leaq	(VEC_SIZE * 2)(%rdi, %rax), %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

	/* Cold case for crossing page with first load.	 */
	.p2align 4
L(cross_page_boundary):
	andq	$-VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx

	vmovdqa	(%rdi), %ymm8
	VPCMPEQ %ymm8, %ymm0, %ymm1
	VPCMPEQ %ymm8, %ymm9, %ymm2
	vpor	%ymm1, %ymm2, %ymm1
	vpmovmskb %ymm1, %eax
	/* Remove the leading bits.	 */
	sarxl	%ecx, %eax, %eax
	testl	%eax, %eax
	jz	L(aligned_more)
	tzcntl	%eax, %eax
	addq	%rcx, %rdi
	addq	%rdi, %rax
# ifndef USE_AS_STRCHRNUL
	cmp (%rax), %CHAR_REG
	cmovne	%rdx, %rax
# endif
	VZEROUPPER_RETURN

END (STRCHR)
# endif
