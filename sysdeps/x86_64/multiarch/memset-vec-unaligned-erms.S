/* memset/bzero with unaligned store and rep stosb
   Copyright (C) 2016-2022 Free Software Foundation, Inc.
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

/* memset is implemented as:
   1. Use overlapping store to avoid branch.
   2. If size is less than VEC, use integer register stores.
   3. If size is from VEC_SIZE to 2 * VEC_SIZE, use 2 VEC stores.
   4. If size is from 2 * VEC_SIZE to 4 * VEC_SIZE, use 4 VEC stores.
   5. If size is more to 4 * VEC_SIZE, align to 4 * VEC_SIZE with
      4 VEC stores and store 4 * VEC at a time until done.  */

#include <sysdep.h>

#ifndef MEMSET_CHK_SYMBOL
# define MEMSET_CHK_SYMBOL(p,s)		MEMSET_SYMBOL(p, s)
#endif

#ifndef WMEMSET_CHK_SYMBOL
# define WMEMSET_CHK_SYMBOL(p,s)	WMEMSET_SYMBOL(p, s)
#endif

#ifndef XMM0
# define XMM0				xmm0
#endif

#ifndef YMM0
# define YMM0				ymm0
#endif

#ifndef VZEROUPPER
# if VEC_SIZE > 16
#  define VZEROUPPER			vzeroupper
#  define VZEROUPPER_SHORT_RETURN	vzeroupper; ret
# else
#  define VZEROUPPER
# endif
#endif

#ifndef VZEROUPPER_SHORT_RETURN
# define VZEROUPPER_SHORT_RETURN	rep; ret
#endif

#ifndef MOVQ
# if VEC_SIZE > 16
#  define MOVQ				vmovq
# else
#  define MOVQ				movq
# endif
#endif

#if VEC_SIZE == 64
# define LOOP_4X_OFFSET	(VEC_SIZE * 4)
#else
# define LOOP_4X_OFFSET	(0)
#endif

#if defined USE_WITH_EVEX || defined USE_WITH_AVX512
# define END_REG	rcx
# define LOOP_REG	rdi
#else
# define END_REG	rdi
# define LOOP_REG	rdx
#endif

#define PAGE_SIZE 4096

/* Macro to calculate size of small memset block for aligning
   purposes.  */
#define SMALL_MEMSET_ALIGN(mov_sz,	ret_sz)	(2 * (mov_sz) + (ret_sz) + 1)


#ifndef SECTION
# error SECTION is not defined!
#endif

	.section SECTION(.text),"ax",@progbits
#if VEC_SIZE == 16 && IS_IN (libc)
ENTRY (__bzero)
	mov	%RDI_LP, %RAX_LP /* Set return value.  */
	mov	%RSI_LP, %RDX_LP /* Set n.  */
	xorl	%esi, %esi
	pxor	%XMM0, %XMM0
	jmp	L(entry_from_bzero)
END (__bzero)
weak_alias (__bzero, bzero)
#endif

#if IS_IN (libc)
# if defined SHARED
ENTRY_CHK (WMEMSET_CHK_SYMBOL (__wmemset_chk, unaligned))
	cmp	%RDX_LP, %RCX_LP
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (WMEMSET_CHK_SYMBOL (__wmemset_chk, unaligned))
# endif

ENTRY (WMEMSET_SYMBOL (__wmemset, unaligned))
	shl	$2, %RDX_LP
	WMEMSET_VDUP_TO_VEC0_AND_SET_RETURN (%esi, %rdi)
	jmp	L(entry_from_bzero)
END (WMEMSET_SYMBOL (__wmemset, unaligned))
#endif

#if defined SHARED && IS_IN (libc)
ENTRY_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned))
	cmp	%RDX_LP, %RCX_LP
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned))
#endif

ENTRY (MEMSET_SYMBOL (__memset, unaligned))
	MEMSET_VDUP_TO_VEC0_AND_SET_RETURN (%esi, %rdi)
# ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	mov	%edx, %edx
# endif
L(entry_from_bzero):
	cmpq	$VEC_SIZE, %rdx
	jb	L(less_vec)
	cmpq	$(VEC_SIZE * 2), %rdx
	ja	L(more_2x_vec)
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE.  */
	VMOVU	%VEC(0), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(0), (%rdi)
	VZEROUPPER_RETURN
#if defined USE_MULTIARCH && IS_IN (libc)
END (MEMSET_SYMBOL (__memset, unaligned))

# if VEC_SIZE == 16
ENTRY (__memset_chk_erms)
	cmp	%RDX_LP, %RCX_LP
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (__memset_chk_erms)

/* Only used to measure performance of REP STOSB.  */
ENTRY (__memset_erms)
	/* Skip zero length.  */
	test	%RDX_LP, %RDX_LP
	jnz	 L(stosb)
	movq	%rdi, %rax
	ret
# else
/* Provide a hidden symbol to debugger.  */
	.hidden	MEMSET_SYMBOL (__memset, erms)
ENTRY (MEMSET_SYMBOL (__memset, erms))
# endif
L(stosb):
	mov	%RDX_LP, %RCX_LP
	movzbl	%sil, %eax
	mov	%RDI_LP, %RDX_LP
	rep stosb
	mov	%RDX_LP, %RAX_LP
	VZEROUPPER_RETURN
# if VEC_SIZE == 16
END (__memset_erms)
# else
END (MEMSET_SYMBOL (__memset, erms))
# endif

# if defined SHARED && IS_IN (libc)
ENTRY_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned_erms))
	cmp	%RDX_LP, %RCX_LP
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned_erms))
# endif

ENTRY_P2ALIGN (MEMSET_SYMBOL (__memset, unaligned_erms), 6)
	MEMSET_VDUP_TO_VEC0_AND_SET_RETURN (%esi, %rdi)
# ifdef __ILP32__
	/* Clear the upper 32 bits.  */
	mov	%edx, %edx
# endif
	cmp	$VEC_SIZE, %RDX_LP
	jb	L(less_vec)
	cmp	$(VEC_SIZE * 2), %RDX_LP
	ja	L(stosb_more_2x_vec)
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE.
	 */
	VMOVU	%VEC(0), (%rax)
	VMOVU	%VEC(0), -VEC_SIZE(%rax, %rdx)
	VZEROUPPER_RETURN
#endif

	.p2align 4,, 10
L(last_2x_vec):
#ifdef USE_LESS_VEC_MASK_STORE
	VMOVU	%VEC(0), (VEC_SIZE * 2 + LOOP_4X_OFFSET)(%rcx)
	VMOVU	%VEC(0), (VEC_SIZE * 3 + LOOP_4X_OFFSET)(%rcx)
#else
	VMOVU	%VEC(0), (VEC_SIZE * -2)(%rdi)
	VMOVU	%VEC(0), (VEC_SIZE * -1)(%rdi)
#endif
	VZEROUPPER_RETURN

	/* If have AVX512 mask instructions put L(less_vec) close to
	   entry as it doesn't take much space and is likely a hot target.
	 */
#ifdef USE_LESS_VEC_MASK_STORE
	.p2align 4,, 10
L(less_vec):
	/* Less than 1 VEC.  */
# if VEC_SIZE != 16 && VEC_SIZE != 32 && VEC_SIZE != 64
#  error Unsupported VEC_SIZE!
# endif
	/* Clear high bits from edi. Only keeping bits relevant to page
	   cross check. Note that we are using rax which is set in
	   MEMSET_VDUP_TO_VEC0_AND_SET_RETURN as ptr from here on out.  */
	andl	$(PAGE_SIZE - 1), %edi
	/* Check if VEC_SIZE store cross page. Mask stores suffer
	   serious performance degradation when it has to fault supress.
	 */
	cmpl	$(PAGE_SIZE - VEC_SIZE), %edi
	/* This is generally considered a cold target.  */
	ja	L(cross_page)
# if VEC_SIZE > 32
	movq	$-1, %rcx
	bzhiq	%rdx, %rcx, %rcx
	kmovq	%rcx, %k1
# else
	movl	$-1, %ecx
	bzhil	%edx, %ecx, %ecx
	kmovd	%ecx, %k1
# endif
	vmovdqu8 %VEC(0), (%rax){%k1}
	VZEROUPPER_RETURN

# if defined USE_MULTIARCH && IS_IN (libc)
	/* Include L(stosb_local) here if including L(less_vec) between
	   L(stosb_more_2x_vec) and ENTRY. This is to cache align the
	   L(stosb_more_2x_vec) target.  */
	.p2align 4,, 10
L(stosb_local):
	movzbl	%sil, %eax
	mov	%RDX_LP, %RCX_LP
	mov	%RDI_LP, %RDX_LP
	rep	stosb
	mov	%RDX_LP, %RAX_LP
	VZEROUPPER_RETURN
# endif
#endif

#if defined USE_MULTIARCH && IS_IN (libc)
	.p2align 4
L(stosb_more_2x_vec):
	cmp	__x86_rep_stosb_threshold(%rip), %RDX_LP
	ja	L(stosb_local)
#endif
	/* Fallthrough goes to L(loop_4x_vec). Tests for memset (2x, 4x]
	   and (4x, 8x] jump to target.  */
L(more_2x_vec):

	/* Two different methods of setting up pointers / compare. The
	   two methods are based on the fact that EVEX/AVX512 mov
	   instructions take more bytes then AVX2/SSE2 mov instructions. As
	   well that EVEX/AVX512 machines also have fast LEA_BID. Both
	   setup and END_REG to avoid complex address mode. For EVEX/AVX512
	   this saves code size and keeps a few targets in one fetch block.
	   For AVX2/SSE2 this helps prevent AGU bottlenecks.  */
#if defined USE_WITH_EVEX || defined USE_WITH_AVX512
	/* If EVEX/AVX512 compute END_REG - (VEC_SIZE * 4 +
	   LOOP_4X_OFFSET) with LEA_BID.  */

	/* END_REG is rcx for EVEX/AVX512.  */
	leaq	-(VEC_SIZE * 4 + LOOP_4X_OFFSET)(%rdi, %rdx), %END_REG
#endif

	/* Stores to first 2x VEC before cmp as any path forward will
	   require it.  */
	VMOVU	%VEC(0), (%rax)
	VMOVU	%VEC(0), VEC_SIZE(%rax)


#if !(defined USE_WITH_EVEX || defined USE_WITH_AVX512)
	/* If AVX2/SSE2 compute END_REG (rdi) with ALU.  */
	addq	%rdx, %END_REG
#endif

	cmpq	$(VEC_SIZE * 4), %rdx
	jbe	L(last_2x_vec)

	/* Store next 2x vec regardless.  */
	VMOVU	%VEC(0), (VEC_SIZE * 2)(%rax)
	VMOVU	%VEC(0), (VEC_SIZE * 3)(%rax)


#if defined USE_WITH_EVEX || defined USE_WITH_AVX512
	/* If LOOP_4X_OFFSET don't readjust LOOP_REG (rdi), just add
	   extra offset to addresses in loop. Used for AVX512 to save space
	   as no way to get (VEC_SIZE * 4) in imm8.  */
# if LOOP_4X_OFFSET == 0
	subq	$-(VEC_SIZE * 4), %LOOP_REG
# endif
	/* Avoid imm32 compare here to save code size.  */
	cmpq	%rdi, %rcx
#else
	addq	$-(VEC_SIZE * 4), %END_REG
	cmpq	$(VEC_SIZE * 8), %rdx
#endif
	jbe	L(last_4x_vec)
#if !(defined USE_WITH_EVEX || defined USE_WITH_AVX512)
	/* Set LOOP_REG (rdx).  */
	leaq	(VEC_SIZE * 4)(%rax), %LOOP_REG
#endif
	/* Align dst for loop.  */
	andq	$(VEC_SIZE * -2), %LOOP_REG
	.p2align 4
L(loop):
	VMOVA	%VEC(0), LOOP_4X_OFFSET(%LOOP_REG)
	VMOVA	%VEC(0), (VEC_SIZE + LOOP_4X_OFFSET)(%LOOP_REG)
	VMOVA	%VEC(0), (VEC_SIZE * 2 + LOOP_4X_OFFSET)(%LOOP_REG)
	VMOVA	%VEC(0), (VEC_SIZE * 3 + LOOP_4X_OFFSET)(%LOOP_REG)
	subq	$-(VEC_SIZE * 4), %LOOP_REG
	cmpq	%END_REG, %LOOP_REG
	jb	L(loop)
	.p2align 4,, MOV_SIZE
L(last_4x_vec):
	VMOVU	%VEC(0), LOOP_4X_OFFSET(%END_REG)
	VMOVU	%VEC(0), (VEC_SIZE + LOOP_4X_OFFSET)(%END_REG)
	VMOVU	%VEC(0), (VEC_SIZE * 2 + LOOP_4X_OFFSET)(%END_REG)
	VMOVU	%VEC(0), (VEC_SIZE * 3 + LOOP_4X_OFFSET)(%END_REG)
L(return):
#if VEC_SIZE > 16
	ZERO_UPPER_VEC_REGISTERS_RETURN
#else
	ret
#endif

	.p2align 4,, 10
#ifndef USE_LESS_VEC_MASK_STORE
# if defined USE_MULTIARCH && IS_IN (libc)
	/* If no USE_LESS_VEC_MASK put L(stosb_local) here. Will be in
	   range for 2-byte jump encoding.  */
L(stosb_local):
	movzbl	%sil, %eax
	mov	%RDX_LP, %RCX_LP
	mov	%RDI_LP, %RDX_LP
	rep	stosb
	mov	%RDX_LP, %RAX_LP
	VZEROUPPER_RETURN
# endif
	/* Define L(less_vec) only if not otherwise defined.  */
	.p2align 4
L(less_vec):
#endif
L(cross_page):
#if VEC_SIZE > 32
	cmpl	$32, %edx
	jae	L(between_32_63)
#endif
#if VEC_SIZE > 16
	cmpl	$16, %edx
	jae	L(between_16_31)
#endif
	MOVQ	%XMM0, %rdi
	cmpl	$8, %edx
	jae	L(between_8_15)
	cmpl	$4, %edx
	jae	L(between_4_7)
	cmpl	$1, %edx
	ja	L(between_2_3)
	jb	L(return)
	movb	%sil, (%rax)
	VZEROUPPER_RETURN

	/* Align small targets only if not doing so would cross a fetch
	   line.  */
#if VEC_SIZE > 32
	.p2align 4,, SMALL_MEMSET_ALIGN(MOV_SIZE, RET_SIZE)
	/* From 32 to 63.  No branch when size == 32.  */
L(between_32_63):
	VMOVU	%YMM0, (%rax)
	VMOVU	%YMM0, -32(%rax, %rdx)
	VZEROUPPER_RETURN
#endif

#if VEC_SIZE >= 32
	.p2align 4,, SMALL_MEMSET_ALIGN(MOV_SIZE, RET_SIZE)
L(between_16_31):
	/* From 16 to 31.  No branch when size == 16.  */
	VMOVU	%XMM0, (%rax)
	VMOVU	%XMM0, -16(%rax, %rdx)
	VZEROUPPER_RETURN
#endif

	.p2align 4,, SMALL_MEMSET_ALIGN(3, RET_SIZE)
L(between_8_15):
	/* From 8 to 15.  No branch when size == 8.  */
	movq	%rdi, (%rax)
	movq	%rdi, -8(%rax, %rdx)
	VZEROUPPER_RETURN

	.p2align 4,, SMALL_MEMSET_ALIGN(2, RET_SIZE)
L(between_4_7):
	/* From 4 to 7.  No branch when size == 4.  */
	movl	%edi, (%rax)
	movl	%edi, -4(%rax, %rdx)
	VZEROUPPER_RETURN

	.p2align 4,, SMALL_MEMSET_ALIGN(3, RET_SIZE)
L(between_2_3):
	/* From 2 to 3.  No branch when size == 2.  */
	movw	%di, (%rax)
	movb	%dil, -1(%rax, %rdx)
	VZEROUPPER_RETURN
END (MEMSET_SYMBOL (__memset, unaligned_erms))
