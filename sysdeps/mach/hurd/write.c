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
#include <unistd.h>
#include <hurd.h>

ssize_t
DEFUN(__write, (fd, buf, nbytes),
      int fd AND CONST PTR buf AND size_t nbytes)
{
  error_t err;
  io_t server;
  int isctty;
  size_t wrote;

  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EBADF;
      return -1;
    }

  server = _hurd_dtable.d[fd].server;
  isctty = _hurd_ctty_check (fd);
  __mutex_unlock (&_hurd_dtable.lock);

  err = __io_write (server, isctty, _hurd_pid, _hurd_pgrp,
		    buf, nbytes, -1, &wrote);

  if (err)
    return __hurd_fail (err);

  return wrote;
}
