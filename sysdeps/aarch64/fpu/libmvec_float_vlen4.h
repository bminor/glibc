/* Single-precision 4 element vector function template.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

ENTRY (VECTOR_FUNCTION)
	stp	x29, x30, [sp, -288]!
	cfi_adjust_cfa_offset (288)
	cfi_rel_offset (x29, 0)
	cfi_rel_offset (x30, 8)
	mov	x29, sp
	stp	 q8,  q9, [sp, 16]
	stp	q10, q11, [sp, 48]
	stp	q12, q13, [sp, 80]
	stp	q14, q15, [sp, 112]
	stp	q16, q17, [sp, 144]
	stp	q18, q19, [sp, 176]
	stp	q20, q21, [sp, 208]
	stp	q22, q23, [sp, 240]

	// Use per lane load/store to avoid endianness issues.
	str	q0, [sp, 272]
	ldr	s0, [sp, 272]
	bl SCALAR_FUNCTION
	str	s0, [sp, 272]
	ldr	s0, [sp, 276]
	bl SCALAR_FUNCTION
	str	s0, [sp, 276]
	ldr	s0, [sp, 280]
	bl SCALAR_FUNCTION
	str	s0, [sp, 280]
	ldr	s0, [sp, 284]
	bl SCALAR_FUNCTION
	str	s0, [sp, 284]
	ldr	q0, [sp, 272]

	ldp	q8, q9, [sp, 16]
	ldp	q10, q11, [sp, 48]
	ldp	q12, q13, [sp, 80]
	ldp	q14, q15, [sp, 112]
	ldp	q16, q17, [sp, 144]
	ldp	q18, q19, [sp, 176]
	ldp	q20, q21, [sp, 208]
	ldp	q22, q23, [sp, 240]
	ldp	x29, x30, [sp], 288
	cfi_adjust_cfa_offset (288)
	cfi_restore (x29)
	cfi_restore (x30)
	ret
END (VECTOR_FUNCTION)
