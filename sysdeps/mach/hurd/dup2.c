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
#include <fcntl.h>
#include <unistd.h>
#include <hurd.h>


/* Duplicate FD to FD2, closing the old FD2 and making FD2 be
   open on the same file as FD is.  Return FD2 or -1.  */
int
DEFUN(__dup2, (fd, fd2), int fd AND int fd2)
{
  int dealloc_dt;
  struct _hurd_fd_user d;
  struct _hurd_fd *d2;
  io_t port, ctty;
  int dealloc, dealloc_ctty;
  int flags;

  /* Extract the ports and flags from FD.  */
  d = _hurd_fd (fd, &dealloc_dt); /* Locks D.d.  */
  if (d.d == NULL)
    {
      errno = EBADF;
      return -1;
    }
  flags = d.d->flags;
  ctty = _hurd_port_get (&d.d->ctty, &dealloc_ctty);
  port = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */

  __mutex_lock (&_hurd_dtable_lock);
  if (fd2 < 0 || fd2 >= _hurd_dtable.size)
    {
      errno = EBADF;
      fd2 = -1;
    }
  else
    {
      /* Give the ports each a user ref for the new descriptor.  */
      __mach_port_mod_refs (__mach_task_self (), port,
			    MACH_PORT_RIGHT_SEND, 1);
      if (ctty != MACH_PORT_NULL)
	__mach_port_mod_refs (__mach_task_self (), ctty,
			      MACH_PORT_RIGHT_SEND, 1);

      /* Install the ports and flags in the new descriptor slot.  */
      d2 = &_hurd_dtable.d[fd2];
      __spin_lock (&d2->port.lock);
      d2->flags = flags;
      _hurd_port_set (&d2->ctty, ctty);
      _hurd_port_locked_set (&d2->port, port); /* Unlocks D2.  */
    }
  __mutex_unlock (&_hurd_dtable_lock);

  _hurd_port_free (&d.d->port, &dealloc, port);
  if (ctty != MACH_PORT_NULL)
    _hurd_port_free (&d.d->ctty, &dealloc_ctty, port);

  _hurd_fd_done (d, &dealloc_dt);

  return fd2;
}
