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

/* Close the file descriptor FD.  */
int
DEFUN(__close, (fd), int fd)
{
  int dealloc;
  struct _hurd_fd_user d = _hurd_fd (fd, &dealloc);
  io_t port, ctty;

  if (d.d == NULL)
    return __hurd_fail (EBADF);

  /* Extract the descriptor's ports and replace them with nil.  */

  ctty = d.d->ctty.port;
  _hurd_port_set (&d.d->ctty, MACH_PORT_NULL);
  port = d.d->port.port;
  _hurd_port_locked_set (&d.d->port, MACH_PORT_NULL);

  _hurd_fd_done (d, &dealloc);

  /* Deallocate the ports.  */
  __mach_port_deallocate (__mach_task_self (), port);
  if (ctty != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), ctty);

  return 0;
}
