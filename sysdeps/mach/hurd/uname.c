/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

#include <sys/utsname.h>
#include <hurd.h>
#include <hurd/startup.h>

int
uname (struct utsname *uname)
{
  startup_t init;
  error_t err;

  /* init is the repository of system version information.  */
  if (err = _HURD_PORT_USE (&_hurd_ports[INIT_PORT_PROC],
			    __proc_getmsgport (port, 1, &init)))
    return __hurd_fail (err);

  err = __startup_uname (uname); /* Ask it.  */

  __mach_port_deallocate (__mach_task_self (), init);

  if (err)
    return __hurd_fail (err);
  return 0;
}
