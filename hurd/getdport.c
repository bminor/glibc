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

struct _hurd_dtable _hurd_dtable;

file_t
__getdport (int fd)
{
  if (_hurd_dtable.d != NULL)
    {
      /* We have a real descriptor table.  */
      file_t dport;
      int err = _HURD_DPORT_USE (fd,
				 __mach_port_mod_refs (__mach_task_self (),
						       (dport = port),
						       MACH_PORT_RIGHT_SEND,
						       1));
      if (err)
	{
	  errno = err;
	  return MACH_PORT_NULL;
	}
      else
	return dport;
    }
  else
    {
      /* getdport is the only use of file descriptors,
	 so we don't bother allocating a real table.  */
      if (fd < 0 || fd > _hurd_init_dtablesize ||
	  _hurd_init_dtable[fd] == MACH_PORT_NULL)
	{
	  errno = EBADF;
	  return MACH_PORT_NULL;
	}
      else      
	{
	  __mach_port_mod_refs (__mach_task_self (), _hurd_init_dtable[fd],
				MACH_PORT_RIGHT_SEND, 1);
	  return _hurd_init_dtable[fd];
	}
    }
}
