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
#include <stddef.h>
#include <unistd.h>
#include <hurd.h>

/* Replace the current process, executing PATH with arguments ARGV and
   environment ENVP.  ARGV and ENVP are terminated by NULL pointers.  */
int
DEFUN(__execve, (path, argv, envp),
      CONST char *path AND char *CONST argv[] AND char *CONST envp[])
{
  error_t err;
  file_t file;
  char *args, *env, *ap;
  size_t argslen, envlen;
  int ints[INIT_INT_MAX];
  mach_port_t ports[INIT_PORT_MAX];
  int dealloc_ports[INIT_PORT_MAX];
  struct _hurd_port *port_cells[INIT_PORT_MAX];
  file_t *dtable;
  int dtablesize;
  struct _hurd_port *dtable_cells;
  int *dealloc_dtable;
  int i;
  char *const *p;
  task_t task;
  int flags;
  struct _hurd_sigstate *ss;

  /* Get a port to the file we want to execute.  */
  file = __path_lookup (path, FS_LOOKUP_EXECUTE, 0);
  if (file == MACH_PORT_NULL)
    return -1;

  /* Pack the arguments into an array with nulls separating the elements.  */
  argslen = 0;
  p = argv;
  while (*p != NULL)
    argslen += strlen (*p++) + 1;
  args = __alloca (argslen);
  ap = args;
  for (p = argv; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', ULONG_MAX);

  /* Pack the environment into an array with nulls separating the elements.  */
  envlen = 0;
  p = envp;
  while (*p != NULL)
    envlen += strlen (*p++) + 1;
  env = __alloca (envlen);
  ap = env;
  for (p = envp; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', ULONG_MAX);

  /* Load up the ports to give to the new program.  */
#define	port(idx, cell)							      \
      case idx:								      \
	port_cells[i] = &cell;						      \
	ports[i] = _hurd_port_get (&cell, &dealloc_ports[i]);		      \
	break
  for (i = 0; i < INIT_PORT_MAX; ++i)
    switch (i)
      {
	port (INIT_PORT_CCDIR, _hurd_ccdir);
	port (INIT_PORT_CWDIR, _hurd_cwdir);
	port (INIT_PORT_CRDIR, _hurd_crdir);
	port (INIT_PORT_AUTH, _hurd_auth);
	port (INIT_PORT_PROC, _hurd_proc);
      default:
	port_cells[i] = NULL;
	break;
      }

  /* Load up the ints to give the new program.  */
  for (i = 0; i < INIT_INT_MAX; ++i)
    switch (i)
      {
      case INIT_UMASK:
	ints[i] = _hurd_umask;
	break;
      case INIT_CTTY_FSTYPE:
	ints[i] = _hurd_ctty_fstype;
	break;
      case INIT_CTTY_FSID1:
	ints[i] = _hurd_ctty_fsid.val[0];
	break;
      case INIT_CTTY_FSID2:
	ints[i] = _hurd_ctty_fsid.val[1];
	break;
      case INIT_CTTY_FILEID:
	ints[i] = _hurd_ctty_fileid;
	break;

      case INIT_SIGMASK:
      case INIT_SIGIGN:
	break;

      default:
	ints[i] = 0;
      }

  ss = _hurd_thread_sigstate (__mach_thread_self ());
  ints[INIT_SIGMASK] = ss->blocked;
  ints[INIT_SIGIGN] = 0;
  for (i = 1; i < NSIG; ++i)
    if (ss->actions[i].sa_handler == SIG_IGN)
      ints[INIT_SIGIGN] |= __sigmask (i);

  /* We hold the sigstate lock until the exec has failed so that no signal
     can arrive between when we pack the blocked and ignored signals, and
     when the exec actually happens.  A signal handler could change what
     signals are blocked and ignored.  Either the change will be reflected
     in the exec, or the signal will never be delivered.  */

#if 0
  if (ss->vforked)
    {
      /* This thread is vfork'd.  */
      task = MACH_PORT_NULL;
      flags = EXEC_NEWTASK;
    }
  else
#endif
    {
      task = __mach_task_self ();
      flags = 0;
    }
  
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
	  struct _hurd_fd *const = &_hurd_dtable.d[i];
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

  err = __file_exec (file, task,
		     args, argslen, env, envlen,
		     dtable, dtablesize,
		     ints, INIT_INT_MAX,
		     ports, INIT_PORT_MAX,
		     flags);

  /* Safe to let signals happen now.  */
  __mutex_unlock (&ss->lock);

  for (i = 0; i < INIT_PORT_MAX; ++i)
    if (port_cells[i] != NULL)
      _hurd_port_free (ports_cells[i], ports[i], &dealloc_ports[i]);

  if (dealloc_dtable != NULL)
    for (i = 0; i < dtablesize; ++i)
      if (dtable[i] != MACH_PORT_NULL)
	_hurd_port_free (dtable_cells[i], dtable[i], &dealloc_dtable[i]);

#if 0
  if (ss->vforked)
    longjmp (ss->vfork_saved.continuation, 1);
#endif

  if (err)
    return __hurd_fail (err);

  /* That's interesting.  */
  return 0;
}
