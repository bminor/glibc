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
    default:
      __spin_unlock (&d.d->lock);
      errno = EINVAL;
      result = -1;
      break;

      /* First the descriptor-based commands, which do no RPCs.  */

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
	      _hurd_port_free (&d.d->ctty, &dealloc_ctty, ctty);
	    _hurd_port_free (&d.d->port, &dealloc, port);
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


      /* Now the real io operations, done by RPCs to io servers.  */

    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      {
	struct flock *fl = va_arg (ap, struct flock *);
	__spin_unlock (&d.d->lock);
	errno = fl?ENOSYS:EINVAL; /* XXX mib needs to implement io rpcs.  */
	result = -1;
	break;
      }

    case F_GETFL:
      {
	int dealloc;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */
	error_t err;

	err = __io_get_openmodes (port, &result);

	_hurd_port_free (&d.d->port, &dealloc, port);

	if (err)
	  result = __hurd_fail (err);
	break;
      }

    case F_SETFL:
      {
	const int flags = va_arg (ap, int);
	int dealloc;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */
	error_t err;

	err = __io_set_all_openmodes (port, flags);
	    
	_hurd_port_free (&d.d->port, &dealloc, port);

	result = err ? __hurd_fail (err) : 0;
	break;
      }

    case F_GETOWN:
      {
	int dealloc;
	error_t err;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */
	err = __io_get_owner (port, &result);
	_hurd_port_free (&d.d->port, &dealloc, port);
	result = err ? __hurd_fail (err) : 0;
	break;
      }

    case F_SETOWN:
      {
	pid_t owner = va_arg (ap, pid_t);
	error_t err;
	int dealloc;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &dealloc); /* Unlocks D.d.  */
	err = __io_mod_owner (port, owner);
	_hurd_port_free (&d.d->port, &dealloc, port);
	result = err ? __hurd_fail (err) : 0;
	break;
      }
    }

  _hurd_fd_done (d, &dealloc_dt);

  va_end (ap);

  return result;
}
