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

#include <hurd.h>

/* Open a file descriptor on a port.  */

int
openport (io_t port)
{
  int fd;

  /* Give the port a new user reference.
     This is a good way to check that it is valid.  */
  if (__mach_port_mod_refs (__mach_task_self (), port,
			    MACH_PORT_RIGHT_SEND, 1))
    {
      errno = EINVAL;
      return -1;
    }

  fd = _hurd_dalloc (port, 0);
  if (fd < 0)
    /* The descriptor table is full.  */
    __mach_port_deallocate (__mach_task_self (), port); 

  return fd;
}
