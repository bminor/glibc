/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Replace the current process, executing FILE with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
error_t
_hurd_exec (file_t file, char *const argv[], char *const envp[])
{
  error_t err;
  char *args, *env, *ap;
  size_t argslen, envlen;
  int ints[INIT_INT_MAX];
  mach_port_t ports[_hurd_nports];
  int dealloc_ports[_hurd_nports];
  file_t *dtable;
  int dtablesize;
  struct _hurd_port **dtable_cells;
  int *dealloc_dtable, *dealloc_cells;
  int i;
  char *const *p;
  struct _hurd_sigstate *ss;

  /* Pack the arguments into an array with nulls separating the elements.  */
  argslen = 0;
  p = argv;
  while (*p != NULL)
    argslen += strlen (*p++) + 1;
  args = __alloca (argslen);
  ap = args;
  for (p = argv; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', ULONG_MAX);

  /* Pack the environment into an array with nulls separating elements.  */
  envlen = 0;
  p = envp;
  while (*p != NULL)
    envlen += strlen (*p++) + 1;
  env = __alloca (envlen);
  ap = env;
  for (p = envp; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', ULONG_MAX);

  /* Load up the ports to give to the new program.  */
  for (i = 0; i < _hurd_nports; ++i)
    ports[i] = _hurd_port_get (&_hurd_ports[i], &dealloc_ports[i]);

  /* Load up the ints to give the new program.  */
  for (i = 0; i < INIT_INT_MAX; ++i)
    switch (i)
      {
      case INIT_UMASK:
	ints[i] = _hurd_umask;
	break;

      case INIT_SIGMASK:
      case INIT_SIGIGN:
      case INIT_SIGPENDING:
	/* We will set these all below.  */
	break;

      default:
	ints[i] = 0;
      }

  ss = _hurd_thread_sigstate (__mach_thread_self ());
  ints[INIT_SIGMASK] = ss->blocked;
  ints[INIT_SIGPENDING] = ss->pending;
  ints[INIT_SIGIGN] = 0;
  for (i = 1; i < NSIG; ++i)
    if (ss->actions[i].sa_handler == SIG_IGN)
      ints[INIT_SIGIGN] |= __sigmask (i);

  /* We hold the sigstate lock until the exec has failed so that no signal
     can arrive between when we pack the blocked and ignored signals, and
     when the exec actually happens.  A signal handler could change what
     signals are blocked and ignored.  Either the change will be reflected
     in the exec, or the signal will never be delivered.  */
  
  /* Pack up the descriptor table to give the new program.  */
  __mutex_lock (&_hurd_dtable_lock);
  if (_hurd_dtable.d != NULL)
    {
      dtablesize = _hurd_dtable.size;
      dtable = __alloca (dtablesize * sizeof (dtable[0]));
      dealloc_dtable = __alloca (dtablesize * sizeof (dealloc_dtable[0]));
      dtable_cells = __alloca (dtablesize * sizeof (dealloc_cells[0]));
      for (i = 0; i < dtablesize; ++i)
	{
	  struct _hurd_fd *const d = &_hurd_dtable.d[i];
	  __spin_lock (&d->port.lock);
	  if (d->flags & FD_CLOEXEC)
	    {
	      dtable[i] = MACH_PORT_NULL;
	      __spin_unlock (&d->port.lock);
	    }
	  else
	    {
	      /* If this is a descriptor to our controlling tty,
		 we want to give the normal port, not the foreground port.  */
	      dtable[i] = _hurd_port_get (&d->ctty, &dealloc_dtable[i]);
	      if (dtable[i] == MACH_PORT_NULL)
		{
		  dtable[i] = _hurd_port_locked_get (&d->port,
						     &dealloc_dtable[i]);
		  dtable_cells[i] = &d->port;
		}
	      else
		{
		  __spin_unlock (&d->port.lock);
		  dtable_cells[i] = &d->ctty;
		}
	    }
	}
    }
  else
    {
      dtable = _hurd_init_dtable;
      dtablesize = _hurd_init_dtablesize;
      dealloc_dtable = NULL;
      dealloc_cells = NULL;
    }

  /* The information is all set up now.  Try to exec the file.  */

  {
    mach_port_t please_dealloc[_hurd_nports + dtablesize];
    mach_port_t *p = please_dealloc;

    /* Request the exec server to deallocate some ports from us if the exec
       succeeds.  The init ports and descriptor ports will arrive in the
       new program's exec_startup message.  If we failed to deallocate
       them, the new program would have duplicate user references for them.
       But we cannot deallocate them ourselves, because we must still have
       them after a failed exec call.  */

    for (i = 0; i < _hurd_nports; ++i)
      *p++ = ports[i];
    for (i = 0; i < dtablesize; ++i)
      *p++ = dtable[i];

    err = __file_exec (file, __mach_task_self (),
		       0,
		       args, argslen, env, envlen,
		       dtable, dtablesize, MACH_MSG_TYPE_COPY_SEND,
		       ports, _hurd_nports, MACH_MSG_TYPE_COPY_SEND,
		       ints, INIT_INT_MAX,
		       please_dealloc, p - please_dealloc,
		       NULL, 0);
  }

  /* Safe to let signals happen now.  */
  __mutex_unlock (&ss->lock);

  /* Release references to the standard ports.  */
  for (i = 0; i < _hurd_nports; ++i)
    _hurd_port_free (&_hurd_ports[i], &dealloc_ports[i], ports[i]);

  if (dealloc_dtable != NULL)
    /* Release references to the file descriptor ports.  */
    for (i = 0; i < dtablesize; ++i)
      if (dtable[i] != MACH_PORT_NULL)
	_hurd_port_free (dtable_cells[i], &dealloc_dtable[i], dtable[i]);

  return err;
}
