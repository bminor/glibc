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

#include <hurd.h>

int
openport (file_t port)
{
  int fd;

  __mutex_lock (&_hurd_dtable.lock);

  fd = _hurd_dalloc ();
  if (fd < 0)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      return -1;
    }

  if (__mach_port_mod_refs (__mach_task_self (), port,
			    MACH_PORT_RIGHT_SEND, 1))
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EINVAL;
      return -1;
    }

  _hurd_dtable.d[fd].server = port;
  __mutex_unlock (&_hurd_dtable.lock);
  return 0;
}
