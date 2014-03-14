/* PLT trampolines.  ARM/NaCl version.
   Copyright (C) 2014 Free Software Foundation, Inc.
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

	.syntax unified
	.text

@ Change &GOT[n+3] into 8*n.  Note relocs are 8 bytes each.
.macro compute_reloc_arg pltgot, got2
	sub r1, \pltgot, \got2	@ r1 = &GOT[n+3] - &GOT[2] = 4*(n-1)
	sub r1, r1, #4		@ r1 = 4*n
	add r1, r1, r1		@ r1 *= 2 = 8*n
.endm

	CFI_SECTIONS
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, %function
	.p2align 4
_dl_runtime_resolve:
	cfi_startproc
	cfi_adjust_cfa_offset (8)

	@ We get called with:
	@ 	lr contains the return address from this call
	@	stack[1] contains &GOT[n+3] (pointer to function)
	@	stack[0] points to &GOT[2]

	ldr ip, [sp]		@ ip gets &GOT[2]

	@ Save the argument registers and the return address.
	@ r4 doesn't need to be saved, but it makes the total
	@ adjustment to sp (including the two words pushed by
	@ the PLT code) an even eight words, so sp stays aligned.
	push {r0-r4, lr}
	cfi_adjust_cfa_offset (24)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r2, 8)
	cfi_rel_offset (r3, 12)
	cfi_rel_offset (r4, 16)
	cfi_rel_offset (lr, 20)

	ldr r1, [sp, #28]	@ r1 gets &GOT[n+3]

	@ Get the 'struct link_map *' for the first argument to _dl_fixup.
	sfi_breg ip, ldr r0, [\B, #-4]

	@ Get the reloc offset for the second argument to _dl_fixup.
	compute_reloc_arg r1, ip

	@ This does the real work, and returns the real call target.
	sfi_bl _dl_fixup
	mov ip, r0

	@ Restore the saved registers.
	pop {r0-r4, lr}
	cfi_adjust_cfa_offset (-24)
	cfi_restore (r0)
	cfi_restore (r1)
	cfi_restore (r2)
	cfi_restore (r3)
	cfi_restore (r4)
	cfi_restore (lr)

	@ Now compensate for the two words pushed by the PLT code.
	sfi_sp add sp, #8
	cfi_adjust_cfa_offset (-8)

	@ Finally, jump to the newfound call target.
	sfi_bx ip

	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve

#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	.p2align 4
_dl_runtime_profile:
	cfi_startproc
	cfi_adjust_cfa_offset (8)
	@ XXX tbd
	sfi_trap
	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous
