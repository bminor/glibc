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
  int dealloc_dt;
  struct _hurd_fd_user d = _hurd_fd (fd, &dealloc_dt);
  io_t port, ctty;
  int dealloc, dealloc_ctty;
  int result;
  
  if (d.d == NULL)
    {
      errno = EBADF;
      return -1;
    }

  va_start (ap, cmd);

  switch (cmd)
    {
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      __spin_unlock (&d.d->lock);
      errno = ENOSYS;
      result = -1;
      break;

    default:
      __spin_unlock (&d.d->lock);
      errno = EINVAL;
      result = -1;
      break;

    case F_DUPFD:
      {
	int fd2 = va_arg (args, int);
	int flags;

	/* Extract the ports and flags from FD.  */
	flags = d->flags;
	ctty = _hurd_port_get (&d->ctty, &dealloc_ctty);
	port = _hurd_port_locked_get (&d->port, &dealloc); /* Unlocks D.  */
	
	if (port == MACH_PORT_NULL)
	  goto badf;
	
	__mutex_lock (&_hurd_dtable_lock);
	if (fd2 < 0 || fd2 >= _hurd_dtable.size)
	  {
	    errno = EBADF;
	    fd2 = -1;
	  }
	else
	  {
	    while (fd2 < _hurd_dtable.size)
	      {
		struct _hurd_fd *d2 = &_hurd_dtable.d[fd2];
		__spin_lock (&d2->port.lock);
		if (d2->port.port == MACH_PORT_NULL)
		  {
		    d2->flags = flags;
		    _hurd_port_set (&d2->ctty, ctty);
		    _hurd_port_locked_set (&d2->port, port); /* Unlocks D2.  */
		    break;
		  }
		else
		  __spin_unlock (&d2->port.lock);
	      }
	    if (fd2 == _hurd_dtable.size)
	      {
		errno = EMFILE;
		fd2 = -1;
	      }
	  }
	__mutex_unlock (&_hurd_dtable_lock);
	
	if (fd2 >= 0)
	  {
	    /* Give the ports each a user ref for the new descriptor.  */
	    __mach_port_mod_refs (__mach_task_self (), port,
				  MACH_PORT_RIGHT_SEND, 1);
	    if (ctty != MACH_PORT_NULL)
	      __mach_port_mod_refs (__mach_task_self (), ctty,
				    MACH_PORT_RIGHT_SEND, 1);
	  }
	
	_hurd_port_free (port, &dealloc);
	if (ctty != MACH_PORT_NULL)
	  _hurd_port_free (ctty, &dealloc_ctty);
	
	result = fd2;
	break;
      }

    case F_GETFD:
      result = d.d->flags;
      __spin_unlock (&d.d->lock);
      break;

    case F_SETFD:
      d.d->flags = va_arg (ap, int);
      __spin_unlock (&d.d->lock);
      result = 0;
      break;

    case F_GETFL:
      {
	error_t err;
	int nonblock, append;
	port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
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
	port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
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
	port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
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
	port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
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

  _hurd_fd_done (d, &dealloc_dt);

  va_end (ap);

  return result;
}
