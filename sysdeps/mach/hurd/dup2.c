/* Copyright (C) 1991 Free Software Foundation, Inc.
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


/* Duplicate FD to FD2, closing the old FD2 and making FD2 be
   open the same file as FD is.  Return FD2 or -1.  */
int
DEFUN(__dup2, (fd, fd2), int fd AND int fd2)
{
  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL ||
      fd2 < 0 || fd2 >= _hurd_dtable.size)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EBADF;
      return -1;
    }
  if (_hurd_dtable.d[fd2].server != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), _hurd_dtable.d[fd2].server);
  _hurd_dtable.d[fd2].server = _hurd_dtable.d[fd].server;
  _hurd_dtable.d[fd2].isctty = _hurd_dtable.d[fd].isctty;
  __mach_port_mod_refs (__mach_task_self (), _hurd_dtable.d[fd].server,
			MACH_PORT_RIGHT_SEND, 1);
  __mutex_unlock (&_hurd_dtable.lock);

  return fd2;
}
