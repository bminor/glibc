/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/signal.h>

/* Send a `sig_post' RPC to process number PID.  If PID is zero,
   send the message to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.
   SIG and REFPORT are passed along in the request message.  */
error_t
_hurd_sig_post (pid_t pid, int sig, mach_port_t refport)
{
  int delivered = 0;		/* Set when we deliver any signal.  */
  error_t err;
  mach_port_t oneport, *ports;
  pid_t *pids;
  mach_msg_type_number_t npids, nports, i;
  mach_port_t proc;
  struct hurd_userlink ulink;

  proc = _hurd_port_get (&_hurd_ports[INIT_PORT_PROC], &ulink);

  if (pid <= 0)
    {
      /* Send SIG to each process in pgrp (- PID).  */
      proccoll_t pcoll;
      err = __proc_pgrp_pcoll (proc, - pid, &pcoll);
      if (!err)
	{
	  err = __proc_get_collports (proc, pcoll,
				      &pids, &npids,
				      &ports, &nports);
	  __mach_port_deallocate (__mach_task_self (), pcoll);
	}
    }
  else
    {
      err = __proc_getmsgport (proc, pid, &oneport);
      npids = 1;
      pids = &pid;
      ports = &oneport;
      nports = 1;
    }

  if (npids != nports)
    {
      /* This is scrod.  We can't reasonably try to deliver any signals.  */
      err = EGRATUITOUS;
      goto out;
    }
    
  for (i = 0; i < nports; ++i)
    {
      int tried = 0;
      do
	{
	  mach_port_t msgport;
	  if (tried)
	    {
	      if (err = __proc_getmsgport (proc, pids[i], &msgport))
		break;
	    }
	  else
	    {
	      msgport = ports[i];
	      tried = 1;
	    }
	  err = __sig_post (msgport, sig, refport);
	  __mach_port_deallocate (__mach_task_self (), msgport);
	} while (err != MACH_SEND_INVALID_DEST && err != MIG_SERVER_DIED);
      if (! err)
	delivered = 1;
    }

 out:
  _hurd_port_free (&_hurd_ports[INIT_PORT_PROC], &ulink, proc);

  if (ports != &oneport)
    __vm_deallocate (__mach_task_self (),
		     (vm_address_t) ports, nports * sizeof (ports[0]));

  return delivered ? 0 : err;
}
