/* Copyright (C) 1992 Free Software Foundation, Inc.
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
  struct _hurd_port *p = _hurd_dport (fd);
  io_t port;
  int dealloc;
  int result;
  
  if (p == NULL)
    {
      errno = EBADF;
      return -1;
    }
  if (p->port == MACH_PORT_NULL)
    {
      __spin_unlock (&p->lock);
      errno = EBADF;
      return -1;
    }

  va_start (ap, cmd);

  switch (cmd)
    {
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      errno = ENOSYS;
      result = -1;
      break;

    default:
      errno = EINVAL;
      result = -1;
      break;

    case F_DUPFD:
      {
	int new;
	__typeof (_hurd_dtable.d[fd]) old;
	old = _hurd_dtable.d[fd];
	_hurd_dtable.d[fd].user_dealloc = &dealloc;
	__spin_unlock (&_hurd_dtable.d[fd].user_dealloc);
	port = _hurd_port_locked_get (p, &dealloc);
	__mutex_lock (&hurd_dtable_lock);
	for (new = va_arg (ap, int); new < _hurd_dtable.size; ++new)
	  {
	    __typeof (&_hurd_dtable.d[new]) d = &_hurd_dtable.d[new];
	    __spin_lock (&d->port.lock);
	    if (d->port.port == MACH_PORT_NULL)
	      {
		*d = old;
		d->port.user_dealloc = NULL;
		__spin_unlock (&d->port.lock);
		__mutex_unlock (&hurd_dtable_lock);
		result = new;
		if (!dealloc)
		  __mach_port_mod_refs (__mach_task_self (), old.port.port,
					MACH_PORT_RIGHT_SEND, 1);
		goto win;
	      }
	    __spin_unlock (&d->port.lock);
	  }
	__mutex_unlock (&hurd_dtable_lock);
	errno = EMFILE;
	result = -1;
      win:
	break;
      }

    case F_GETFD:
      {
	result = _hurd_dtable.d[fd].flags;
	__spin_unlock (&_hurd_dtable.d[fd].lock);
	break;
      }

    case F_SETFD:
      _hurd_dtable.d[fd].flags = va_arg (ap, int);
      __spin_unlock (&_hurd_dtable.d[fd].lock);
      result = 0;
      break;

    case F_GETFL:
      {
	error_t err;
	int nonblock, append;
	port = _hurd_port_locked_get (p, &dealloc);
	err = __io_get_nonblock (port, &nonblock);
	if (!err)
	  err = __io_get_append (port, &append);
	_hurd_port_free (port, &dealloc);
	if (err)
	  {
	    errno = err;
	    result = -1;
	  }
	else
	  result = ((nonblock ? O_NONBLOCK : 0) |
		    (append ? O_APPEND : 0));
	break;
      }

    case F_SETFL:
      {
	int flags = va_arg (ap, int);
	error_t err;
	port = _hurd_port_locked_get (p, &dealloc);
	err = __io_mod_nonblock (port, flags & O_NONBLOCK);
	if (!err)
	  err = __io_mod_append (port, flags & O_APPEND);
	_hurd_port_free (port, &dealloc);
	if (err)
	  {
	    result = -1;
	    errno = err;
	  }
	else
	  result = 0;
	break;
      }

    case F_GETOWN:
      {
	error_t err;
	port = _hurd_port_locked_get (p, &dealloc);
	if (err = __io_get_owner (port, &result))
	  {
	    errno = err;
	    result = -1;
	  }
	_hurd_port_free (port, &dealloc);
	break;
      }

    case F_SETOWN:
      {
	pid_t owner = va_arg (ap, pid_t);
	error_t err;
	port = _hurd_port_locked_get (p, &dealloc);
	if (err = __io_set_owner (port, owner))
	  {
	    errno = err;
	    result = -1;
	  }
	else
	  result = 0;
	_hurd_port_free (port, &dealloc);
	break;
      }
    }

  va_end (ap);

  return result;
}
