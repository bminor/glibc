/* longjmp for ARC.
   Copyright (C) 2017-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <jmpbuf-offsets.h>

;@ r0 = jump buffer from which regs will be restored
;@ r1 = value that setjmp( ) will return due to this longjmp

ENTRY (__longjmp)

	ld_s r13,   [r0]
	ld_s r14,   [r0,4]
	ld   r15,   [r0,8]
	ld   r16,   [r0,12]
	ld   r17,   [r0,16]
	ld   r18,   [r0,20]
	ld   r19,   [r0,24]
	ld   r20,   [r0,28]
	ld   r21,   [r0,32]
	ld   r22,   [r0,36]
	ld   r23,   [r0,40]
	ld   r24,   [r0,44]
	ld   r25,   [r0,48]

	ld   blink, [r0,60]
	ld   fp,    [r0,52]
	ld   sp,    [r0,56]

	mov.f  r0, r1	; get the setjmp return value(due to longjmp) in place

	j.d    [blink]	; to caller of setjmp location, right after the call
	mov.z  r0, 1	; can't let setjmp return 0 when it is due to longjmp

END (__longjmp)
