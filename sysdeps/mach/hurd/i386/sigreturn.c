/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <hurd.h>
#include <hurd/signal.h>

int
__sigreturn (register const struct sigcontext *scp)
{
  struct hurd_sigstate *ss;
  register int *usp asm ("%eax"); /* Force it into a register.  */

  if (scp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_self_sigstate ();
  ss->blocked = scp->sc_mask;
  ss->intr_port = scp->sc_intr_port;
  if (scp->sc_onstack)
    ss->sigaltstack.ss_flags &= ~SA_ONSTACK;
  __mutex_unlock (&ss->lock);

  /* Push the flags and registers onto the stack we're returning to.  */
  usp = (int *) scp->sc_uesp;

  /* The last thing popped will be the PC being restored, so push it first.  */
  *--usp = scp->sc_eip;
  *--usp = scp->sc_efl;		/* Second to last, processor flags.  */

  /* Segment registers??? XXX */

  /* Now push the general registers in the reverse of the order they will
     be popped off by the `popa' instruction.  The instruction passes over
     a word corresponding to %esp, but does not use the value.  */
  *--usp = scp->sc_eax;
  *--usp = scp->sc_ecx;
  *--usp = scp->sc_edx;
  *--usp = scp->sc_ebx;
  *--usp = 0;			/* Ignored by `popa'.  */
  *--usp = scp->sc_ebp;
  *--usp = scp->sc_esi;
  *--usp = scp->sc_edi;

  asm volatile
    ("movl %0, %%esp\n"		/* Switch to the target stack.  */
     "popa\n"			/* Pop all the general registers.  */
     "popf\n"			/* Pop the flags.  */
     "ret\n"			/* Pop the target PC and jump to it.  */
     "hlt" : :			/* Firewall.  */
     "g" (usp));

  /* NOTREACHED */
  return -1;
}
