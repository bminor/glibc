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
#include <fcntl.h>
#include <hurd.h>

/* Perform file control operations on FD.  */
int
DEFUN(__fcntl, (fd, cmd), int fd AND int cmd DOTS)
{
  va_list ap;

  __mutex_lock (&_hurd_dtable_lock);

  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      __mutex_unlock (&_hurd_dtable_lock);
      errno = EBADF;
      return -1;
    }

  switch (cmd)
    {
    case F_DUPFD:
      {
	int new;
	va_start (ap, cmd);
	new = va_arg (ap, int);
	va_end (ap);
	if (new < 0 || fd >= _hurd_dtable.size)
	  {
	    __mutex_unlock (&_hurd_dtable_lock);
	    errno = EBADF;
	    return -1;
	  }
	while (new < _hurd_dtable.size)
	  if (_hurd_dtable.d[new].server == MACH_PORT_NULL)
	    {
	      if (__mach_port_mod_refs (__mach_task_self (),
					_hurd_dtable.d[fd].server,
					MACH_PORT_RIGHT_SEND,
					1))
		{
		  __mutex_unlock (&_hurd_dtable_lock);
		  errno = EBADF;
		  return -1;
		}
	      _hurd_dtable.d[new] = _hurd_dtable.d[fd];
	      __mutex_unlock (&_hurd_dtable_lock);
	      return new;
	    }
	  else
	    ++new;
	__mutex_unlock (&_hurd_dtable_lock);
	errno = EMFILE;
	return -1;
      }

    case F_GETFD:
      {
	const int flags = _hurd_dtable.d[fd].flags;
	__mutex_unlock (&_hurd_dtable_lock);
	return flags;
      }

    case F_SETFD:
      {
	int flags;
	va_start (ap, cmd);
	flags = va_arg (ap, int);
	va_end (ap);
	_hurd_dtable.d[fd].flags = flags;
	__mutex_unlock (&_hurd_dtable_lock);
	return 0;
      }

    case F_GETFL:		/* XXX io_get_flags */
    case F_SETFL:		/* XXX io_set_flags */
    case F_SETLK:
    case F_SETLKW:
    case F_GETOWN:
    case F_SETOWN:
      /* XXX */

    default:
      __mutex_unlock (&_hurd_dtable_lock);
      errno = EINVAL;
      return -1;
    }
}
