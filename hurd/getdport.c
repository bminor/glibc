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

const struct
  {
    size_t n;
    file_t (*getdport) (int fd);
  } _hurd_getdport_fn;

file_t
__getdport (int fd)
{
  if (_hurd_init_dtable != NULL)
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
  else
    return (*_hurd_getdport_fn.getdport) (fd);
}
