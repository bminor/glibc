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

#include <hurd.h>
#include <gnu-stabs.h>

#if 0
struct mutex _hurd_pid_lock;
#endif
pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
int _hurd_orphaned;

static void
init_pids (void)
{
#if 0
  __mutex_init (&_hurd_pid_lock);
#endif

  __USEPORT (PROC,
	     ({
	       __proc_getpids (port, &_hurd_pid, &_hurd_ppid, &_hurd_orphaned);
	       __proc_getpgrp (port, _hurd_pid, &_hurd_pgrp);
	     }));
}

text_set_element (__libc_subinit, init_pids);

#include "hurd/msg_server.h"

error_t
_S_proc_newids (mach_port_t me,
		task_t task,
		pid_t ppid, pid_t pgrp, int orphaned)
{
  if (task != __mach_task_self ())
    return EPERM;

  __mach_port_deallocate (__mach_task_self (), task);

  __mutex_lock (&_hurd_pid_lock);
  _hurd_ppid = ppid;
  _hurd_pgrp = pgrp;
  _hurd_orphaned = orphaned;
  __mutex_unlock (&_hurd_pid_lock);

  return 0;
}
