/* Compare two memory blocks for differences in the first COUNT bytes.
   Copyright (C) 2004-2013 Free Software Foundation, Inc.
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
#include "asm-syntax.h"

#define PARMS		4+4	/* Preserve EBX.  */
#define BLK1		PARMS
#define BLK2		BLK1+4
#define LEN		BLK2+4
#define ENTRANCE	pushl %ebx; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (ebx, 0)
#define RETURN		popl %ebx; cfi_adjust_cfa_offset (-4); \
			cfi_restore (ebx); ret

/* Load an entry in a jump table into EBX.  TABLE is a jump table
   with relative offsets.  INDEX is a register contains the index
   into the jump table.  */
#define LOAD_JUMP_TABLE_ENTRY(TABLE, INDEX) \
  /* We first load PC into EBX.  */					      \
  SETUP_PIC_REG(bx);							      \
  /* Get the address of the jump table.  */				      \
  addl	$(TABLE - .), %ebx;						      \
  /* Get the entry and convert the relative offset to the		      \
     absolute address.  */						      \
  addl	(%ebx,INDEX,4), %ebx

        .text
	ALIGN (4)
ENTRY (memcmp)
	ENTRANCE

	movl	BLK1(%esp), %eax
	movl	BLK2(%esp), %edx
	movl	LEN(%esp), %ecx
#ifdef __CHKP__
	bndldx	BLK1(%esp,%eax,1), %bnd0
	bndldx	BLK2(%esp,%edx,1), %bnd1
#endif

	cmpl 	$1, %ecx
	jne	L(not_1)
#ifdef __CHKP__
	bndcl	(%eax), %bnd0
	bndcu	(%eax), %bnd0
	bndcl	(%edx), %bnd1
	bndcu	(%edx), %bnd1
#endif
	movzbl	(%eax), %ecx		/* LEN == 1  */
	cmpb	(%edx), %cl
	jne	L(neq)
L(bye):
	xorl	%eax, %eax
	RETURN

	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
L(neq):
	sbbl	%eax, %eax
	sbbl	$-1, %eax
	RETURN

	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
L(not_1):
	jl	L(bye)			/* LEN == 0  */
#ifdef __CHKP__
	bndcl	(%eax), %bnd0
	bndcu	(%eax), %bnd0
	bndcl	(%edx), %bnd1
	bndcu	(%edx), %bnd1
#endif

	pushl	%esi
	cfi_adjust_cfa_offset (4)
	movl	%eax, %esi
	cfi_rel_offset (esi, 0)
	cmpl	$32, %ecx;
	jge	L(32bytesormore)	/* LEN => 32  */

	LOAD_JUMP_TABLE_ENTRY (L(table_32bytes), %ecx)
	addl	%ecx, %edx
	addl	%ecx, %esi
	jmp	*%ebx

	ALIGN (4)
L(28bytes):
#ifdef __CHKP__
	bndcu	-28(%esi), %bnd0
	bndcu	-28(%edx), %bnd1
#endif
	movl	-28(%esi), %eax
	movl	-28(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(24bytes):
#ifdef __CHKP__
	bndcu	-24(%esi), %bnd0
	bndcu	-24(%edx), %bnd1
#endif
	movl	-24(%esi), %eax
	movl	-24(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(20bytes):
#ifdef __CHKP__
	bndcu	-20(%esi), %bnd0
	bndcu	-20(%edx), %bnd1
#endif
	movl	-20(%esi), %eax
	movl	-20(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(16bytes):
#ifdef __CHKP__
	bndcu	-16(%esi), %bnd0
	bndcu	-16(%edx), %bnd1
#endif
	movl	-16(%esi), %eax
	movl	-16(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(12bytes):
#ifdef __CHKP__
	bndcu	-12(%esi), %bnd0
	bndcu	-12(%edx), %bnd1
#endif
	movl	-12(%esi), %eax
	movl	-12(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(8bytes):
#ifdef __CHKP__
	bndcu	-8(%esi), %bnd0
	bndcu	-8(%edx), %bnd1
#endif
	movl	-8(%esi), %eax
	movl	-8(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(4bytes):
#ifdef __CHKP__
	bndcu	-4(%esi), %bnd0
	bndcu	-4(%edx), %bnd1
#endif
	movl	-4(%esi), %eax
	movl	-4(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(0bytes):
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	xorl	%eax, %eax
	RETURN

	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (esi, 0)
	cfi_rel_offset (ebx, 4)
L(29bytes):
#ifdef __CHKP__
	bndcu	-29(%esi), %bnd0
	bndcu	-29(%edx), %bnd1
#endif
	movl	-29(%esi), %eax
	movl	-29(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(25bytes):
#ifdef __CHKP__
	bndcu	-25(%esi), %bnd0
	bndcu	-25(%edx), %bnd1
#endif
	movl	-25(%esi), %eax
	movl	-25(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(21bytes):
#ifdef __CHKP__
	bndcu	-21(%esi), %bnd0
	bndcu	-21(%edx), %bnd1
#endif
	movl	-21(%esi), %eax
	movl	-21(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(17bytes):
#ifdef __CHKP__
	bndcu	-17(%esi), %bnd0
	bndcu	-17(%edx), %bnd1
#endif
	movl	-17(%esi), %eax
	movl	-17(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(13bytes):
#ifdef __CHKP__
	bndcu	-13(%esi), %bnd0
	bndcu	-13(%edx), %bnd1
#endif
	movl	-13(%esi), %eax
	movl	-13(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(9bytes):
#ifdef __CHKP__
	bndcu	-9(%esi), %bnd0
	bndcu	-9(%edx), %bnd1
#endif
	movl	-9(%esi), %eax
	movl	-9(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(5bytes):
#ifdef __CHKP__
	bndcu	-5(%esi), %bnd0
	bndcu	-5(%edx), %bnd1
#endif
	movl	-5(%esi), %eax
	movl	-5(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(1bytes):
#ifdef __CHKP__
	bndcu	-1(%esi), %bnd0
	bndcu	-1(%edx), %bnd1
#endif
	movzbl	-1(%esi), %eax
	cmpb	-1(%edx), %al
	jne	L(set)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	xorl	%eax, %eax
	RETURN

	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (esi, 0)
	cfi_rel_offset (ebx, 4)
L(30bytes):
#ifdef __CHKP__
	bndcu	-30(%esi), %bnd0
	bndcu	-30(%edx), %bnd1
#endif
	movl	-30(%esi), %eax
	movl	-30(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(26bytes):
#ifdef __CHKP__
	bndcu	-26(%esi), %bnd0
	bndcu	-26(%edx), %bnd1
#endif
	movl	-26(%esi), %eax
	movl	-26(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(22bytes):
#ifdef __CHKP__
	bndcu	-22(%esi), %bnd0
	bndcu	-22(%edx), %bnd1
#endif
	movl	-22(%esi), %eax
	movl	-22(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(18bytes):
#ifdef __CHKP__
	bndcu	-18(%esi), %bnd0
	bndcu	-18(%edx), %bnd1
#endif
	movl	-18(%esi), %eax
	movl	-18(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(14bytes):
#ifdef __CHKP__
	bndcu	-14(%esi), %bnd0
	bndcu	-14(%edx), %bnd1
#endif
	movl	-14(%esi), %eax
	movl	-14(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(10bytes):
#ifdef __CHKP__
	bndcu	-10(%esi), %bnd0
	bndcu	-10(%edx), %bnd1
#endif
	movl	-10(%esi), %eax
	movl	-10(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(6bytes):
#ifdef __CHKP__
	bndcu	-6(%esi), %bnd0
	bndcu	-6(%edx), %bnd1
#endif
	movl	-6(%esi), %eax
	movl	-6(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(2bytes):
#ifdef __CHKP__
	bndcu	-2(%esi), %bnd0
	bndcu	-2(%edx), %bnd1
#endif
	movzwl	-2(%esi), %eax
	movzwl	-2(%edx), %ecx
	cmpb	%cl, %al
	jne	L(set)
	cmpl	%ecx, %eax
	jne	L(set)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	xorl	%eax, %eax
	RETURN

	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (esi, 0)
	cfi_rel_offset (ebx, 4)
L(31bytes):
#ifdef __CHKP__
	bndcu	-31(%esi), %bnd0
	bndcu	-31(%edx), %bnd1
#endif
	movl	-31(%esi), %eax
	movl	-31(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(27bytes):
#ifdef __CHKP__
	bndcu	-27(%esi), %bnd0
	bndcu	-27(%edx), %bnd1
#endif
	movl	-27(%esi), %eax
	movl	-27(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(23bytes):
#ifdef __CHKP__
	bndcu	-23(%esi), %bnd0
	bndcu	-23(%edx), %bnd1
#endif
	movl	-23(%esi), %eax
	movl	-23(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(19bytes):
#ifdef __CHKP__
	bndcu	-19(%esi), %bnd0
	bndcu	-19(%edx), %bnd1
#endif
	movl	-19(%esi), %eax
	movl	-19(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(15bytes):
#ifdef __CHKP__
	bndcu	-15(%esi), %bnd0
	bndcu	-15(%edx), %bnd1
#endif
	movl	-15(%esi), %eax
	movl	-15(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(11bytes):
#ifdef __CHKP__
	bndcu	-11(%esi), %bnd0
	bndcu	-11(%edx), %bnd1
#endif
	movl	-11(%esi), %eax
	movl	-11(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(7bytes):
#ifdef __CHKP__
	bndcu	-7(%esi), %bnd0
	bndcu	-7(%edx), %bnd1
#endif
	movl	-7(%esi), %eax
	movl	-7(%edx), %ecx
	cmpl	%ecx, %eax
	jne	L(find_diff)
L(3bytes):
#ifdef __CHKP__
	bndcu	-3(%esi), %bnd0
	bndcu	-3(%edx), %bnd1
#endif
	movzwl	-3(%esi), %eax
	movzwl	-3(%edx), %ecx
	cmpb	%cl, %al
	jne	L(set)
	cmpl	%ecx, %eax
	jne	L(set)
	movzbl	-1(%esi), %eax
	cmpb	-1(%edx), %al
	jne	L(set)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	xorl	%eax, %eax
	RETURN

	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (esi, 0)
	cfi_rel_offset (ebx, 4)
	ALIGN (4)
/* ECX >= 32.  */
L(32bytesormore):
	subl	$32, %ecx

#ifdef __CHKP__
	bndcu	(%esi), %bnd0
	bndcu	(%edx), %bnd1
#endif
	movl	(%esi), %eax
	cmpl	(%edx), %eax
	jne	L(load_ecx)

#ifdef __CHKP__
	bndcu	4(%esi), %bnd0
	bndcu	4(%edx), %bnd1
#endif
	movl	4(%esi), %eax
	cmpl	4(%edx), %eax
	jne	L(load_ecx_4)

#ifdef __CHKP__
	bndcu	8(%esi), %bnd0
	bndcu	8(%edx), %bnd1
#endif
	movl	8(%esi), %eax
	cmpl	8(%edx), %eax
	jne	L(load_ecx_8)

#ifdef __CHKP__
	bndcu	12(%esi), %bnd0
	bndcu	12(%edx), %bnd1
#endif
	movl	12(%esi), %eax
	cmpl	12(%edx), %eax
	jne	L(load_ecx_12)

#ifdef __CHKP__
	bndcu	16(%esi), %bnd0
	bndcu	16(%edx), %bnd1
#endif
	movl	16(%esi), %eax
	cmpl	16(%edx), %eax
	jne	L(load_ecx_16)

#ifdef __CHKP__
	bndcu	20(%esi), %bnd0
	bndcu	20(%edx), %bnd1
#endif
	movl	20(%esi), %eax
	cmpl	20(%edx), %eax
	jne	L(load_ecx_20)

#ifdef __CHKP__
	bndcu	24(%esi), %bnd0
	bndcu	24(%edx), %bnd1
#endif
	movl	24(%esi), %eax
	cmpl	24(%edx), %eax
	jne	L(load_ecx_24)

#ifdef __CHKP__
	bndcu	28(%esi), %bnd0
	bndcu	28(%edx), %bnd1
#endif
	movl	28(%esi), %eax
	cmpl	28(%edx), %eax
	jne	L(load_ecx_28)

	addl	$32, %esi
	addl	$32, %edx
	cmpl	$32, %ecx
	jge	L(32bytesormore)

	LOAD_JUMP_TABLE_ENTRY (L(table_32bytes), %ecx)
	addl	%ecx, %edx
	addl	%ecx, %esi
	jmp	*%ebx

L(load_ecx_28):
	addl	$0x4, %edx
L(load_ecx_24):
	addl	$0x4, %edx
L(load_ecx_20):
	addl	$0x4, %edx
L(load_ecx_16):
	addl	$0x4, %edx
L(load_ecx_12):
	addl	$0x4, %edx
L(load_ecx_8):
	addl	$0x4, %edx
L(load_ecx_4):
	addl	$0x4, %edx
L(load_ecx):
	movl	(%edx), %ecx

L(find_diff):
	cmpb	%cl, %al
	jne	L(set)
	cmpb	%ch, %ah
	jne	L(set)
	shrl	$16,%eax
	shrl	$16,%ecx
	cmpb	%cl, %al
	jne	L(set)
	/* We get there only if we already know there is a
	   difference.  */
	cmpl	%ecx, %eax
L(set):
	sbbl	%eax, %eax
	sbbl	$-1, %eax
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	RETURN
END (memcmp)

	.section	.rodata
	ALIGN (2)
L(table_32bytes) :
	.long	L(0bytes) - L(table_32bytes)
	.long	L(1bytes) - L(table_32bytes)
	.long	L(2bytes) - L(table_32bytes)
	.long	L(3bytes) - L(table_32bytes)
	.long	L(4bytes) - L(table_32bytes)
	.long	L(5bytes) - L(table_32bytes)
	.long	L(6bytes) - L(table_32bytes)
	.long	L(7bytes) - L(table_32bytes)
	.long	L(8bytes) - L(table_32bytes)
	.long	L(9bytes) - L(table_32bytes)
	.long	L(10bytes) - L(table_32bytes)
	.long	L(11bytes) - L(table_32bytes)
	.long	L(12bytes) - L(table_32bytes)
	.long	L(13bytes) - L(table_32bytes)
	.long	L(14bytes) - L(table_32bytes)
	.long	L(15bytes) - L(table_32bytes)
	.long	L(16bytes) - L(table_32bytes)
	.long	L(17bytes) - L(table_32bytes)
	.long	L(18bytes) - L(table_32bytes)
	.long	L(19bytes) - L(table_32bytes)
	.long	L(20bytes) - L(table_32bytes)
	.long	L(21bytes) - L(table_32bytes)
	.long	L(22bytes) - L(table_32bytes)
	.long	L(23bytes) - L(table_32bytes)
	.long	L(24bytes) - L(table_32bytes)
	.long	L(25bytes) - L(table_32bytes)
	.long	L(26bytes) - L(table_32bytes)
	.long	L(27bytes) - L(table_32bytes)
	.long	L(28bytes) - L(table_32bytes)
	.long	L(29bytes) - L(table_32bytes)
	.long	L(30bytes) - L(table_32bytes)
	.long	L(31bytes) - L(table_32bytes)


#undef bcmp
weak_alias (memcmp, bcmp)
libc_hidden_builtin_def (memcmp)
