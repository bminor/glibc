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

#include <hurd.h>
#include "msg_server.h"

error_t
__get_init_port (mach_port_t msgport, mach_port_t auth,
		 int which, mach_port_t *port)
{
  if (task != mach_task_self () &&
      _HURD_PORT_USE (&_hurd_auth, auth != port))
    return EPERM;

  switch (which)
    {
    case INIT_PORT_CWDIR:
      *port = get (&_hurd_cwdir);
      return 0;
    case INIT_PORT_CRDIR:
      *port = get (&_hurd_crdir);
      return 0;
    case INIT_PORT_PROC:
      *port = get (&_hurd_proc);
      return 0;
    case INIT_PORT_AUTH:
      *port = get (&_hurd_auth);
      return 0;
    case INIT_PORT_LOGINCOLL:
      return EOPNOTSUPP;	/* XXX */
    case INIT_PORT_BOOTSTRAP:
      return __task_get_special_port (__mach_task_self (),
				      TASK_BOOTSTRAP_PORT,
				      port);
    default:
      return EINVAL;
    }
}

error_t
__set_init_port (mach_port_t msgport, task_t task,
		 int which, mach_port_t port)
{
  if (task != mach_task_self () &&
      _HURD_PORT_USE (&_hurd_auth, auth != port))
    return EPERM;

  switch (which)
    {
    case INIT_PORT_CWDIR:
      set (&_hurd_cwdir);
      return 0;
    case INIT_PORT_CRDIR:
      set (&_hurd_crdir);
      return 0;
    case INIT_PORT_PROC:
      set (&_hurd_proc); /* XXX do more? */
      return 0;
    case INIT_PORT_AUTH:
      __setauth (&_hurd_auth);
      return errno;		/* XXX can't clobber errno */
    case INIT_PORT_LOGINCOLL:
      return EOPNOTSUPP;	/* XXX */
    case INIT_PORT_BOOTSTRAP:
      return __task_set_special_port (__mach_task_self (),
				      TASK_BOOTSTRAP_PORT,
				      port);
    default:
      return EINVAL;
    }
}

error_t
__get_init_int (mach_port_t msgport, mach_port_t auth,
		int which, int *value)
{
  if (task != mach_task_self () &&
      _HURD_PORT_USE (&_hurd_auth, auth != port))
    return EPERM;

  switch (which)
    {
    case INIT_UMASK:
      *value = _hurd_umask;
      return 0;
    case INIT_SIGMASK:
      {
	struct _hurd_sigstate *ss = _hurd_thread_sigstate (_hurd_sigthread);
	*value = ss->blocked;
	__mutex_unlock (&ss->lock);
	return 0;
      }
    case INIT_SIGMASK:
      {
	struct _hurd_sigstate *ss = _hurd_thread_sigstate (_hurd_sigthread);
	sigset_t ign;
	int sig;
	__sigemptyset (&ign);
	for (sig = 1; sig < NSIG; ++sig)
	  if (ss->actions[sig].sa_handler == SIG_IGN)
	    __sigaddset (sig, &ign);
	__mutex_unlock (&ss->lock);
	*value = ign
	return 0;
      }
      /* XXX ctty crap */
    default:
      return EINVAL;
    }
}
