/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <hurd/fd.h>
#include <stdarg.h>


/* Perform file control operations on FD.  */
int
DEFUN(__fcntl, (fd, cmd), int fd AND int cmd DOTS)
{
  va_list ap;
  struct hurd_userlink dtable_ulink;
  struct hurd_fd_user d = _hurd_fd_get (fd, &dtable_ulink);
  int result;
  
  if (d.d == NULL)
    return __hurd_fail (EBADF);

  va_start (ap, cmd);

  switch (cmd)
    {
    default:			/* Bad command.  */
      __spin_unlock (&d.d->lock);
      errno = EINVAL;
      result = -1;
      break;

      /* First the descriptor-based commands, which do no RPCs.  */

    case F_DUPFD:		/* Duplicate the file descriptor.  */
      {
	struct hurd_fd *new;
	io_t port, ctty;
	struct hurd_userlink ulink, ctty_ulink;
	int flags;

	/* Extract the ports and flags from the file descriptor.  */
	flags = d.d->flags;
	ctty = _hurd_port_get (&d.d->ctty, &ctty_ulink);
	port = _hurd_port_locked_get (&d.d->port, &ulink); /* Unlocks D.d.  */

	/* Get a new file descriptor.  The third argument to __fcntl is the
	   minimum file descriptor number for it.  */
	new = _hurd_alloc_fd (&result, va_arg (ap, int));
	if (new == NULL)
	  /* _hurd_alloc_fd has set errno.  */
	  result = -1;
	else
	  {
	    /* Give the ports each a user ref for the new descriptor.  */
	    __mach_port_mod_refs (__mach_task_self (), port,
				  MACH_PORT_RIGHT_SEND, 1);
	    if (ctty != MACH_PORT_NULL)
	      __mach_port_mod_refs (__mach_task_self (), ctty,
				    MACH_PORT_RIGHT_SEND, 1);

	    /* Install the ports and flags in the new descriptor.  */
	    if (ctty != MACH_PORT_NULL)
	      _hurd_port_set (&new->ctty, ctty);
	    _hurd_port_locked_set (&new->port, port);
	    /* Duplication clears the FD_CLOEXEC flag.  */
	    new->flags = flags & ~FD_CLOEXEC;
	  }

	_hurd_port_free (&d.d->port, &ulink, port);
	if (ctty != MACH_PORT_NULL)
	  _hurd_port_free (&d.d->ctty, &ctty_ulink, port);

	break;
      }

    case F_GETFD:		/* Get descriptor flags.  */
      result = d.d->flags;
      __spin_unlock (&d.d->lock);
      break;

    case F_SETFD:		/* Set descriptor flags.  */
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

    case F_GETFL:		/* Get per-open flags.  */
      {
	struct hurd_userlink ulink;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &ulink); /* Unlocks D.d.  */ 
	error_t err;

	err = __io_get_openmodes (port, &result);

	_hurd_port_free (&d.d->port, &ulink, port);

	if (err)
	  result = __hurd_fail (err);
	break;
      }

    case F_SETFL:		/* Set per-open flags.  */
      {
	const int flags = va_arg (ap, int);
	struct hurd_userlink ulink;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &ulink); /* Unlocks D.d.  */
	error_t err;

	err = __io_set_all_openmodes (port, flags);
	    
	_hurd_port_free (&d.d->port, &ulink, port);

	result = err ? __hurd_fail (err) : 0;
	break;
      }

    case F_GETOWN:		/* Get owner.  */
      {
	struct hurd_userlink ulink;
	error_t err;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &ulink); /* Unlocks D.d.  */
	err = __io_get_owner (port, &result);
	_hurd_port_free (&d.d->port, &ulink, port);
	result = err ? __hurd_fail (err) : 0;
	break;
      }

    case F_SETOWN:		/* Set owner.  */
      {
	pid_t owner = va_arg (ap, pid_t);
	error_t err;
	struct hurd_userlink ulink;
	io_t port
	  = _hurd_port_locked_get (&d.d->port, &ulink); /* Unlocks D.d.  */
	err = __io_mod_owner (port, owner);
	_hurd_port_free (&d.d->port, &ulink, port);
	result = err ? __hurd_fail (err) : 0;
	break;
      }
    }

  _hurd_fd_free (d, &dtable_ulink);

  va_end (ap);

  return result;
}
