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
#include <unistd.h>
#include <hurd.h>

/* Close the file descriptor FD.  */
int
DEFUN(__close, (fd), int fd)
{
  struct _hurd_fd *d;
  io_t port, ctty;

  __mutex_lock (&_hurd_dtable_lock);
  if (fd < 0 || fd >= _hurd_dtable.size)
    {
      __mutex_unlock (&_hurd_dtable_lock);
      errno = EBADF;
      return -1;
    }

  d = &_hurd_dtable.d[fd];
  __spin_lock (d->port);
  port = d->port.port;
  ctty = d->ctty.port;
  d->port.port = d->ctty.port = MACH_PORT_NULL;
  d->port.user_dealloc = d->ctty.user_dealloc = NULL;
  d->flags = 0;
  __spin_unlock (&d->port);

  __mutex_unlock (&_hurd_dtable_lock);

  if (port == MACH_PORT_NULL)
    {
      errno = EBADF;
      return -1;
    }

  __mach_port_deallocate (__mach_task_self (), port);
  if (ctty != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), ctty);

  return 0;
}
