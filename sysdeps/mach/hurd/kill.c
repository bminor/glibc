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
  /* XXXXXXXXXXXXXXXXXXXXXXXXXXX */
  error_t err;

  __mutex_lock (&_hurd_lock);

  if (pid == 0)
    pid = - _hurd_pgrp;

  if (pid < 0)
    {
      /* Send SIG to each process in pgrp (- PID).  */
      proccoll_t pcoll;
      err = __proc_pgrp_pcoll (_hurd_proc, - pid, &pcoll);
      if (!err)
	{
	  err = __proc_get_collports (_hurd_proc, pcoll, &ports, &nports);
	  __mach_port_deallocate (__mach_task_self (), pcoll);
	}
    }
  else
    {
      err = __proc_pid2task (_hurd_proc, pid, &refport);
      if (err)
	err = __proc_getsidport (_hurd_proc, &refport);
      if (!err)
	err = __proc_kill (_hurd_proc, pid, sig);
      if (refport != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), refport);
      __mutex_unlock (&_hurd_lock);
    }

  if (err)
    return __hurd_fail (err);
  return 0;
}
