/* Swap to new context.
   Copyright (C) 2008-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <ucontext.h>
#include "ucontext_i.h"

extern int __getcontext (ucontext_t *ucp);
extern int __setcontext (const ucontext_t *ucp);

int
__swapcontext (ucontext_t *oucp, const ucontext_t *ucp)
{
  /* Save ucp in stack argument slot.  */
  asm ("stw %r25,-40(%sp)");
  asm (".cfi_offset 25, -40");

  /* Save rp for debugger.  */
  asm ("stw %rp,-20(%sp)");
  asm (".cfi_offset 2, -20");

  /* Copy rp to ret0 (r28).  */
  asm ("copy %rp,%ret0");

  /* Create a frame.  */
  asm ("ldo 64(%sp),%sp");
  asm (".cfi_def_cfa_offset -64");

  /* Save the current machine context to oucp.  */
  asm ("bl __getcontext,%rp");

  /* Copy oucp to register ret1 (r29).  __getcontext saves and restores it
     on a normal return.  It is restored from oR29 on reactivation.  */
  asm ("copy %r26,%ret1");

  /* Pop frame.  */
  asm ("ldo -64(%sp),%sp");
  asm (".cfi_def_cfa_offset 0");

  /* Load return pointer from oR28.  */
  asm ("ldw %0(%%ret1),%%rp" : : "i" (oR28));

  /* Return if error.  */
  asm ("or,= %r0,%ret0,%r0");
  asm ("bv,n %r0(%rp)");

  /* Load sc_sar flag.  */
  asm ("ldw %0(%%ret1),%%r20" : : "i" (oSAR));

  /* Return if oucp context has been reactivated.  */
  asm ("or,= %r0,%r20,%r0");
  asm ("bv,n %r0(%rp)");

  /* Mark sc_sar flag.  */
  asm ("1: ldi 1,%r20");
  asm ("stw %%r20,%0(%%ret1)" : : "i" (oSAR));

  /* Activate the machine context in ucp.  */
  asm ("bl __setcontext,%rp");
  asm ("ldw -40(%sp),%r26");

  /* Load return pointer.  */
  asm ("ldw %0(%%ret1),%%rp" : : "i" (oR28));

  /* A successful call to setcontext does not return.  */
  asm ("bv,n %r0(%rp)");

  /* Make gcc happy.  */
  return 0;
}

weak_alias (__swapcontext, swapcontext)
