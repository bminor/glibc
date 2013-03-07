/* Optimized memset implementation for PowerPC32.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

/* Let other functions reuse the code of this file.  */
#ifdef MEMSET
# define memset MEMSET
#endif

#ifndef INTERNAL_MEMSET
# define INTERNAL_MEMSET memset
#endif

#define CONCAT(x, y) __ ## x ## _ ## y
#define EVALUATE(x, y) CONCAT(x, y)
#define OPTIM(NAME) EVALUATE(memset, NAME)

#include <sysdep.h>
#include <rtld-global-offsets.h>

/* Define multiple versions only for the definition in libc.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(INTERNAL_MEMSET)
	.type   memset, @gnu_indirect_function
# ifdef PIC
	mflr	r11
	cfi_register (lr,r11)
	bcl 	20,31,1f
1:	mflr	r5
	addis	r5,r5,_GLOBAL_OFFSET_TABLE_-1b@ha
	addi	r5,r5,_GLOBAL_OFFSET_TABLE_-1b@l
	lwz	r6,_rtld_global_ro@got(r5)
	mtlr	r11
	cfi_same_value (lr)
	lwz	r6,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET+4(r6)
# else
	lis	r6,(_dl_hwcap+4)@ha
	lwz	r6,(_dl_hwcap+4)@l(r6)
# endif
	/* r5 - got pointer | r6 - _dl_hwcap */
	andi.	r7,r6,PPC_FEATURE_HAS_VSX
	bne-	L(power7)
	andi.	r7,r6,PPC_FEATURE_ARCH_2_05
	bne-	L(power6)
	andis.	r7,r6,(PPC_FEATURE_POWER4>>16)
	bne-	L(power4)
# ifdef PIC
	lwz	r3,OPTIM(ppc32)@got(r5)
# else
	lis	r3,OPTIM(ppc32)@ha
	lwz	r3,OPTIM(ppc32)@l(r3)
# endif
	blr
L(power7):
# ifdef PIC
	lwz	r3,OPTIM(power7)@got(r5)
# else
	lis	r3,OPTIM(power7)@ha
	lwz	r3,OPTIM(power7)@l(r3)
# endif
	blr
L(power6):
# ifdef PIC
	lwz	r3,OPTIM(power6)@got(r5)
# else
	lis	r3,OPTIM(power6)@ha
	lwz	r3,OPTIM(power6)@l(r3)
# endif
	blr
L(power4):
# ifdef PIC
	lwz	r3,OPTIM(power4)@got(r5)
# else
	lis	r3,OPTIM(power4)@ha
	lwz	r3,OPTIM(power4)@l(r3)
# endif
	blr
END(INTERNAL_MEMSET)

# undef EALIGN
# define EALIGN(name, alignt, words)                           \
  .globl C_SYMBOL_NAME(OPTIM(ppc32));                          \
  .type C_SYMBOL_NAME(OPTIM(ppc32)),@function ;                \
  .align ALIGNARG(alignt);                                     \
  EALIGN_W_##words;                                            \
  C_LABEL(OPTIM(ppc32))                                        \
  cfi_startproc;

# undef END
# define END(name)                                             \
  cfi_endproc;                                                 \
  ASM_SIZE_DIRECTIVE(OPTIM(ppc32))

# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)                         \
  .globl EVALUATE(GI, memset); 	EVALUATE(GI, memset) = OPTIM(ppc32)

#endif

#ifndef MEMSET
# include "../memset.S"
#endif
