/* Copyright (C) 1992-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdarg.h>
#include <sys/file.h>		/* XXX for LOCK_* */
#ifdef NOCANCEL
#include <not-cancel.h>
#else
#include <sysdep-cancel.h>
#endif
#include "f_setlk.h"

/* Perform file control operations on FD.  */
int
__libc_fcntl (int fd, int cmd, ...)
{
  va_list ap;
  struct hurd_fd *d;
  int result;

  d = _hurd_fd_get (fd);

  if (d == NULL)
    return __hurd_fail (EBADF);

  va_start (ap, cmd);

  switch (cmd)
    {
      error_t err;

    default:			/* Bad command.  */
      result = __hurd_fail (EINVAL);
      break;

      /* First the descriptor-based commands, which do no RPCs.  */

    case F_DUPFD:		/* Duplicate the file descriptor.  */
    case F_DUPFD_CLOEXEC:
      {
	struct hurd_fd *new;
	io_t port, ctty;
	struct hurd_userlink ulink, ctty_ulink;
	int flags;

	HURD_CRITICAL_BEGIN;

	/* Extract the ports and flags from the file descriptor.  */
	__spin_lock (&d->port.lock);
	flags = d->flags;
	ctty = _hurd_port_get (&d->ctty, &ctty_ulink);
	port = _hurd_port_locked_get (&d->port, &ulink); /* Unlocks D.  */

	if (cmd == F_DUPFD_CLOEXEC)
	  flags |= FD_CLOEXEC;
	else
	  /* Duplication clears the FD_CLOEXEC flag.  */
	  flags &= ~FD_CLOEXEC;

	/* Get a new file descriptor.  The third argument to __fcntl is the
	   minimum file descriptor number for it.  */
	new = _hurd_alloc_fd (&result, va_arg (ap, int));
	if (new == NULL)
	  /* _hurd_alloc_fd has set errno.  */
	  result = -1;
	else
	  {
	    /* Give the io server port a user ref for the new descriptor.  */
	    err = __mach_port_mod_refs (__mach_task_self (), port,
					MACH_PORT_RIGHT_SEND, 1);

	    if (err == KERN_UREFS_OVERFLOW)
	      result = __hurd_fail (EMFILE);
	    else if (err)
	      result = __hurd_fail (EINVAL);
	    else if (ctty != MACH_PORT_NULL)
	      {
		/* We have confirmed the io server port has got a user ref
		   count, now give ctty port a user ref for the new
		   descriptor.  */
		err = __mach_port_mod_refs (__mach_task_self (), ctty,
					    MACH_PORT_RIGHT_SEND, 1);

		if (err)
		  {
		    /* In this case the io server port has got a ref count
		    but the ctty port fails to get one, so we need to clean
		    the ref count we just assigned.  */
		    __mach_port_mod_refs (__mach_task_self (), port,
					  MACH_PORT_RIGHT_SEND, -1);

		    if (err == KERN_UREFS_OVERFLOW)
		      result = __hurd_fail (EMFILE);
		    else
		      result = __hurd_fail (EINVAL);
		  }
	      }

	    if (!err)
	      {
		/* The ref counts of the ports are incremented successfully.  */
		/* Install the ports and flags in the new descriptor.  */
		if (ctty != MACH_PORT_NULL)
		  _hurd_port_set (&new->ctty, ctty);
		new->flags = flags;
		/* Unlocks NEW.  */
		_hurd_port_locked_set (&new->port, port);
	      }
	  }

	HURD_CRITICAL_END;

	_hurd_port_free (&d->port, &ulink, port);
	if (ctty != MACH_PORT_NULL)
	  _hurd_port_free (&d->ctty, &ctty_ulink, port);

	break;
      }

      /* Set RESULT by evaluating EXPR with the descriptor locked.
	 Check for an empty descriptor and return EBADF.  */
#define LOCKED(expr) do {						      \
      HURD_CRITICAL_BEGIN;						      \
      __spin_lock (&d->port.lock);					      \
      if (d->port.port == MACH_PORT_NULL)				      \
	result = __hurd_fail (EBADF);					      \
      else								      \
	result = (expr);						      \
      __spin_unlock (&d->port.lock);					      \
      HURD_CRITICAL_END;						      \
} while(0)

    case F_GETFD:		/* Get descriptor flags.  */
      LOCKED (d->flags);
      break;

    case F_SETFD:		/* Set descriptor flags.  */
      LOCKED ((d->flags = va_arg (ap, int), 0));
      break;


      /* Now the real io operations, done by RPCs to io servers.  */

    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
      {
	struct flock *fl = va_arg (ap, struct flock *);

	switch (cmd)
	  {
	  case F_GETLK:
	    cmd = F_GETLK64;
	    break;
	  case F_SETLK:
	    cmd = F_SETLK64;
	    break;
	  case F_SETLKW:
	    cmd = F_SETLKW64;
	    break;
	  default:
	    va_end (ap);
	    return __hurd_fail (EINVAL);
	  }

	struct flock64 fl64 = {
	  .l_type = fl->l_type,
	  .l_whence = fl->l_whence,
	  .l_start = fl->l_start,
	  .l_len = fl->l_len,
	  .l_pid = fl->l_pid
	};

#ifndef NOCANCEL
	if (cmd == F_SETLKW64)
	  {
	    int cancel_oldtype = LIBC_CANCEL_ASYNC();
	    err = HURD_FD_PORT_USE_CANCEL (d, __file_record_lock (port, cmd,
					   &fl64, MACH_PORT_NULL,
					   MACH_MSG_TYPE_MAKE_SEND));
	    LIBC_CANCEL_RESET (cancel_oldtype);
	  }
	else
#endif
	  err = HURD_FD_PORT_USE (d, __file_record_lock (port, cmd, &fl64,
				  MACH_PORT_NULL, MACH_MSG_TYPE_MAKE_SEND));

	/* XXX: To remove once file_record_lock RPC is settled.  */
	if (err == EMIG_BAD_ID || err == EOPNOTSUPP)
	  {
	    int wait = 0;
	    va_end (ap);
	    switch (cmd)
	      {
	      case F_GETLK64:
		return __hurd_fail (ENOSYS);
	      case F_SETLKW64:
		wait = 1;
		/* FALLTHROUGH */
	      case F_SETLK64:
		return __f_setlk (fd, fl->l_type, fl->l_whence,
				  fl->l_start, fl->l_len, wait);
	      default:
		return __hurd_fail (EINVAL);
	      }
	  }
	else if (cmd == F_GETLK64)
	  {
	    fl->l_type = fl64.l_type;
	    fl->l_whence = fl64.l_whence;
	    fl->l_start = fl64.l_start;
	    fl->l_len = fl64.l_len;
	    fl->l_pid = fl64.l_pid;

	    if ((sizeof fl->l_start != sizeof fl64.l_start
		 && fl->l_start != fl64.l_start)
	     || (sizeof fl->l_len != sizeof fl64.l_len
		 && fl->l_len != fl64.l_len))
	      {
	        va_end (ap);
	        return __hurd_fail (EOVERFLOW);
	      }
	  }

	result = err ? __hurd_dfail (fd, err) : 0;
	break;
      }

    case F_GETLK64:
    case F_SETLK64:
    case F_SETLKW64:
      {
	struct flock64 *fl = va_arg (ap, struct flock64 *);

#ifndef NOCANCEL
	if (cmd == F_SETLKW64)
	  {
	    int cancel_oldtype = LIBC_CANCEL_ASYNC();
	    err = HURD_FD_PORT_USE_CANCEL (d, __file_record_lock (port, cmd,
					   fl, MACH_PORT_NULL,
					   MACH_MSG_TYPE_MAKE_SEND));
	    LIBC_CANCEL_RESET (cancel_oldtype);
	  }
	else
#endif
	  err = HURD_FD_PORT_USE (d, __file_record_lock (port, cmd, fl,
				  MACH_PORT_NULL, MACH_MSG_TYPE_MAKE_SEND));

	/* XXX: To remove once file_record_lock RPC is settled.  */
	if (err == EMIG_BAD_ID || err == EOPNOTSUPP)
	  {
	    int wait = 0;
	    va_end (ap);
	    switch (cmd)
	      {
	      case F_GETLK64:
		return __hurd_fail (ENOSYS);
	      case F_SETLKW64:
		wait = 1;
		/* FALLTHROUGH */
	      case F_SETLK64:
		return __f_setlk (fd, fl->l_type, fl->l_whence,
				  fl->l_start, fl->l_len, wait);
	      default:
		return __hurd_fail (EINVAL);
	      }
	  }

	result = err ? __hurd_dfail (fd, err) : 0;
	break;
      }

    case F_GETFL:		/* Get per-open flags.  */
      if (err = HURD_FD_PORT_USE (d, __io_get_openmodes (port, &result)))
	result = __hurd_dfail (fd, err);
      break;

    case F_SETFL:		/* Set per-open flags.  */
      err = HURD_FD_PORT_USE (d, __io_set_all_openmodes (port,
							 va_arg (ap, int)));
      result = err ? __hurd_dfail (fd, err) : 0;
      break;

    case F_GETOWN:		/* Get owner.  */
      if (err = HURD_FD_PORT_USE (d, __io_get_owner (port, &result)))
	result = __hurd_dfail (fd, err);
      break;

    case F_SETOWN:		/* Set owner.  */
      err = HURD_FD_PORT_USE (d, __io_mod_owner (port, va_arg (ap, pid_t)));
      result = err ? __hurd_dfail (fd, err) : 0;
      break;
    }

  va_end (ap);

  return result;
}
libc_hidden_def (__libc_fcntl)

#ifndef NOCANCEL
weak_alias (__libc_fcntl, __fcntl)
libc_hidden_weak (__fcntl)
weak_alias (__libc_fcntl, fcntl)

strong_alias (__libc_fcntl, __libc_fcntl64)
libc_hidden_def (__libc_fcntl64)
weak_alias (__libc_fcntl64, __fcntl64)
libc_hidden_weak (__fcntl64)
weak_alias (__fcntl64, fcntl64)
#endif
