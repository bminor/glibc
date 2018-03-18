/* Copyright (C) 2000-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
     Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

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

#include <sysdeps/generic/sysdep.h>

#ifdef __ASSEMBLER__

/* Macros to help writing .prologue directives in assembly code.  */
#define ASM_UNW_PRLG_RP			0x8
#define ASM_UNW_PRLG_PFS		0x4
#define ASM_UNW_PRLG_PSP		0x2
#define ASM_UNW_PRLG_PR			0x1
#define ASM_UNW_PRLG_GRSAVE(ninputs)	(32+(ninputs))

#define ENTRY(name)				\
	.text;					\
	.align 32;				\
	.proc C_SYMBOL_NAME(name);		\
	.global C_SYMBOL_NAME(name);		\
	C_LABEL(name)				\
	CALL_MCOUNT

#define LOCAL_ENTRY(name)			\
	.text;					\
	.align 32;				\
	.proc C_SYMBOL_NAME(name);		\
	C_LABEL(name)				\
	CALL_MCOUNT

#define LEAF(name)				\
  .text;					\
  .align 32;					\
  .proc C_SYMBOL_NAME(name);			\
  .global name;					\
  C_LABEL(name)

#define LOCAL_LEAF(name)			\
  .text;					\
  .align 32;					\
  .proc C_SYMBOL_NAME(name);			\
  C_LABEL(name)

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.endp C_SYMBOL_NAME(sym)

/* Make a "sibling call" to DEST -- that is, transfer control to DEST
   as-if it had been the function called by the caller of this function.
   DEST is likely to be defined in a different shared object.  Only
   ever used immediately after ENTRY.  Must not touch the stack at
   all, and must preserve all argument and call-saved registers.  */
#undef SIBCALL
#define SIBCALL(dest)				\
  .mib; nop 0; nop 0; br.sptk.many C_SYMBOL_NAME(dest);;

#endif /* ASSEMBLER */
