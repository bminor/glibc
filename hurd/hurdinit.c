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

#include <hurd.h>


/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.

   PORTARRAY and INTARRAY are vm_deallocate'd.  */

void
_hurd_init (char **argv,
	    mach_port_t *portarray, size_t portarraysize,
	    int *intarray, size_t intarraysize)
{
  int i;

  _hurd_ports = malloc (portarraysize * sizeof (*_hurd_ports));
  if (_hurd_ports == NULL)
    __libc_fatal ("Can't allocate _hurd_ports\n");
  _hurd_nports = portarraysize;

  /* See what ports we were passed.  */
  for (i = 0; i < portarraysize; ++i)
    {
      _hurd_port_init (&_hurd_ports[i], portarray[i]);

      switch (i)
	{
	case INIT_PORT_PROC:
	  /* Tell the proc server we exist.  */
	  _hurd_proc_init (argv);
	  break;

	case INIT_PORT_BOOTSTRAP:
	  /* When the user asks for the bootstrap port,
	     he will get the one the exec server passed us.  */
	  __task_set_special_port (__mach_task_self (),
				   TASK_BOOTSTRAP_PORT, portarray[i]);
	  break;
	}
    }

  if (intarraysize > INIT_UMASK)
    _hurd_umask = intarray[INIT_UMASK] & 0777;
  else
    _hurd_umask = 0022;		/* XXX */

  /* All done with init ints and ports.  */
  __vm_deallocate (__mach_task_self (), intarray, nints * sizeof (int));
  __vm_deallocate (__mach_task_self (),
		   portarray, nports * sizeof (mach_port_t));
}

/* The user can do "int _hide_arguments = 1;" to make
   sure the arguments are never visible with `ps'.  */
int _hide_arguments, _hide_environment;

/* Do startup handshaking with the proc server just installed in _hurd_ports.
   Call _hurdsig_init to set up signal processing.  */

void
_hurd_proc_init (char **argv)
{
  mach_port_t oldmsg, oldtask;
  int dealloc;
  process_t procserver;

  /* Initialize the signal code; Mach exceptions will become signals.
     This function will be a no-op on calls after the first.
     On the first call, it sets up the message port and the signal thread.  */
  _hurdsig_init ();

  /* The signal thread is now prepared to receive messages.
     It is safe to give the port to the proc server.  */

  procserver = _hurd_port_get (&_hurd_ports[INIT_PORT_PROC], &dealloc);

  /* Give the proc server our task and message ports.  */
  __proc_setports (procserver,
		   _hurd_msgport, __mach_task_self (),
		   &oldmsg, &oldtask);

  /* Tell the proc server where our args and environment are.  */
  __proc_setprocargs (procserver,
		      _hide_arguments ? 0 : argv,
		      _hide_environment ? 0 : __environ);

  _hurd_port_free (&_hurd_ports[INIT_PORT_PROC], &dealloc, procserver);

  /* Initialize proc server-assisted fault recovery for the signal thread.  */
  _hurdsig_fault_init ();

  if (oldmsg != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldmsg);
  if (oldtask != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldtask);
}
