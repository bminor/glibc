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

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <hurd.h>

/* Get the process group ID of process PID.  */
int
DEFUN(__getpgrp, (pid), pid_t pid)
{
  error_t err;
  pid_t pgrp;

  __mutex_lock (&_hurd_pid_lock);

  if (pid == 0)
    {
      pgrp = _hurd_pgrp;
      err = 0;
    }
  else
    err = _HURD_PORT_USE (&_hurd_ports[INIT_PORT_PROC],
			  __proc_getpgrp (port, pid, &pgrp));

  __mutex_unlock (&_hurd_pid_lock);

  return err ? __hurd_fail (err) : pgrp;
}
