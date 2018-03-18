/* Copyright (C) 1992-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.

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

#include <sysdeps/unix/mips/sysdep.h>

#ifdef __ASSEMBLER__

/* Note that while it's better structurally, going back to call __syscall_error
   can make things confusing if you're debugging---it looks like it's jumping
   backwards into the previous fn.  */
#ifdef __PIC__
#define PSEUDO(name, syscall_name, args) \
  .align 2;								      \
  .set nomips16;							      \
  cfi_startproc;							      \
  99:;									      \
  .set noat;								      \
  .cpsetup t9, $1, name;						      \
  cfi_register (gp, $1);						      \
  .set at;								      \
  la t9,__syscall_error;						      \
  .cpreturn;								      \
  cfi_restore (gp);							      \
  jr t9;								      \
  cfi_endproc;								      \
  ENTRY(name)								      \
  li v0, SYS_ify(syscall_name);						      \
  syscall;								      \
  bne a3, zero, 99b;							      \
L(syse1):
#else
#define PSEUDO(name, syscall_name, args) \
  .set noreorder;							      \
  .align 2;								      \
  .set nomips16;							      \
  cfi_startproc;							      \
  99: j __syscall_error;						      \
  nop;                                                                        \
  cfi_endproc;								      \
  ENTRY(name)								      \
  .set noreorder;							      \
  li v0, SYS_ify(syscall_name);						      \
  syscall;								      \
  .set reorder;								      \
  bne a3, zero, 99b;							      \
L(syse1):
#endif

/* Make a "sibling call" to DEST -- that is, transfer control to DEST
   as-if it had been the function called by the caller of this function.
   DEST is likely to be defined in a different shared object.  Only
   ever used immediately after ENTRY.  Must not touch the stack at
   all, and must preserve all argument and call-saved registers.  */
#ifdef __PIC__
/* There's no PLT on MIPS/n32, we are expected to load the address by
   hand, but the usual gp register on MIPS ($28) is call-saved so we
   can't use it.  Use $at ($1) instead.  */
#define SIBCALL(dest)							      \
  .set	nomips16;							      \
  .set	noreorder;							      \
  .set	nomacro;							      \
  .set	noat;								      \
0:	lui	$1,	%hi(%neg(%gp_rel(0b)));				      \
	addiu	$1, $1, %lo(%neg(%gp_rel(0b)));				      \
	addu	$1, $1, $25;						      \
	lw	$1, %call16(dest)($1);					      \
	nop;								      \
	.reloc	1f, R_MIPS_JALR, dest;					      \
1:	jr	$25;							      \
	nop
#else
#define SIBCALL(dest)							      \
  .set	nomips16;							      \
  .set	noreorder;							      \
  .set	nomacro;							      \
	j	dest;							      \
	nop
#endif

#endif /* __ASSEMBLER__ */
