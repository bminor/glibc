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
  int i;
  char *const *p;
  task_t task;
  int flags;
  struct _hurd_sigstate *ss;

  file = __hurd_path_lookup (path, FS_LOOKUP_EXECUTE, 0);
  if (file == MACH_PORT_NULL)
    return -1;

  argslen = 0;
  p = argv;
  while (*p != NULL)
    argslen += strlen (*p++) + 1;
  args = __alloca (argslen);
  ap = args;
  for (p = argv; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', UINT_MAX);

  envlen = 0;
  p = envp;
  while (*p != NULL)
    envlen += strlen (*p++) + 1;
  env = __alloca (envlen);
  ap = env;
  for (p = envp; *p != NULL; ++p)
    ap = __memccpy (ap, *p, '\0', UINT_MAX);

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
  for (i = 0; i < INIT_PORT_MAX; ++i)
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

  if (ss->vforked)
    {
      /* This thread is vfork'd.  */
      task = MACH_PORT_NULL;
      flags = FS_EXEC_NEWTASK;
    }
  else
    {
      task = __mach_task_self ();
      flags = 0;
    }
  
  __mutex_lock (&_hurd_dtable_lock);
  if (_hurd_dtable.d != NULL)
    {
      dtablesize = _hurd_dtable.size;
      dtable = __alloca (dtablesize * sizeof (file_t));
      for (i = 0; i < dtablesize; ++i)
	if (_hurd_dtable.d[i].flags & FD_CLOEXEC)
	  dtable[i] = MACH_PORT_NULL;
	else
	  dtable[i] = _hurd_dtable.d[i].server;
    }
  else
    {
      dtable = _hurd_init_dtable;
      dtablesize = _hurd_init_dtablesize;
    }

  err = __file_exec (file, task,
		     args, argslen, env, envlen,
		     dtable, dtablesize,
		     ints, INIT_INT_MAX,
		     ports, INIT_PORT_MAX,
		     flags);
  /* We must hold the dtable lock while doing the file_exec to avoid
     the dtable entries being deallocated before we send them.  */
  __mutex_unlock (&_hurd_dtable_lock);

  for (i = 0; i < INIT_PORT_MAX; ++i)
    if (port_cells[i] != NULL)
      _hurd_port_free (ports_cells[i], ports[i], dealloc_ports[i]);

  if (err)
    return __hurd_fail (err);

  if (ss->vforked)
    longjmp (ss->vfork_saved.continuation, 1);

  __mutex_unlock (&ss->lock);

  /* That's interesting.  */
  return 0;
}
