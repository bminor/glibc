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


/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.  */

void
_hurd_init (char **argv,
	    mach_port_t *portarray, size_t portarraysize,
	    int *intarray, size_t intarraysize)
{
  int i;

  /* See what ports we were passed.  */
  for (i = 0; i < portarraysize; ++i)
    switch (i)
      {
#define	initport(upper, lower) \
      case INIT_PORT_##upper: \
	_hurd_port_init (&_hurd_##lower, portarray[i]); \
	break

	  /* Install the standard ports in their cells.  */
	initport (CCDIR, ccdir);
	initport (CWDIR, cwdir);
	initport (CRDIR, crdir);
	initport (AUTH, auth);

      case INIT_PORT_PROC:
	/* Install the proc port and tell the proc server we exist.  */
	_hurd_proc_init (portarray[i], argv);
	break;

      case INIT_PORT_BOOTSTRAP:
	/* When the user asks for the bootstrap port,
	   he will get the one the exec server passed us.  */
	__task_set_special_port (__mach_task_self (),
				 TASK_BOOTSTRAP_PORT, portarray[i]);
	/* FALLTHROUGH */

      default:
	/* Wonder what that could be.  */
	__mach_port_deallocate (__mach_task_self (), portarray[i]);
	break;
      }

  if (intarraysize > INIT_UMASK)
    _hurd_umask = intarray[INIT_UMASK] & 0777;
  else
    _hurd_umask = 0022;		/* XXX */
  if (intarraysize > INIT_CTTY_FILEID) /* Knows that these are sequential.  */
    {
      _hurd_ctty_fstype = intarray[INIT_CTTY_FSTYPE];
      _hurd_ctty_fsid.val[0] = intarray[INIT_CTTY_FSID1];
      _hurd_ctty_fsid.val[1] = intarray[INIT_CTTY_FSID2];
      _hurd_ctty_fileid = intarray[INIT_CTTY_FILEID];
    }


  /* All done with init ints and ports.  */
  __vm_deallocate (__mach_task_self (), intarray, nints * sizeof (int));
  __vm_deallocate (__mach_task_self (),
		   portarray, nports * sizeof (mach_port_t));
}

/* Make PROCSERVER be our proc server port.
   Tell the proc server that we exist.  */

void
_hurd_proc_init (process_t procserver, char **argv)
{
  mach_port_t oldsig, oldtask;

  _hurd_port_init (&_hurd_proc, procserver);

  /* Tell the proc server where our args and environment are.  */
  __proc_setprocargs (procserver, argv, __environ);

  /* Initialize the signal code; Mach exceptions will become signals.
     This sets _hurd_sigport; it must be run before _hurd_proc_init.  */
  _hurdsig_init ();

  /* Give the proc server our task and signal ports.  */
  __proc_setports (procserver,
		   _hurd_sigport, __mach_task_self (),
		   &oldsig, &oldtask);
  if (oldsig != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldsig);
  if (oldtask != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldtask);
}
