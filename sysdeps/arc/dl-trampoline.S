/* PLT trampolines.  ARC version.
   Copyright (C) 2005-2019 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <libc-symbols.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Save the registers which resolver could possibly clobber
	r0-r9: args to the function - symbol being resolved
	r10-r12 are already clobbered by PLTn, PLT0 thus neednot be saved.  */

.macro	SAVE_CALLER_SAVED
	push_s	r0
	push_s	r1
	push_s	r2
	push_s	r3
	st.a	r4, [sp, -4]
	st.a	r5, [sp, -4]
	st.a	r6, [sp, -4]
	st.a	r7, [sp, -4]
	st.a	r8, [sp, -4]
	st.a	r9, [sp, -4]
	cfi_adjust_cfa_offset (40)
	push_s	blink
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (blink, 0)
.endm

.macro RESTORE_CALLER_SAVED_BUT_R0
	ld.ab	blink,[sp, 4]
	cfi_adjust_cfa_offset (-4)
	cfi_restore (blink)
	ld.ab	r9, [sp, 4]
	ld.ab	r8, [sp, 4]
	ld.ab	r7, [sp, 4]
	ld.ab	r6, [sp, 4]
	ld.ab	r5, [sp, 4]
	ld.ab	r4, [sp, 4]
	pop_s   r3
	pop_s   r2
	pop_s   r1
	cfi_adjust_cfa_offset (-36)
.endm

/* Upon entry, PLTn, which led us here, sets up the following regs
	r11 = Module info (tpnt pointer as expected by resolver)
	r12 = PC of the PLTn itself - needed by resolver to find
	      corresponding .rela.plt entry.  */

ENTRY (_dl_runtime_resolve)
	; args to func being resolved, which resolver might clobber
	SAVE_CALLER_SAVED

	mov_s 	r1, r12
	bl.d  	_dl_fixup
	mov   	r0, r11

	RESTORE_CALLER_SAVED_BUT_R0
	j_s.d   [r0]    /* r0 has resolved function addr.  */
	pop_s   r0      /* restore first arg to resolved call.  */
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r0)
END (_dl_runtime_resolve)
