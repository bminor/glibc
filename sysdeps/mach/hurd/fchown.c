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

/* Change the owner and group of the file referred to by FD.  */
int
DEFUN(__fchown, (fd, owner, group),
      int fd AND uid_t owner AND gid_t group)
{
  error_t err;
  io_t server;

  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EBADF;
      return -1;
    }
  server = _hurd_dtable.d[fd].server;
  __mutex_unlock (&_hurd_dtable.lock);

  if (err = __file_chown (server, owner, group))
    return __hurd_fail (err);
  return 0;
}
