/* Thread-local storage handling in the ELF dynamic linker.
   AArch64 Morello version.
   Copyright (C) 2011-2022 Free Software Foundation, Inc.

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

#include <sysdep.h>
#include <tls.h>
#include "tlsdesc.h"

#define NSAVEDQREGPAIRS	16
#define SAVE_Q_REGISTERS				\
	stp	q0, q1,	[csp, #-32*NSAVEDQREGPAIRS]!;	\
	cfi_adjust_cfa_offset (32*NSAVEDQREGPAIRS);	\
	stp	 q2,  q3, [csp, #32*1];			\
	stp	 q4,  q5, [csp, #32*2];			\
	stp	 q6,  q7, [csp, #32*3];			\
	stp	 q8,  q9, [csp, #32*4];			\
	stp	q10, q11, [csp, #32*5];			\
	stp	q12, q13, [csp, #32*6];			\
	stp	q14, q15, [csp, #32*7];			\
	stp	q16, q17, [csp, #32*8];			\
	stp	q18, q19, [csp, #32*9];			\
	stp	q20, q21, [csp, #32*10];		\
	stp	q22, q23, [csp, #32*11];		\
	stp	q24, q25, [csp, #32*12];		\
	stp	q26, q27, [csp, #32*13];		\
	stp	q28, q29, [csp, #32*14];		\
	stp	q30, q31, [csp, #32*15];

#define RESTORE_Q_REGISTERS				\
	ldp	 q2,  q3, [csp, #32*1];			\
	ldp	 q4,  q5, [csp, #32*2];			\
	ldp	 q6,  q7, [csp, #32*3];			\
	ldp	 q8,  q9, [csp, #32*4];			\
	ldp	q10, q11, [csp, #32*5];			\
	ldp	q12, q13, [csp, #32*6];			\
	ldp	q14, q15, [csp, #32*7];			\
	ldp	q16, q17, [csp, #32*8];			\
	ldp	q18, q19, [csp, #32*9];			\
	ldp	q20, q21, [csp, #32*10];		\
	ldp	q22, q23, [csp, #32*11];		\
	ldp	q24, q25, [csp, #32*12];		\
	ldp	q26, q27, [csp, #32*13];		\
	ldp	q28, q29, [csp, #32*14];		\
	ldp	q30, q31, [csp, #32*15];		\
	ldp	 q0,  q1, [csp], #32*NSAVEDQREGPAIRS;	\
	cfi_adjust_cfa_offset (-32*NSAVEDQREGPAIRS);

	.text

	/* Compute the address for symbols in the static TLS block.
	   Prototype:
	   _dl_tlsdesc_return (tlsdesc *tdp, void *unused, void *tp);
	 */
	.hidden _dl_tlsdesc_return
	.global	_dl_tlsdesc_return
	.type	_dl_tlsdesc_return,%function
	cfi_startproc
	.align 2
_dl_tlsdesc_return:
	ldp	x0, x1, [c0, #PTR_SIZE] /* Load offset, size.  */
	add	c0, c2, x0
	scbndse	c0, c0, x1
	RET
	cfi_endproc
	.size	_dl_tlsdesc_return, .-_dl_tlsdesc_return

	/* Handler for undefined weak TLS symbols: returns NULL.
	   Prototype:
	   _dl_tlsdesc_undefweak (tlsdesc *tdp, void *unused, void *tp);
	 */
	.hidden _dl_tlsdesc_undefweak
	.global	_dl_tlsdesc_undefweak
	.type	_dl_tlsdesc_undefweak,%function
	cfi_startproc
	.align  2
_dl_tlsdesc_undefweak:
	mov	x0, 0
	RET
	cfi_endproc
	.size	_dl_tlsdesc_undefweak, .-_dl_tlsdesc_undefweak

#ifdef SHARED
	/* Handler for dynamic TLS symbols.
	   Prototype:
	   _dl_tlsdesc_dynamic (tlsdesc *tdp, void *unused, void *tp);

	   The second word of the descriptor points to a
	   tlsdesc_dynamic_arg structure.

	   Returns the address of the tls object.

	   void *
	   _dl_tlsdesc_dynamic (struct tlsdesc *tdp, void *unused, void *tp)
	   {
	     struct tlsdesc_dynamic_arg *td = tdp->arg;
	     dtv_t *dtv = *(dtv_t **)((char *)tp + TCBHEAD_DTV);
	     if (__builtin_expect (td->gen_count <= dtv[0].counter
		&& (dtv[td->tlsinfo.ti_module].pointer.val
		    != TLS_DTV_UNALLOCATED),
		1))
	       return dtv[td->tlsinfo.ti_module].pointer.val
		+ td->tlsinfo.ti_offset;

	     return ___tls_get_addr (&td->tlsinfo);
	   }
	 */
	.hidden _dl_tlsdesc_dynamic
	.global	_dl_tlsdesc_dynamic
	.type	_dl_tlsdesc_dynamic,%function
	cfi_startproc
	.align 2
_dl_tlsdesc_dynamic:

	/* Save just enough registers to support fast path, if we fall
	   into slow path we will save additional registers.  */
	stp	c3, c4, [csp, #-32]!
	cfi_adjust_cfa_offset (32)
	cfi_rel_offset (c3, 0)
	cfi_rel_offset (c4, 16)

	ldr	c1, [c0,#TLSDESC_ARG]
	ldr	c0, [c2,#TCBHEAD_DTV]
	ldr	x3, [c1,#TLSDESC_GEN_COUNT]
	ldr	x4, [c0,#DTV_COUNTER]
	cmp	x3, x4
	b.hi	2f
	/* Load r3 = td->tlsinfo.ti_module and r4 = td->tlsinfo.ti_offset.  */
	ldp	x3, x4, [c1,#TLSDESC_MODID]
	lsl	x3, x3, #(PTR_LOG_SIZE+1)
	ldr	c0, [c0, x3] /* Load val member of DTV entry.  */
	cmp	x0, #TLS_DTV_UNALLOCATED
	b.eq	2f
	cfi_remember_state
	/* Load r3 = td->tlsinfo.ti_size.  */
	ldr	x3, [c1, #TLSDESC_SIZE]
	add	c0, c0, x4
	scbndse	c0, c0, x3
1:
	ldp	c3, c4, [csp], #32
	cfi_adjust_cfa_offset (-32)
	RET
2:
	/* This is the slow path. We need to call __tls_get_addr() which
	   means we need to save and restore all the register that the
	   callee will trash.  */

	/* Save the remaining registers that we must treat as caller save.  */
	cfi_restore_state

# define NSAVEXREGPAIRS 9
	stp	c29, c30, [csp,#-32*NSAVEXREGPAIRS]!
	cfi_adjust_cfa_offset (32*NSAVEXREGPAIRS)
	cfi_rel_offset (c29, 0)
	cfi_rel_offset (c30, 16)
	mov	c29, csp
	stp	 c5,  c6, [csp, #32*1]
	stp	 c7,  c8, [csp, #32*2]
	stp	 c9, c10, [csp, #32*3]
	stp	c11, c12, [csp, #32*4]
	stp	c13, c14, [csp, #32*5]
	stp	c15, c16, [csp, #32*6]
	stp	c17, c18, [csp, #32*7]
	cfi_rel_offset (c5, 32*1)
	cfi_rel_offset (c6, 32*1+8)
	cfi_rel_offset (c7, 32*2)
	cfi_rel_offset (c8, 32*2+8)
	cfi_rel_offset (c9, 32*3)
	cfi_rel_offset (c10, 32*3+8)
	cfi_rel_offset (c11, 32*4)
	cfi_rel_offset (c12, 32*4+8)
	cfi_rel_offset (c13, 32*5)
	cfi_rel_offset (c14, 32*5+8)
	cfi_rel_offset (c15, 32*6)
	cfi_rel_offset (c16, 32*6+8)
	cfi_rel_offset (c17, 32*7)
	cfi_rel_offset (c18, 32*7+8)

	SAVE_Q_REGISTERS

	mov	c0, c1
	bl	__tls_get_addr

	mrs	c2, ctpidr_el0 /* Restore c2.  */

	RESTORE_Q_REGISTERS

	ldp	 c5,  c6, [csp, #32*1]
	ldp	 c7,  c8, [csp, #32*2]
	ldp	 c9, c10, [csp, #32*3]
	ldp	c11, c12, [csp, #32*4]
	ldp	c13, c14, [csp, #32*5]
	ldp	c15, c16, [csp, #32*6]
	ldp	c17, c18, [csp, #32*7]

	ldp	c29, c30, [csp], #32*NSAVEXREGPAIRS
	cfi_adjust_cfa_offset (-32*NSAVEXREGPAIRS)
	cfi_restore (c29)
	cfi_restore (c30)

	b	1b
	cfi_endproc
	.size	_dl_tlsdesc_dynamic, .-_dl_tlsdesc_dynamic
# undef NSAVEXREGPAIRS
#endif
