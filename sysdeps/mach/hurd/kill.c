/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <signal.h>
#include <hurd.h>

/* Send signal SIG to process number PID.  If PID is zero,
   send SIG to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.  */
int
DEFUN(__kill, (pid, sig), int pid AND int sig)
{
  error_t err;
  mach_port_t portbuf[10];
  mach_port_t *ports = portbuf;
  size_t nports = 10, i;
  mach_port_t proc;
  int dealloc_proc;

  proc = _hurd_port_get (&_hurd_proc, &dealloc_proc);

  if (pid <= 0)
    {
      /* Send SIG to each process in pgrp (- PID).  */
      proccoll_t pcoll;
      err = __proc_pgrp_pcoll (proc, - pid, &pcoll);
      if (!err)
	{
	  err = __proc_get_collports (proc, pcoll, &ports, &nports);
	  __mach_port_deallocate (__mach_task_self (), pcoll);
	}
    }
  else
    {
      err = __proc_getmsgport (proc, pid, &ports[0]);
      nports = 1;
    }

  if (!err)
    {
      err = __proc_pid2task (proc, pid, &refport);
      if (err)
	err = __proc_getsidport (proc, &refport);
    }

  _hurd_port_free (&_hurd_proc, proc, &dealloc_proc);

  for (i = 0; i < nports; ++i)
    {
      if (!err)
	err = __sig_post (ports[i], sig, refport);
      __mach_port_deallocate (__mach_task_self (), ports[i]);
    }

  if (refport != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), refport);

  if (ports != portbuf)
    __vm_deallocate (__mach_task_self (), ports, nports * sizeof (ports[0]));

  if (err)
    return __hurd_fail (err);
  return 0;
}
