/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <hurd/fd.h>

/* Close the file descriptor FD.  */
int
DEFUN(__close, (fd), int fd)
{
  struct hurd_userlink ulink;
  struct hurd_fd_user d;
  error_t err;

  HURD_CRITICAL_BEGIN;
  d = _hurd_fd_get (fd, &ulink);

  if (d.d == NULL)
    err = EBADF;
  else
    err = _hurd_fd_close (d.d);

  _hurd_fd_free (d, &ulink);
  HURD_CRITICAL_END;

  return err ? __hurd_fail (err) : 0;
}
