/* Set thread_state for sighandler, and sigcontext to recover.  i386 version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <mach/thread_status.h>

static void
trampoline (void (*handler) (int signo, int sigcode, struct sigcontext *scp),
	    int signo, int sigcode, struct sigcontext *scp)
{
  (*handler) (signo, sigcode, scp);
  (void) __sigreturn (scp);	/* Does not return.  */
  while (1)
    asm volatile ("hlt");		/* Firewall.  */
}

struct sigcontext *
_hurd_setup_sighandler (int flags,
			__sighandler_t handler,
			struct sigaltstack *sigaltstack,
			int signo, int sigcode,
			void *state)
{
  struct i386_thread_state *ts;
  void *sigsp;
  struct sigcontext *scp;
  struct 
    {
      void *retaddr;		/* Never used.  */
      __sighandler_t handler;
      int signo;
      int sigcode;
      struct sigcontext *scp;	/* Points to ctx, below.  */
      struct sigcontext ctx;
    } *stackframe;

  ts = state;

  if ((flags & SA_ONSTACK) &&
      !(sigaltstack->ss_flags & (SA_DISABLE|SA_ONSTACK)))
    {
      sigsp = sigaltstack->ss_sp + sigaltstack->ss_size;
      sigaltstack->ss_flags |= SA_ONSTACK;
    }
  else
    sigsp = (char *) ts->uesp;

  /* Push the arguments to call `trampoline' on the stack.  */
  sigsp -= sizeof (*stackframe);
  stackframe = sigsp;
  stackframe->handler = handler;
  stackframe->signo = signo;
  stackframe->sigcode = sigcode;
  stackframe->scp = scp = &stackframe->ctx;

  /* Set up the sigcontext from the current state of the thread.  */

  scp->sc_onstack = sigaltstack->ss_flags & SA_ONSTACK ? 1 : 0;

  scp->sc_edi = ts->edi;
  scp->sc_esi = ts->esi;
  scp->sc_ebp = ts->ebp;
  /* Segment registers??? XXX */

  scp->sc_ebx = ts->ebx;
  scp->sc_edx = ts->edx;
  scp->sc_ecx = ts->ecx;
  scp->sc_eax = ts->eax;
  
  scp->sc_eip = ts->eip;
  scp->sc_uesp = ts->uesp;
  scp->sc_efl = ts->efl;

  /* Modify the thread state to call `trampoline' on the new stack.  */
  ts->uesp = (int) sigsp;
  ts->eip = (int) &trampoline;

  return scp;
}
