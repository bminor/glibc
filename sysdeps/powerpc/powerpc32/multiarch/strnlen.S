/* Copyright (C) 2013 Free Software Foundation, Inc.
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
#include <rtld-global-offsets.h>

/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc
	.text
ENTRY(__strnlen)
	.type   __strnlen, @gnu_indirect_function
# ifdef PIC
	mflr	r6
	cfi_register (lr,r6)
	SETUP_GOT_ACCESS (r5,got_label)
	addis	r5,r5,_GLOBAL_OFFSET_TABLE_-got_label@ha
	addi	r5,r5,_GLOBAL_OFFSET_TABLE_-got_label@l
	mtlr	r6
	cfi_same_value (lr)
#  ifdef SHARED
	lwz	r6,_rtld_global_ro@got(r5)
	/* If _rtld_global_ro is not initialized use the default ppc32
	   implementation.  */
	cmplwi  r6,0
	beq	L(ppc32)
	lwz	r6,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET+4(r6)
#  else
	lwz     r6,_dl_hwcap@got(r5)
	lwz	r6,4(r6)
#  endif
# else /* PIC  */
	lis	r6,(_dl_hwcap+4)@ha
	lwz	r6,(_dl_hwcap+4)@l(r6)
# endif
	andi.	r7,r6,PPC_FEATURE_HAS_VSX
	bne	L(power7)
L(ppc32):
# ifdef PIC
	lwz	r3,__strnlen_ppc32@got(r5)
# else
	lis	r3,__strnlen_ppc32@ha
	lwz	r3,__strnlen_ppc32@l(r3)
# endif
	blr
L(power7):
# ifdef PIC
	lwz	r3,__strnlen_power7@got(r5)
# else
	lis	r3,__strnlen_power7@ha
	lwz	r3,__strnlen_power7@l(r3)
# endif
	blr
END(__strnlen)

weak_alias (__strnlen, strnlen)
#endif
