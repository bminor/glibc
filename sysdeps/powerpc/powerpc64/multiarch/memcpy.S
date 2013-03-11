/* Multiple versions of memcpy PowerPC64.
   Copyright (C) 2012-2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <rtld-global-offsets.h>

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need memcpy before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.section	".toc","aw"
.LC__dl_hwcap:
# ifdef SHARED
	.tc _rtld_global_ro[TC],_rtld_global_ro
# else
	.tc _dl_hwcap[TC],_dl_hwcap
# endif
	.section ".text"

ENTRY(memcpy)
	.type   memcpy, @gnu_indirect_function
	ld	r5,.LC__dl_hwcap@toc(r2)
# ifdef SHARED
	/* Load _rtld-global._dl_hwcap. */
	ld	r5,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET(r5)
# else
	ld	r5,0(r5) /* Load extern _dl_hwcap. */
# endif
	rldicl.	r0,r5,57,63
	bne- L(power7)
	rldicl. r0,r5,56,63
	bne- L(powerA2)
	rldicl.	r0,r5,52,63
	bne- L(power6)
	rldicl.	r0,r5,48,63
	bne- L(powercell)
	rldicl. r0,r5,45,63
	bne- L(power4)
	ld	r3,__memcpy_ppc64@got(r2)
	blr
L(power7):
	ld	r3,__memcpy_power7@got(r2)
	blr
L(powerA2):
	ld	r3,__memcpy_power_a2@got(r2)
	blr
L(power6):
	ld	r3,__memcpy_power6@got(r2)
	blr
L(powercell):
	ld	r3,__memcpy_cell@got(r2)
	blr
L(power4):
	ld	r3,__memcpy_power4@got(r2)
	blr
END(memcpy)

# undef EALIGN
# define EALIGN(name, alignt, words)                  \
  ENTRY_2(__memcpy_ppc64)                             \
BODY_LABEL(__memcpy_ppc64):                           \
  cfi_startproc;

# undef END_GEN_TB
# define END_GEN_TB(name, mask)                       \
  cfi_endproc;                                        \
  TRACEBACK_MASK(__memcpy_ppc64,mask)                 \
  END_2(__memcpy_ppc64)

# undef libc_hidden_builtin_def
# define libc_hidden_builtin_def(name)                \
  .globl __GI_memcpy; __GI_memcpy = __memcpy_ppc64

#endif

#include "../memcpy.S"
