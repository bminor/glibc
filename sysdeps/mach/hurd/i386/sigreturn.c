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
  register int *usp asm ("%eax");

  if (scp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  ss = _hurd_thread_sigstate (__mach_thread_self ());
  ss->blocked = scp->sc_mask;
  ss->intr_port = scp->sc_intr_port;
  if (scp->sc_onstack)
    ss->sigaltstack.ss_flags &= ~SA_ONSTACK;
  __mutex_unlock (&ss->lock);

  /* Push the flags and registers onto the stack we're returning to.  */
  usp = (int *) scp->sc_uesp;
  *--usp = scp->sc_eip;
  *--usp = scp->sc_efl;
  /* Segment registers??? XXX */
  *--usp = scp->sc_edi;
  *--usp = scp->sc_esi;
  *--usp = scp->sc_ebp;
  *--usp = scp->sc_ebx;
  *--usp = scp->sc_edx;
  *--usp = scp->sc_ecx;
  *--usp = scp->sc_eax;

  /* Switch to the target stack, and pop the state off it.  */
  asm volatile ("movl %0, %%esp\n"
		"popl %%eax\n"
		"popl %%ecx\n"
		"popl %%edx\n"
		"popl %%ebx\n"
		"popl %%ebp\n"
		"popl %%esi\n"
		"popl %%edi\n"
		"popf\n"
		"ret"
		: /* No outputs.  */
		: "g" (usp));

  /* NOTREACHED */
  return -1;
}
