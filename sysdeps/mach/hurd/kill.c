/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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

/* Send signal SIG to process number PID.  If PID is zero,
   send SIG to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.  */
int
__kill (pid_t pid, int sig)
{
  error_t err;
  mach_port_t oneport, *ports;
  pid_t *pids;
  mach_msg_type_number_t npids, nports, i;
  mach_port_t proc;
  int dealloc_proc;

  proc = _hurd_port_get (&_hurd_ports[INIT_PORT_PROC], &dealloc_proc);

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
      if (sig == SIGKILL)
	{
	  err = 0;
	  oneport = MACH_PORT_NULL;
	}
      else
	err = __proc_getmsgport (proc, pid, &oneport);
      npids = 1;
      pids = &pid;
      ports = &oneport;
      nports = 1;
    }

  if (npids != nports)
    err = EGRATUITOUS;

  if (sig == SIGKILL)
    {
      /* SIGKILL is not delivered as a normal signal.
	 Sending SIGKILL to a process means to terminate its task.  */
      for (i = 0; i < npids; ++i)
	{
	  /* We don't care about the message port.  Deallocate it.  */
	  if (ports[i])
	    __mach_port_deallocate (__mach_task_self (), ports[i]);
	  if (!err)
	    /* XXX The above if makes kill fail as soon as it cannot kill
               one process, returning that one's error and ignoring the
               rest.  Is that right?  */

	    /* Fetch the process's task port and terminate the task.  We
	       loop in case the process execs and changes its task port.
	       If the old task port dies after we fetch it but before we
	       send the RPC, we get MACH_SEND_INVALID_DEST; if it dies
	       after we send the RPC request but before it is serviced, we
	       get MIG_SERVER_DIED.  */
	    do
	      {
		task_t refport;
		err = __proc_pid2task (proc, pids[i], &refport);
		if (!err)
		  {
		    err = __task_terminate (refport);
		    __mach_port_deallocate (__mach_task_self (), refport);
		  }
	      } while (err != MACH_SEND_INVALID_DEST &&
		       err != MIG_SERVER_DIED);
	}
    }

  for (i = 0; i < nports; ++i)
    {
      int tried = 0;
      if (err)
	/* We have already lost.  Just deallocate the port.  */
	    /* XXX The above if makes kill fail as soon as it cannot kill
               one process, returning that one's error and ignoring the
               rest.  Is that right?  */
	__mach_port_deallocate (__mach_task_self (), ports[i]);
      else
	err = _HURD_MSGPORT_RPC (tried ?
				 __proc_getmsgport (proc, pids[i], &msgport) :
				 (tried = 1, ports[i]),
				 ({ err = __proc_pid2task (proc, pids[i],
							   &refport);
				    if (err)
				      err = __proc_getsidport (proc, &refport);
				    err; }),
				 __sig_post (msgport, sig, refport));
    }

  _hurd_port_free (&_hurd_ports[INIT_PORT_PROC], &dealloc_proc, proc);

  if (ports != &oneport)
    __vm_deallocate (__mach_task_self (),
		     (vm_address_t) ports, nports * sizeof (ports[0]));

  return err ? __hurd_fail (err) : 0;
}
