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
#include <stddef.h>
#include <unistd.h>
#include <hurd.h>

/* Change the current directory to FD.  */
int
DEFUN(fchdir, (fd), int fd)
{
  file_t old;

  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EBADF;
      return -1;
    }

  __mach_port_mod_refs (__mach_task_self (), _hurd_dtable.d[fd].server,
			MACH_PORT_RIGHT_SEND, 1);
  __mutex_lock (&_hurd_lock);
  old = cwdir;
  cwdir = _hurd_dtable.d[fd].server;
  __mutex_unlock (&_hurd_lock);
  __mutex_unlock (&_hurd_dtable.lock);
  __mach_port_deallocate (__mach_task_self (), old);
  return 0;
}
