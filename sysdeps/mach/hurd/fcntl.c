/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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
#include <stdarg.h>

/* Mapping of F_GETFL/F_SETFL flag bits to io protocol functions.  */

struct flag
  {
    int flag;
    error_t (*get) (io_t, int *value);
    error_t (*mod) (io_t, int value);
  };
static const struct flag flags[] =
  {
    { O_NONBLOCK, __io_get_nonblock, __io_mod_nonblock },
    { O_APPEND, __io_get_append, __io_mod_append },
  };


/* Perform file control operations on FD.  */
int
DEFUN(__fcntl, (fd, cmd), int fd AND int cmd DOTS)
{
  va_list ap;
  int dealloc_dt;
  struct _hurd_fd_user d = _hurd_fd (fd, &dealloc_dt);
  int result;
  
  if (d.d == NULL)
    return __hurd_fail (EBADF);

  va_start (ap, cmd);

  switch (cmd)
    {
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      __spin_unlock (&d.d->lock);
      errno = ENOSYS;		/* XXX */
      result = -1;
      break;

    default:
      __spin_unlock (&d.d->lock);
      errno = EINVAL;
      result = -1;
      break;

    case F_DUPFD:
      {
	/* Duplicate the descriptor.  */

	struct _hurd_fd *new;

	/* Get a new descriptor.
	   It is unfortunate that D.d must remain locked during this call.  */
	new = _hurd_alloc_fd (&result, 0);
	if (new != NULL)
	  {
	    int dealloc, dealloc_ctty;
	    int flags = d.d->flags;
	    io_t ctty = _hurd_port_get (&d.d->ctty, &dealloc_ctty);
	    io_t port = _hurd_port_locked_get (&d.d->port,
					       &dealloc); /* Unlocks D.d.  */

	    /* Give the ports each a user ref for the new descriptor.  */
	    __mach_port_mod_refs (__mach_task_self (), port,
				  MACH_PORT_RIGHT_SEND, 1);
	    if (ctty != MACH_PORT_NULL)
	      __mach_port_mod_refs (__mach_task_self (), ctty,
				    MACH_PORT_RIGHT_SEND, 1);

	    /* Install the ports and flags in the new descriptor.  */
	    _hurd_port_set (&new->ctty, ctty);
	    _hurd_port_locked_set (&new->port, port);
	    new->flags = flags;

	    if (ctty != MACH_PORT_NULL)
	      _hurd_port_free (&d->ctty, ctty, &dealloc_ctty);
	    _hurd_port_free (&d->port, port, &dealloc);
	  }
	else
	  result = -1;

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
	int dealloc;
	io_t port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
	error_t err;
	unsigned int i;
	int openstat;

	err = __io_get_openstat (port, &openstat);

	if (!err)
	  {
	    switch (openstat & (FS_LOOKUP_READ|FS_LOOKUP_WRITE))
	      {
	      case FS_LOOKUP_READ:
		result = O_RDONLY;
		break;
	      case FS_LOOKUP_WRITE:
		result = O_WRONLY;
		break;
	      case FS_LOOKUP_READ|FS_LOOKUP_WRITE:
		result = O_RDWR;
		break;
	      default:
		result = 0;
		break;
	      }

	    for (i = 0; i < sizeof (flags) / sizeof (flags[0]); ++i)
	      {
		int value;
		if (err = (*flags[i].get) (port, &value))
		  break;
		if (value)
		  result |= flags[i].flag;
	      }
	  }
	    
	_hurd_port_free (port, &dealloc);

	if (err)
	  {
	    errno = err;
	    result = -1;
	  }
	break;
      }

    case F_SETFL:
      {
	int dealloc;
	const int dflags = d.d->flags;
	io_t port = _hurd_port_locked_get (d.d, &dealloc); /* Unlocks D.d.  */
	error_t err;
	unsigned int i;

	for (i = 0; i < sizeof (flags) / sizeof (flags[0]); ++i)
	  if (err = (*flags[i].get) (port, dflags & flags[i].flag))
	    break;
	    
	_hurd_port_free (port, &dealloc);

	if (err)
	  {
	    errno = err;
	    result = -1;
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
	if (err = __io_mod_owner (port, owner))
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
