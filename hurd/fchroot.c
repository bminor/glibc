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
#include <stddef.h>
#include <unistd.h>
#include <hurd.h>

/* Change the current directory root to FD.  */
int
DEFUN(fchroot, (fd), int fd)
{
  error_t err;
  file_t crdir;

  /* XXX check that's it's a directory */

  if (err = _HURD_DPORT_USE (fd,
			     __mach_port_mod_refs (__mach_task_self (),
						   (crdir = port),
						   MACH_PORT_RIGHT_SEND, 1)))
    return err;

  _hurd_port_set (&_hurd_ports[INIT_PORT_CRDIR], crdir);

  return 0;
}
