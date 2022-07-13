/* strcmp/wcscmp/strncmp/wcsncmp optimized with 256-bit EVEX instructions.
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

# define STRCMP_ISA	_evex
# include "strcmp-naming.h"

# include <sysdep.h>
# if defined USE_AS_STRCASECMP_L
#  include "locale-defines.h"
# endif

# ifndef STRCMP
#  define STRCMP	__strcmp_evex
# endif

# define PAGE_SIZE	4096

	/* VEC_SIZE = Number of bytes in a ymm register.  */
# define VEC_SIZE	32
# define CHAR_PER_VEC	(VEC_SIZE	/	SIZE_OF_CHAR)

# define VMOVU	vmovdqu64
# define VMOVA	vmovdqa64

# ifdef USE_AS_WCSCMP
#  define TESTEQ	subl $0xff,
	/* Compare packed dwords.  */
#  define VPCMP	vpcmpd
#  define VPMINU	vpminud
#  define VPTESTM	vptestmd
#  define VPTESTNM	vptestnmd
	/* 1 dword char == 4 bytes.  */
#  define SIZE_OF_CHAR	4
# else
#  define TESTEQ	incl
	/* Compare packed bytes.  */
#  define VPCMP	vpcmpb
#  define VPMINU	vpminub
#  define VPTESTM	vptestmb
#  define VPTESTNM	vptestnmb
	/* 1 byte char == 1 byte.  */
#  define SIZE_OF_CHAR	1
# endif

# ifdef USE_AS_STRNCMP
#  define LOOP_REG	r9d
#  define LOOP_REG64	r9

#  define OFFSET_REG8	r9b
#  define OFFSET_REG	r9d
#  define OFFSET_REG64	r9
# else
#  define LOOP_REG	edx
#  define LOOP_REG64	rdx

#  define OFFSET_REG8	dl
#  define OFFSET_REG	edx
#  define OFFSET_REG64	rdx
# endif

# if defined USE_AS_STRNCMP || defined USE_AS_WCSCMP
#  define VEC_OFFSET	0
# else
#  define VEC_OFFSET	(-VEC_SIZE)
# endif

# define XMM0	xmm17
# define XMM1	xmm18

# define XMM10	xmm27
# define XMM11	xmm28
# define XMM12	xmm29
# define XMM13	xmm30
# define XMM14	xmm31


# define YMM0	ymm17
# define YMM1	ymm18
# define YMM2	ymm19
# define YMM3	ymm20
# define YMM4	ymm21
# define YMM5	ymm22
# define YMM6	ymm23
# define YMM7	ymm24
# define YMM8	ymm25
# define YMM9	ymm26
# define YMM10	ymm27
# define YMM11	ymm28
# define YMM12	ymm29
# define YMM13	ymm30
# define YMM14	ymm31

# ifdef USE_AS_STRCASECMP_L
#  define BYTE_LOOP_REG	OFFSET_REG
# else
#  define BYTE_LOOP_REG	ecx
# endif

# ifdef USE_AS_STRCASECMP_L
#  ifdef USE_AS_STRNCMP
#   define LOCALE_REG	rcx
#   define LOCALE_REG_LP	RCX_LP
#  else
#   define LOCALE_REG	rdx
#   define LOCALE_REG_LP	RDX_LP
#  endif
# endif

# define LCASE_MIN_YMM	%YMM12
# define LCASE_MAX_YMM	%YMM13
# define CASE_ADD_YMM	%YMM14

# define LCASE_MIN_XMM	%XMM12
# define LCASE_MAX_XMM	%XMM13
# define CASE_ADD_XMM	%XMM14

	/* NB: wcsncmp uses r11 but strcasecmp is never used in
	   conjunction with wcscmp.  */
# define TOLOWER_BASE	%r11

# ifdef USE_AS_STRCASECMP_L
#  define _REG(x, y) x ## y
#  define REG(x, y) _REG(x, y)
#  define TOLOWER(reg1, reg2, ext)										\
	vpsubb	REG(LCASE_MIN_, ext), reg1, REG(%ext, 10);					\
	vpsubb	REG(LCASE_MIN_, ext), reg2, REG(%ext, 11);					\
	vpcmpub	$1, REG(LCASE_MAX_, ext), REG(%ext, 10), %k5;				\
	vpcmpub	$1, REG(LCASE_MAX_, ext), REG(%ext, 11), %k6;				\
	vpaddb	reg1, REG(CASE_ADD_, ext), reg1{%k5};						\
	vpaddb	reg2, REG(CASE_ADD_, ext), reg2{%k6}

#  define TOLOWER_gpr(src, dst) movl (TOLOWER_BASE, src, 4), dst
#  define TOLOWER_YMM(...)	TOLOWER(__VA_ARGS__, YMM)
#  define TOLOWER_XMM(...)	TOLOWER(__VA_ARGS__, XMM)

#  define CMP_R1_R2(s1_reg, s2_reg, reg_out, ext)						\
	TOLOWER	(s1_reg, s2_reg, ext);										\
	VPCMP	$0, s1_reg, s2_reg, reg_out

#  define CMP_R1_S2(s1_reg, s2_mem, s2_reg, reg_out, ext)				\
	VMOVU	s2_mem, s2_reg;												\
	CMP_R1_R2(s1_reg, s2_reg, reg_out, ext)

#  define CMP_R1_R2_YMM(...) CMP_R1_R2(__VA_ARGS__, YMM)
#  define CMP_R1_R2_XMM(...) CMP_R1_R2(__VA_ARGS__, XMM)

#  define CMP_R1_S2_YMM(...) CMP_R1_S2(__VA_ARGS__, YMM)
#  define CMP_R1_S2_XMM(...) CMP_R1_S2(__VA_ARGS__, XMM)

# else
#  define TOLOWER_gpr(...)
#  define TOLOWER_YMM(...)
#  define TOLOWER_XMM(...)

#  define CMP_R1_R2_YMM(s1_reg, s2_reg, reg_out)						\
	VPCMP	$0, s2_reg, s1_reg, reg_out

#  define CMP_R1_R2_XMM(...) CMP_R1_R2_YMM(__VA_ARGS__)

#  define CMP_R1_S2_YMM(s1_reg, s2_mem, unused, reg_out)				\
	VPCMP	$0, s2_mem, s1_reg, reg_out

#  define CMP_R1_S2_XMM(...) CMP_R1_S2_YMM(__VA_ARGS__)
# endif

/* Warning!
           wcscmp/wcsncmp have to use SIGNED comparison for elements.
           strcmp/strncmp have to use UNSIGNED comparison for elements.
*/

/* The main idea of the string comparison (byte or dword) using 256-bit
   EVEX instructions consists of comparing (VPCMP) two ymm vectors. The
   latter can be on either packed bytes or dwords depending on
   USE_AS_WCSCMP. In order to check the null CHAR, algorithm keeps the
   matched bytes/dwords, requiring 5 EVEX instructions (3 VPCMP and 2
   KORD). In general, the costs of comparing VEC_SIZE bytes (32-bytes)
   are 3 VPCMP and 2 KORD instructions, together with VMOVU and ktestd
   instructions.  Main loop (away from from page boundary) compares 4
   vectors are a time, effectively comparing 4 x VEC_SIZE bytes (128
   bytes) on each loop.

   The routine strncmp/wcsncmp (enabled by defining USE_AS_STRNCMP) logic
   is the same as strcmp, except that an a maximum offset is tracked.  If
   the maximum offset is reached before a difference is found, zero is
   returned.  */

	.section .text.evex, "ax", @progbits
	.align	16
	.type	STRCMP, @function
	.globl	STRCMP
# ifdef USE_AS_STRCASECMP_L
ENTRY (STRCASECMP)
	movq	__libc_tsd_LOCALE@gottpoff(%rip), %rax
	mov	%fs:(%rax), %LOCALE_REG_LP

	/* Either 1 or 5 bytes (dependeing if CET is enabled).  */
	.p2align 4
END (STRCASECMP)
	/* FALLTHROUGH to strcasecmp/strncasecmp_l.  */
# endif

	.p2align 4
STRCMP:
	cfi_startproc
	_CET_ENDBR
	CALL_MCOUNT

# if defined USE_AS_STRCASECMP_L
	/* We have to fall back on the C implementation for locales with
	   encodings not matching ASCII for single bytes.  */
#  if LOCALE_T___LOCALES != 0 || LC_CTYPE != 0
	mov	LOCALE_T___LOCALES + LC_CTYPE * LP_SIZE(%LOCALE_REG), %RAX_LP
#  else
	mov	(%LOCALE_REG), %RAX_LP
#  endif
	testl	$1, LOCALE_DATA_VALUES + _NL_CTYPE_NONASCII_CASE * SIZEOF_VALUES(%rax)
	jne	STRCASECMP_L_NONASCII
	leaq	_nl_C_LC_CTYPE_tolower + 128 * 4(%rip), TOLOWER_BASE
# endif

# ifdef USE_AS_STRNCMP
	/* Don't overwrite LOCALE_REG (rcx) until we have pass
	   L(one_or_less). Otherwise we might use the wrong locale in
	   the OVERFLOW_STRCMP (strcasecmp_l).  */
#  ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	movl	%edx, %edx
#  endif
	cmp	$1, %RDX_LP
	/* Signed comparison intentional. We use this branch to also
	   test cases where length >= 2^63. These very large sizes can be
	   handled with strcmp as there is no way for that length to
	   actually bound the buffer.  */
	jle	L(one_or_less)
# endif

# if defined USE_AS_STRCASECMP_L
	.section .rodata.cst32, "aM", @progbits, 32
	.align	32
L(lcase_min):
	.quad	0x4141414141414141
	.quad	0x4141414141414141
	.quad	0x4141414141414141
	.quad	0x4141414141414141
L(lcase_max):
	.quad	0x1a1a1a1a1a1a1a1a
	.quad	0x1a1a1a1a1a1a1a1a
	.quad	0x1a1a1a1a1a1a1a1a
	.quad	0x1a1a1a1a1a1a1a1a
L(case_add):
	.quad	0x2020202020202020
	.quad	0x2020202020202020
	.quad	0x2020202020202020
	.quad	0x2020202020202020
	.previous

	vmovdqa64 L(lcase_min)(%rip), LCASE_MIN_YMM
	vmovdqa64 L(lcase_max)(%rip), LCASE_MAX_YMM
	vmovdqa64 L(case_add)(%rip), CASE_ADD_YMM
# endif

	movl	%edi, %eax
	orl	%esi, %eax
	/* Shift out the bits irrelivant to page boundary ([63:12]).  */
	sall	$20, %eax
	/* Check if s1 or s2 may cross a page in next 4x VEC loads.  */
	cmpl	$((PAGE_SIZE -(VEC_SIZE * 4)) << 20), %eax
	ja	L(page_cross)

L(no_page_cross):
	/* Safe to compare 4x vectors.  */
	VMOVU	(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	/* Each bit cleared in K1 represents a mismatch or a null CHAR
	   in YMM0 and 32 bytes at (%rsi).  */
	CMP_R1_S2_YMM (%YMM0, (%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_STRNCMP
	cmpq	$CHAR_PER_VEC, %rdx
	jbe	L(vec_0_test_len)
# endif

	/* TESTEQ is `incl` for strcmp/strncmp and `subl $0xff` for
	   wcscmp/wcsncmp.  */

	/* All 1s represents all equals. TESTEQ will overflow to zero in
	   all equals case. Otherwise 1s will carry until position of first
	   mismatch.  */
	TESTEQ	%ecx
	jz	L(more_3x_vec)

	.p2align 4,, 4
L(return_vec_0):
	tzcntl	%ecx, %ecx
# ifdef USE_AS_WCSCMP
	movl	(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret0)
	setl	%al
	negl	%eax
	orl	$1, %eax
# else
	movzbl	(%rdi, %rcx), %eax
	movzbl	(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
# endif
L(ret0):
	ret

# ifdef USE_AS_STRNCMP
	.p2align 4,, 4
L(vec_0_test_len):
	notl	%ecx
	bzhil	%edx, %ecx, %eax
	jnz	L(return_vec_0)
	/* Align if will cross fetch block.  */
	.p2align 4,, 2
L(ret_zero):
	xorl	%eax, %eax
	ret

	.p2align 4,, 5
L(one_or_less):
#  ifdef USE_AS_STRCASECMP_L
	/* Set locale argument for strcasecmp.  */
	movq	%LOCALE_REG, %rdx
#  endif
	jb	L(ret_zero)
	/* 'nbe' covers the case where length is negative (large
	   unsigned).  */
	jnbe	OVERFLOW_STRCMP
#  ifdef USE_AS_WCSCMP
	movl	(%rdi), %edx
	xorl	%eax, %eax
	cmpl	(%rsi), %edx
	je	L(ret1)
	setl	%al
	negl	%eax
	orl	$1, %eax
#  else
	movzbl	(%rdi), %eax
	movzbl	(%rsi), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
#  endif
L(ret1):
	ret
# endif

	.p2align 4,, 10
L(return_vec_1):
	tzcntl	%ecx, %ecx
# ifdef USE_AS_STRNCMP
	/* rdx must be > CHAR_PER_VEC so its safe to subtract without
	   worrying about underflow.  */
	addq	$-CHAR_PER_VEC, %rdx
	cmpq	%rcx, %rdx
	jbe	L(ret_zero)
# endif
# ifdef USE_AS_WCSCMP
	movl	VEC_SIZE(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	VEC_SIZE(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret2)
	setl	%al
	negl	%eax
	orl	$1, %eax
# else
	movzbl	VEC_SIZE(%rdi, %rcx), %eax
	movzbl	VEC_SIZE(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
# endif
L(ret2):
	ret

	.p2align 4,, 10
# ifdef USE_AS_STRNCMP
L(return_vec_3):
#  if CHAR_PER_VEC <= 16
	sall	$CHAR_PER_VEC, %ecx
#  else
	salq	$CHAR_PER_VEC, %rcx
#  endif
# endif
L(return_vec_2):
# if (CHAR_PER_VEC <= 16) || !(defined USE_AS_STRNCMP)
	tzcntl	%ecx, %ecx
# else
	tzcntq	%rcx, %rcx
# endif

# ifdef USE_AS_STRNCMP
	cmpq	%rcx, %rdx
	jbe	L(ret_zero)
# endif

# ifdef USE_AS_WCSCMP
	movl	(VEC_SIZE * 2)(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	(VEC_SIZE * 2)(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret3)
	setl	%al
	negl	%eax
	orl	$1, %eax
# else
	movzbl	(VEC_SIZE * 2)(%rdi, %rcx), %eax
	movzbl	(VEC_SIZE * 2)(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
# endif
L(ret3):
	ret

# ifndef USE_AS_STRNCMP
	.p2align 4,, 10
L(return_vec_3):
	tzcntl	%ecx, %ecx
#  ifdef USE_AS_WCSCMP
	movl	(VEC_SIZE * 3)(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	(VEC_SIZE * 3)(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret4)
	setl	%al
	negl	%eax
	orl	$1, %eax
#  else
	movzbl	(VEC_SIZE * 3)(%rdi, %rcx), %eax
	movzbl	(VEC_SIZE * 3)(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
#  endif
L(ret4):
	ret
# endif

	/* 32 byte align here ensures the main loop is ideally aligned
	   for DSB.  */
	.p2align 5
L(more_3x_vec):
	/* Safe to compare 4x vectors.  */
	VMOVU	(VEC_SIZE)(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, VEC_SIZE(%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_1)

# ifdef USE_AS_STRNCMP
	subq	$(CHAR_PER_VEC * 2), %rdx
	jbe	L(ret_zero)
# endif

	VMOVU	(VEC_SIZE * 2)(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, (VEC_SIZE * 2)(%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_2)

	VMOVU	(VEC_SIZE * 3)(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, (VEC_SIZE * 3)(%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_3)

# ifdef USE_AS_STRNCMP
	cmpq	$(CHAR_PER_VEC * 2), %rdx
	jbe	L(ret_zero)
# endif


# ifdef USE_AS_WCSCMP
	/* any non-zero positive value that doesn't inference with 0x1.
	 */
	movl	$2, %r8d

# else
	xorl	%r8d, %r8d
# endif

	/* The prepare labels are various entry points from the page
	   cross logic.  */
L(prepare_loop):

# ifdef USE_AS_STRNCMP
#  ifdef USE_AS_WCSCMP
L(prepare_loop_no_len):
	movl	%edi, %ecx
	andl	$(VEC_SIZE * 4 - 1), %ecx
	shrl	$2, %ecx
	leaq	(CHAR_PER_VEC * 2)(%rdx, %rcx), %rdx
#  else
	/* Store N + (VEC_SIZE * 4) and place check at the begining of
	   the loop.  */
	leaq	(VEC_SIZE * 2)(%rdi, %rdx), %rdx
L(prepare_loop_no_len):
#  endif
# else
L(prepare_loop_no_len):
# endif

	/* Align s1 and adjust s2 accordingly.  */
	subq	%rdi, %rsi
	andq	$-(VEC_SIZE * 4), %rdi
L(prepare_loop_readj):
	addq	%rdi, %rsi
# if (defined USE_AS_STRNCMP) && !(defined USE_AS_WCSCMP)
	subq	%rdi, %rdx
# endif

L(prepare_loop_aligned):
	/* eax stores distance from rsi to next page cross. These cases
	   need to be handled specially as the 4x loop could potentially
	   read memory past the length of s1 or s2 and across a page
	   boundary.  */
	movl	$-(VEC_SIZE * 4), %eax
	subl	%esi, %eax
	andl	$(PAGE_SIZE - 1), %eax


	/* Loop 4x comparisons at a time.  */
	.p2align 4
L(loop):

	/* End condition for strncmp.  */
# ifdef USE_AS_STRNCMP
	subq	$(CHAR_PER_VEC * 4), %rdx
	jbe	L(ret_zero)
# endif

	subq	$-(VEC_SIZE * 4), %rdi
	subq	$-(VEC_SIZE * 4), %rsi

	/* Check if rsi loads will cross a page boundary.  */
	addl	$-(VEC_SIZE * 4), %eax
	jnb	L(page_cross_during_loop)

	/* Loop entry after handling page cross during loop.  */
L(loop_skip_page_cross_check):
	VMOVA	(VEC_SIZE * 0)(%rdi), %YMM0
	VMOVA	(VEC_SIZE * 1)(%rdi), %YMM2
	VMOVA	(VEC_SIZE * 2)(%rdi), %YMM4
	VMOVA	(VEC_SIZE * 3)(%rdi), %YMM6

	VPMINU	%YMM0, %YMM2, %YMM8
	VPMINU	%YMM4, %YMM6, %YMM9

	/* A zero CHAR in YMM9 means that there is a null CHAR.  */
	VPMINU	%YMM8, %YMM9, %YMM9

	/* Each bit set in K1 represents a non-null CHAR in YMM9.  */
	VPTESTM	%YMM9, %YMM9, %k1
# ifndef USE_AS_STRCASECMP_L
	vpxorq	(VEC_SIZE * 0)(%rsi), %YMM0, %YMM1
	vpxorq	(VEC_SIZE * 1)(%rsi), %YMM2, %YMM3
	vpxorq	(VEC_SIZE * 2)(%rsi), %YMM4, %YMM5
	/* Ternary logic to xor (VEC_SIZE * 3)(%rsi) with YMM6 while
	   oring with YMM1. Result is stored in YMM6.  */
	vpternlogd $0xde, (VEC_SIZE * 3)(%rsi), %YMM1, %YMM6
# else
	VMOVU	(VEC_SIZE * 0)(%rsi), %YMM1
	TOLOWER_YMM (%YMM0, %YMM1)
	VMOVU	(VEC_SIZE * 1)(%rsi), %YMM3
	TOLOWER_YMM (%YMM2, %YMM3)
	VMOVU	(VEC_SIZE * 2)(%rsi), %YMM5
	TOLOWER_YMM (%YMM4, %YMM5)
	VMOVU	(VEC_SIZE * 3)(%rsi), %YMM7
	TOLOWER_YMM (%YMM6, %YMM7)
	vpxorq	%YMM0, %YMM1, %YMM1
	vpxorq	%YMM2, %YMM3, %YMM3
	vpxorq	%YMM4, %YMM5, %YMM5
	vpternlogd $0xde, %YMM7, %YMM1, %YMM6
# endif
	/* Or together YMM3, YMM5, and YMM6.  */
	vpternlogd $0xfe, %YMM3, %YMM5, %YMM6


	/* A non-zero CHAR in YMM6 represents a mismatch.  */
	VPTESTNM %YMM6, %YMM6, %k0{%k1}
	kmovd	%k0, %LOOP_REG

	TESTEQ	%LOOP_REG
	jz	L(loop)


	/* Find which VEC has the mismatch of end of string.  */
	VPTESTM	%YMM0, %YMM0, %k1
	VPTESTNM %YMM1, %YMM1, %k0{%k1}
	kmovd	%k0, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_0_end)

	VPTESTM	%YMM2, %YMM2, %k1
	VPTESTNM %YMM3, %YMM3, %k0{%k1}
	kmovd	%k0, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_1_end)


	/* Handle VEC 2 and 3 without branches.  */
L(return_vec_2_3_end):
# ifdef USE_AS_STRNCMP
	subq	$(CHAR_PER_VEC * 2), %rdx
	jbe	L(ret_zero_end)
# endif

	VPTESTM	%YMM4, %YMM4, %k1
	VPTESTNM %YMM5, %YMM5, %k0{%k1}
	kmovd	%k0, %ecx
	TESTEQ	%ecx
# if CHAR_PER_VEC <= 16
	sall	$CHAR_PER_VEC, %LOOP_REG
	orl	%ecx, %LOOP_REG
# else
	salq	$CHAR_PER_VEC, %LOOP_REG64
	orq	%rcx, %LOOP_REG64
# endif
L(return_vec_3_end):
	/* LOOP_REG contains matches for null/mismatch from the loop. If
	   VEC 0,1,and 2 all have no null and no mismatches then mismatch
	   must entirely be from VEC 3 which is fully represented by
	   LOOP_REG.  */
# if CHAR_PER_VEC <= 16
	tzcntl	%LOOP_REG, %LOOP_REG
# else
	tzcntq	%LOOP_REG64, %LOOP_REG64
# endif
# ifdef USE_AS_STRNCMP
	cmpq	%LOOP_REG64, %rdx
	jbe	L(ret_zero_end)
# endif

# ifdef USE_AS_WCSCMP
	movl	(VEC_SIZE * 2)(%rdi, %LOOP_REG64, SIZE_OF_CHAR), %ecx
	xorl	%eax, %eax
	cmpl	(VEC_SIZE * 2)(%rsi, %LOOP_REG64, SIZE_OF_CHAR), %ecx
	je	L(ret5)
	setl	%al
	negl	%eax
	xorl	%r8d, %eax
# else
	movzbl	(VEC_SIZE * 2)(%rdi, %LOOP_REG64), %eax
	movzbl	(VEC_SIZE * 2)(%rsi, %LOOP_REG64), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	xorl	%r8d, %eax
	subl	%r8d, %eax
# endif
L(ret5):
	ret

# ifdef USE_AS_STRNCMP
	.p2align 4,, 2
L(ret_zero_end):
	xorl	%eax, %eax
	ret
# endif


	/* The L(return_vec_N_end) differ from L(return_vec_N) in that
	   they use the value of `r8` to negate the return value. This is
	   because the page cross logic can swap `rdi` and `rsi`.  */
	.p2align 4,, 10
# ifdef USE_AS_STRNCMP
L(return_vec_1_end):
#  if CHAR_PER_VEC <= 16
	sall	$CHAR_PER_VEC, %ecx
#  else
	salq	$CHAR_PER_VEC, %rcx
#  endif
# endif
L(return_vec_0_end):
# if (CHAR_PER_VEC <= 16) || !(defined USE_AS_STRNCMP)
	tzcntl	%ecx, %ecx
# else
	tzcntq	%rcx, %rcx
# endif

# ifdef USE_AS_STRNCMP
	cmpq	%rcx, %rdx
	jbe	L(ret_zero_end)
# endif

# ifdef USE_AS_WCSCMP
	movl	(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret6)
	setl	%al
	negl	%eax
	/* This is the non-zero case for `eax` so just xorl with `r8d`
	   flip is `rdi` and `rsi` where swapped.  */
	xorl	%r8d, %eax
# else
	movzbl	(%rdi, %rcx), %eax
	movzbl	(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	/* Flip `eax` if `rdi` and `rsi` where swapped in page cross
	   logic. Subtract `r8d` after xor for zero case.  */
	xorl	%r8d, %eax
	subl	%r8d, %eax
# endif
L(ret6):
	ret

# ifndef USE_AS_STRNCMP
	.p2align 4,, 10
L(return_vec_1_end):
	tzcntl	%ecx, %ecx
#  ifdef USE_AS_WCSCMP
	movl	VEC_SIZE(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	VEC_SIZE(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret7)
	setl	%al
	negl	%eax
	xorl	%r8d, %eax
#  else
	movzbl	VEC_SIZE(%rdi, %rcx), %eax
	movzbl	VEC_SIZE(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	xorl	%r8d, %eax
	subl	%r8d, %eax
#  endif
L(ret7):
	ret
# endif


	/* Page cross in rsi in next 4x VEC.  */

	/* TODO: Improve logic here.  */
	.p2align 4,, 10
L(page_cross_during_loop):
	/* eax contains [distance_from_page - (VEC_SIZE * 4)].  */

	/* Optimistically rsi and rdi and both aligned in which case we
	   don't need any logic here.  */
	cmpl	$-(VEC_SIZE * 4), %eax
	/* Don't adjust eax before jumping back to loop and we will
	   never hit page cross case again.  */
	je	L(loop_skip_page_cross_check)

	/* Check if we can safely load a VEC.  */
	cmpl	$-(VEC_SIZE * 3), %eax
	jle	L(less_1x_vec_till_page_cross)

	VMOVA	(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, (%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_0_end)

	/* if distance >= 2x VEC then eax > -(VEC_SIZE * 2).  */
	cmpl	$-(VEC_SIZE * 2), %eax
	jg	L(more_2x_vec_till_page_cross)

	.p2align 4,, 4
L(less_1x_vec_till_page_cross):
	subl	$-(VEC_SIZE * 4), %eax
	/* Guranteed safe to read from rdi - VEC_SIZE here. The only
	   concerning case is first iteration if incoming s1 was near start
	   of a page and s2 near end. If s1 was near the start of the page
	   we already aligned up to nearest VEC_SIZE * 4 so gurnateed safe
	   to read back -VEC_SIZE. If rdi is truly at the start of a page
	   here, it means the previous page (rdi - VEC_SIZE) has already
	   been loaded earlier so must be valid.  */
	VMOVU	-VEC_SIZE(%rdi, %rax), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, -VEC_SIZE(%rsi, %rax), %YMM1, %k1){%k2}
	/* Mask of potentially valid bits. The lower bits can be out of
	   range comparisons (but safe regarding page crosses).  */

# ifdef USE_AS_WCSCMP
	movl	$-1, %r10d
	movl	%esi, %ecx
	andl	$(VEC_SIZE - 1), %ecx
	shrl	$2, %ecx
	shlxl	%ecx, %r10d, %ecx
	movzbl	%cl, %r10d
# else
	movl	$-1, %ecx
	shlxl	%esi, %ecx, %r10d
# endif

	kmovd	%k1, %ecx
	notl	%ecx


# ifdef USE_AS_STRNCMP
#  ifdef USE_AS_WCSCMP
	/* NB: strcasecmp not used with WCSCMP so this access to r11 is
	   safe.  */
	movl	%eax, %r11d
	shrl	$2, %r11d
	cmpq	%r11, %rdx
#  else
	cmpq	%rax, %rdx
#  endif
	jbe	L(return_page_cross_end_check)
# endif
	movl	%eax, %OFFSET_REG

	/* Readjust eax before potentially returning to the loop.  */
	addl	$(PAGE_SIZE - VEC_SIZE * 4), %eax

	andl	%r10d, %ecx
	jz	L(loop_skip_page_cross_check)

	.p2align 4,, 3
L(return_page_cross_end):
	tzcntl	%ecx, %ecx

# if (defined USE_AS_STRNCMP) || (defined USE_AS_WCSCMP)
	leal	-VEC_SIZE(%OFFSET_REG64, %rcx, SIZE_OF_CHAR), %ecx
L(return_page_cross_cmp_mem):
# else
	addl	%OFFSET_REG, %ecx
# endif
# ifdef USE_AS_WCSCMP
	movl	VEC_OFFSET(%rdi, %rcx), %edx
	xorl	%eax, %eax
	cmpl	VEC_OFFSET(%rsi, %rcx), %edx
	je	L(ret8)
	setl	%al
	negl	%eax
	xorl	%r8d, %eax
# else
	movzbl	VEC_OFFSET(%rdi, %rcx), %eax
	movzbl	VEC_OFFSET(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	xorl	%r8d, %eax
	subl	%r8d, %eax
# endif
L(ret8):
	ret

# ifdef USE_AS_STRNCMP
	.p2align 4,, 10
L(return_page_cross_end_check):
	andl	%r10d, %ecx
	tzcntl	%ecx, %ecx
	leal	-VEC_SIZE(%rax, %rcx, SIZE_OF_CHAR), %ecx
#  ifdef USE_AS_WCSCMP
	sall	$2, %edx
#  endif
	cmpl	%ecx, %edx
	ja	L(return_page_cross_cmp_mem)
	xorl	%eax, %eax
	ret
# endif


	.p2align 4,, 10
L(more_2x_vec_till_page_cross):
	/* If more 2x vec till cross we will complete a full loop
	   iteration here.  */

	VMOVA	VEC_SIZE(%rdi), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, VEC_SIZE(%rsi), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_1_end)

# ifdef USE_AS_STRNCMP
	cmpq	$(CHAR_PER_VEC * 2), %rdx
	jbe	L(ret_zero_in_loop_page_cross)
# endif

	subl	$-(VEC_SIZE * 4), %eax

	/* Safe to include comparisons from lower bytes.  */
	VMOVU	-(VEC_SIZE * 2)(%rdi, %rax), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, -(VEC_SIZE * 2)(%rsi, %rax), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_page_cross_0)

	VMOVU	-(VEC_SIZE * 1)(%rdi, %rax), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, -(VEC_SIZE * 1)(%rsi, %rax), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(return_vec_page_cross_1)

# ifdef USE_AS_STRNCMP
	/* Must check length here as length might proclude reading next
	   page.  */
#  ifdef USE_AS_WCSCMP
	/* NB: strcasecmp not used with WCSCMP so this access to r11 is
	   safe.  */
	movl	%eax, %r11d
	shrl	$2, %r11d
	cmpq	%r11, %rdx
#  else
	cmpq	%rax, %rdx
#  endif
	jbe	L(ret_zero_in_loop_page_cross)
# endif

	/* Finish the loop.  */
	VMOVA	(VEC_SIZE * 2)(%rdi), %YMM4
	VMOVA	(VEC_SIZE * 3)(%rdi), %YMM6
	VPMINU	%YMM4, %YMM6, %YMM9
	VPTESTM	%YMM9, %YMM9, %k1
# ifndef USE_AS_STRCASECMP_L
	vpxorq	(VEC_SIZE * 2)(%rsi), %YMM4, %YMM5
	/* YMM6 = YMM5 | ((VEC_SIZE * 3)(%rsi) ^ YMM6).  */
	vpternlogd $0xde, (VEC_SIZE * 3)(%rsi), %YMM5, %YMM6
# else
	VMOVU	(VEC_SIZE * 2)(%rsi), %YMM5
	TOLOWER_YMM (%YMM4, %YMM5)
	VMOVU	(VEC_SIZE * 3)(%rsi), %YMM7
	TOLOWER_YMM (%YMM6, %YMM7)
	vpxorq	%YMM4, %YMM5, %YMM5
	vpternlogd $0xde, %YMM7, %YMM5, %YMM6
# endif
	VPTESTNM %YMM6, %YMM6, %k0{%k1}
	kmovd	%k0, %LOOP_REG
	TESTEQ	%LOOP_REG
	jnz	L(return_vec_2_3_end)

	/* Best for code size to include ucond-jmp here. Would be faster
	   if this case is hot to duplicate the L(return_vec_2_3_end) code
	   as fall-through and have jump back to loop on mismatch
	   comparison.  */
	subq	$-(VEC_SIZE * 4), %rdi
	subq	$-(VEC_SIZE * 4), %rsi
	addl	$(PAGE_SIZE - VEC_SIZE * 8), %eax
# ifdef USE_AS_STRNCMP
	subq	$(CHAR_PER_VEC * 4), %rdx
	ja	L(loop_skip_page_cross_check)
L(ret_zero_in_loop_page_cross):
	xorl	%eax, %eax
	ret
# else
	jmp	L(loop_skip_page_cross_check)
# endif


	.p2align 4,, 10
L(return_vec_page_cross_0):
	addl	$-VEC_SIZE, %eax
L(return_vec_page_cross_1):
	tzcntl	%ecx, %ecx
# if defined USE_AS_STRNCMP || defined USE_AS_WCSCMP
	leal	-VEC_SIZE(%rax, %rcx, SIZE_OF_CHAR), %ecx
#  ifdef USE_AS_STRNCMP
#   ifdef USE_AS_WCSCMP
	/* Must divide ecx instead of multiply rdx due to overflow.  */
	movl	%ecx, %eax
	shrl	$2, %eax
	cmpq	%rax, %rdx
#   else
	cmpq	%rcx, %rdx
#   endif
	jbe	L(ret_zero_in_loop_page_cross)
#  endif
# else
	addl	%eax, %ecx
# endif

# ifdef USE_AS_WCSCMP
	movl	VEC_OFFSET(%rdi, %rcx), %edx
	xorl	%eax, %eax
	cmpl	VEC_OFFSET(%rsi, %rcx), %edx
	je	L(ret9)
	setl	%al
	negl	%eax
	xorl	%r8d, %eax
# else
	movzbl	VEC_OFFSET(%rdi, %rcx), %eax
	movzbl	VEC_OFFSET(%rsi, %rcx), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	xorl	%r8d, %eax
	subl	%r8d, %eax
# endif
L(ret9):
	ret


	.p2align 4,, 10
L(page_cross):
# ifndef USE_AS_STRNCMP
	/* If both are VEC aligned we don't need any special logic here.
	   Only valid for strcmp where stop condition is guranteed to be
	   reachable by just reading memory.  */
	testl	$((VEC_SIZE - 1) << 20), %eax
	jz	L(no_page_cross)
# endif

	movl	%edi, %eax
	movl	%esi, %ecx
	andl	$(PAGE_SIZE - 1), %eax
	andl	$(PAGE_SIZE - 1), %ecx

	xorl	%OFFSET_REG, %OFFSET_REG

	/* Check which is closer to page cross, s1 or s2.  */
	cmpl	%eax, %ecx
	jg	L(page_cross_s2)

	/* The previous page cross check has false positives. Check for
	   true positive as page cross logic is very expensive.  */
	subl	$(PAGE_SIZE - VEC_SIZE * 4), %eax
	jbe	L(no_page_cross)


	/* Set r8 to not interfere with normal return value (rdi and rsi
	   did not swap).  */
# ifdef USE_AS_WCSCMP
	/* any non-zero positive value that doesn't inference with 0x1.
	 */
	movl	$2, %r8d
# else
	xorl	%r8d, %r8d
# endif

	/* Check if less than 1x VEC till page cross.  */
	subl	$(VEC_SIZE * 3), %eax
	jg	L(less_1x_vec_till_page)


	/* If more than 1x VEC till page cross, loop throuh safely
	   loadable memory until within 1x VEC of page cross.  */
	.p2align 4,, 8
L(page_cross_loop):
	VMOVU	(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, (%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %YMM1, %k1){%k2}
	kmovd	%k1, %ecx
	TESTEQ	%ecx
	jnz	L(check_ret_vec_page_cross)
	addl	$CHAR_PER_VEC, %OFFSET_REG
# ifdef USE_AS_STRNCMP
	cmpq	%OFFSET_REG64, %rdx
	jbe	L(ret_zero_page_cross)
# endif
	addl	$VEC_SIZE, %eax
	jl	L(page_cross_loop)

# ifdef USE_AS_WCSCMP
	shrl	$2, %eax
# endif


	subl	%eax, %OFFSET_REG
	/* OFFSET_REG has distance to page cross - VEC_SIZE. Guranteed
	   to not cross page so is safe to load. Since we have already
	   loaded at least 1 VEC from rsi it is also guranteed to be safe.
	 */
	VMOVU	(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %YMM0
	VPTESTM	%YMM0, %YMM0, %k2
	CMP_R1_S2_YMM (%YMM0, (%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %YMM1, %k1){%k2}

	kmovd	%k1, %ecx
# ifdef USE_AS_STRNCMP
	leal	CHAR_PER_VEC(%OFFSET_REG64), %eax
	cmpq	%rax, %rdx
	jbe	L(check_ret_vec_page_cross2)
#  ifdef USE_AS_WCSCMP
	addq	$-(CHAR_PER_VEC * 2), %rdx
#  else
	addq	%rdi, %rdx
#  endif
# endif
	TESTEQ	%ecx
	jz	L(prepare_loop_no_len)

	.p2align 4,, 4
L(ret_vec_page_cross):
# ifndef USE_AS_STRNCMP
L(check_ret_vec_page_cross):
# endif
	tzcntl	%ecx, %ecx
	addl	%OFFSET_REG, %ecx
L(ret_vec_page_cross_cont):
# ifdef USE_AS_WCSCMP
	movl	(%rdi, %rcx, SIZE_OF_CHAR), %edx
	xorl	%eax, %eax
	cmpl	(%rsi, %rcx, SIZE_OF_CHAR), %edx
	je	L(ret12)
	setl	%al
	negl	%eax
	xorl	%r8d, %eax
# else
	movzbl	(%rdi, %rcx, SIZE_OF_CHAR), %eax
	movzbl	(%rsi, %rcx, SIZE_OF_CHAR), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %ecx)
	subl	%ecx, %eax
	xorl	%r8d, %eax
	subl	%r8d, %eax
# endif
L(ret12):
	ret


# ifdef USE_AS_STRNCMP
	.p2align 4,, 10
L(check_ret_vec_page_cross2):
	TESTEQ	%ecx
L(check_ret_vec_page_cross):
	tzcntl	%ecx, %ecx
	addl	%OFFSET_REG, %ecx
	cmpq	%rcx, %rdx
	ja	L(ret_vec_page_cross_cont)
	.p2align 4,, 2
L(ret_zero_page_cross):
	xorl	%eax, %eax
	ret
# endif

	.p2align 4,, 4
L(page_cross_s2):
	/* Ensure this is a true page cross.  */
	subl	$(PAGE_SIZE - VEC_SIZE * 4), %ecx
	jbe	L(no_page_cross)


	movl	%ecx, %eax
	movq	%rdi, %rcx
	movq	%rsi, %rdi
	movq	%rcx, %rsi

	/* set r8 to negate return value as rdi and rsi swapped.  */
# ifdef USE_AS_WCSCMP
	movl	$-4, %r8d
# else
	movl	$-1, %r8d
# endif
	xorl	%OFFSET_REG, %OFFSET_REG

	/* Check if more than 1x VEC till page cross.  */
	subl	$(VEC_SIZE * 3), %eax
	jle	L(page_cross_loop)

	.p2align 4,, 6
L(less_1x_vec_till_page):
# ifdef USE_AS_WCSCMP
	shrl	$2, %eax
# endif
	/* Find largest load size we can use.  */
	cmpl	$(16 / SIZE_OF_CHAR), %eax
	ja	L(less_16_till_page)

	/* Use 16 byte comparison.  */
	vmovdqu	(%rdi), %xmm0
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_S2_XMM (%xmm0, (%rsi), %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xf, %ecx
# else
	incw	%cx
# endif
	jnz	L(check_ret_vec_page_cross)
	movl	$(16 / SIZE_OF_CHAR), %OFFSET_REG
# ifdef USE_AS_STRNCMP
	cmpq	%OFFSET_REG64, %rdx
	jbe	L(ret_zero_page_cross_slow_case0)
	subl	%eax, %OFFSET_REG
# else
	/* Explicit check for 16 byte alignment.  */
	subl	%eax, %OFFSET_REG
	jz	L(prepare_loop)
# endif
	vmovdqu	(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm0
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_S2_XMM (%xmm0, (%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0xf, %ecx
# else
	incw	%cx
# endif
	jnz	L(check_ret_vec_page_cross)
# ifdef USE_AS_STRNCMP
	addl	$(16 / SIZE_OF_CHAR), %OFFSET_REG
	subq	%OFFSET_REG64, %rdx
	jbe	L(ret_zero_page_cross_slow_case0)
	subq	$-(CHAR_PER_VEC * 4), %rdx

	leaq	-(VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	-(VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
# else
	leaq	(16 - VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	(16 - VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
# endif
	jmp	L(prepare_loop_aligned)

# ifdef USE_AS_STRNCMP
	.p2align 4,, 2
L(ret_zero_page_cross_slow_case0):
	xorl	%eax, %eax
	ret
# endif


	.p2align 4,, 10
L(less_16_till_page):
	cmpl	$(24 / SIZE_OF_CHAR), %eax
	ja	L(less_8_till_page)

	/* Use 8 byte comparison.  */
	vmovq	(%rdi), %xmm0
	vmovq	(%rsi), %xmm1
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_R2_XMM (%xmm0, %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0x3, %ecx
# else
	incb	%cl
# endif
	jnz	L(check_ret_vec_page_cross)


# ifdef USE_AS_STRNCMP
	cmpq	$(8 / SIZE_OF_CHAR), %rdx
	jbe	L(ret_zero_page_cross_slow_case0)
# endif
	movl	$(24 / SIZE_OF_CHAR), %OFFSET_REG
	subl	%eax, %OFFSET_REG

	vmovq	(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm0
	vmovq	(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm1
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_R2_XMM (%xmm0, %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
# ifdef USE_AS_WCSCMP
	subl	$0x3, %ecx
# else
	incb	%cl
# endif
	jnz	L(check_ret_vec_page_cross)


# ifdef USE_AS_STRNCMP
	addl	$(8 / SIZE_OF_CHAR), %OFFSET_REG
	subq	%OFFSET_REG64, %rdx
	jbe	L(ret_zero_page_cross_slow_case0)
	subq	$-(CHAR_PER_VEC * 4), %rdx

	leaq	-(VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	-(VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
# else
	leaq	(8 - VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	(8 - VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
# endif
	jmp	L(prepare_loop_aligned)




	.p2align 4,, 10
L(less_8_till_page):
# ifdef USE_AS_WCSCMP
	/* If using wchar then this is the only check before we reach
	   the page boundary.  */
	movl	(%rdi), %eax
	movl	(%rsi), %ecx
	cmpl	%ecx, %eax
	jnz	L(ret_less_8_wcs)
#  ifdef USE_AS_STRNCMP
	addq	$-(CHAR_PER_VEC * 2), %rdx
	/* We already checked for len <= 1 so cannot hit that case here.
	 */
#  endif
	testl	%eax, %eax
	jnz	L(prepare_loop)
	ret

	.p2align 4,, 8
L(ret_less_8_wcs):
	setl	%OFFSET_REG8
	negl	%OFFSET_REG
	movl	%OFFSET_REG, %eax
	xorl	%r8d, %eax
	ret

# else
	cmpl	$28, %eax
	ja	L(less_4_till_page)

	vmovd	(%rdi), %xmm0
	vmovd	(%rsi), %xmm1
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_R2_XMM (%xmm0, %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
	subl	$0xf, %ecx
	jnz	L(check_ret_vec_page_cross)

#  ifdef USE_AS_STRNCMP
	cmpq	$4, %rdx
	jbe	L(ret_zero_page_cross_slow_case1)
#  endif
	movl	$(28 / SIZE_OF_CHAR), %OFFSET_REG
	subl	%eax, %OFFSET_REG

	vmovd	(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm0
	vmovd	(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %xmm1
	VPTESTM	%xmm0, %xmm0, %k2
	CMP_R1_R2_XMM (%xmm0, %xmm1, %k1){%k2}
	kmovd	%k1, %ecx
	subl	$0xf, %ecx
	jnz	L(check_ret_vec_page_cross)
#  ifdef USE_AS_STRNCMP
	addl	$(4 / SIZE_OF_CHAR), %OFFSET_REG
	subq	%OFFSET_REG64, %rdx
	jbe	L(ret_zero_page_cross_slow_case1)
	subq	$-(CHAR_PER_VEC * 4), %rdx

	leaq	-(VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	-(VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
#  else
	leaq	(4 - VEC_SIZE * 4)(%rdi, %OFFSET_REG64, SIZE_OF_CHAR), %rdi
	leaq	(4 - VEC_SIZE * 4)(%rsi, %OFFSET_REG64, SIZE_OF_CHAR), %rsi
#  endif
	jmp	L(prepare_loop_aligned)


#  ifdef USE_AS_STRNCMP
	.p2align 4,, 2
L(ret_zero_page_cross_slow_case1):
	xorl	%eax, %eax
	ret
#  endif

	.p2align 4,, 10
L(less_4_till_page):
	subq	%rdi, %rsi
	/* Extremely slow byte comparison loop.  */
L(less_4_loop):
	movzbl	(%rdi), %eax
	movzbl	(%rsi, %rdi), %ecx
	TOLOWER_gpr (%rax, %eax)
	TOLOWER_gpr (%rcx, %BYTE_LOOP_REG)
	subl	%BYTE_LOOP_REG, %eax
	jnz	L(ret_less_4_loop)
	testl	%ecx, %ecx
	jz	L(ret_zero_4_loop)
#  ifdef USE_AS_STRNCMP
	decq	%rdx
	jz	L(ret_zero_4_loop)
#  endif
	incq	%rdi
	/* end condition is reach page boundary (rdi is aligned).  */
	testl	$31, %edi
	jnz	L(less_4_loop)
	leaq	-(VEC_SIZE * 4)(%rdi, %rsi), %rsi
	addq	$-(VEC_SIZE * 4), %rdi
#  ifdef USE_AS_STRNCMP
	subq	$-(CHAR_PER_VEC * 4), %rdx
#  endif
	jmp	L(prepare_loop_aligned)

L(ret_zero_4_loop):
	xorl	%eax, %eax
	ret
L(ret_less_4_loop):
	xorl	%r8d, %eax
	subl	%r8d, %eax
	ret
# endif
	cfi_endproc
	.size	STRCMP, .-STRCMP
#endif
