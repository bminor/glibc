/* Copyright (C) 1991 Free Software Foundation, Inc.
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

int
__sigreturn (register const struct sigcontext *scp)
{
  error_t err;
  struct i386_thread_state ts;
  sturct _hurd_sigstate *ss;

  if (scp == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  else
    *(volatile const struct sigcontext *) scp;

  ss = _hurd_thread_sigstate (__mach_thread_self ());
  ss->blocked = scp->sc_mask;
  ss->sigstack.ss_onstack = scp->sc_onstack;
  __mutex_unlock (&ss->lock);

  /* There is no way to restore all the registers and condition codes
     with user-level code.  We must have the microkernel do it for us.  */

  /* Segment registers??? XXX */
  ts.edi = scp->sc_edi;
  ts.esi = scp->sc_esi;
  ts.ebp = scp->sc_ebp;

  ts.ebx = scp->sc_ebx;
  ts.edx = scp->sc_edx;
  ts.ecx = scp->sc_ecx;
  ts.eax = scp->sc_eax;
  
  ts.eip = scp->sc_eip;
  ts.uesp = scp->sc_uesp;
  ts.efl = scp->sc_efl;

  if (err = __thread_set_state (__mach_thread_self (), i386_THREAD_STATE,
				(int *) &ts, i386_THREAD_STATE_COUNT))
    {
      errno = EIO;		/* XXX ? */
      return -1;
    }

  /* NOTREACHED */
  return 0;
}
