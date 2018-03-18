/* Assembler macros for x86.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#ifndef _X86_SYSDEP_H
#define _X86_SYSDEP_H 1

#include <sysdeps/generic/sysdep.h>

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

/* ELF uses byte-counts for .align, most others use log2 of count of bytes.  */
#define ALIGNARG(log2) 1<<log2
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  .globl C_SYMBOL_NAME(name);						      \
  .type C_SYMBOL_NAME(name),@function;					      \
  .align ALIGNARG(4);							      \
  C_LABEL(name)								      \
  cfi_startproc;							      \
  CALL_MCOUNT

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)

#define ENTRY_CHK(name) ENTRY (name)
#define END_CHK(name) END (name)

/* Make a "sibling call" to DEST -- that is, transfer control to DEST
   as-if it had been the function called by the caller of this function.
   DEST is likely to be defined in a different shared object.  Only
   ever used immediately after ENTRY.  Must not touch the stack at
   all, and must preserve all argument and call-saved registers.  */
#undef SIBCALL
#define SIBCALL(dest)                           \
  jmp JUMPTARGET(dest)

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#define mcount		_mcount

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  END (name)

/* Local label name for asm code. */
#ifndef L
/* ELF-like local names start with `.L'.  */
# define L(name)	.L##name
#endif

#define atom_text_section .section ".text.atom", "ax"

#endif	/* __ASSEMBLER__ */

#endif	/* _X86_SYSDEP_H */
