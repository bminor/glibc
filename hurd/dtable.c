/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <hurd/term.h>
#include <gnu-stabs.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>


#ifdef noteven
struct mutex _hurd_dtable_lock;
#endif
struct _hurd_dtable _hurd_dtable;
int _hurd_dtable_rlimit;
int *_hurd_dtable_user_dealloc;

const struct _hurd_dtable_resizes _hurd_dtable_resizes;


/* Initialize the file descriptor table at startup.  */

static void
init_dtable (void)
{
  register size_t i;

#ifdef noteven
  __mutex_init (&_hurd_dtable_lock);
#endif

  _hurd_dtable_user_dealloc = NULL;

  /* The initial size of the descriptor table is that of the passed-in
     table, rounded up to a multiple of FOPEN_MAX descriptors.  */
  _hurd_dtable.size
    = (_hurd_init_dtablesize + FOPEN_MAX - 1) / FOPEN_MAX * FOPEN_MAX;
  _hurd_dtable_rlimit = _hurd_dtable.size;

  _hurd_dtable.d = malloc (_hurd_dtable.size * sizeof (*_hurd_dtable.d));
  if (_hurd_dtable.d == NULL)
    __libc_fatal ("hurd: Can't allocate file descriptor table\n");

  for (i = 0; i < _hurd_init_dtablesize; ++i)
    {
      struct _hurd_fd *const d = &_hurd_dtable.d[i];

      _hurd_port_init (&d->port, MACH_PORT_NULL);
      _hurd_port_init (&d->ctty, MACH_PORT_NULL);

      _hurd_port2fd (d, _hurd_init_dtable[i], 0);
    }

  /* Clear out the initial descriptor table.
     Everything must use _hurd_dtable now.  */
  __vm_deallocate (__mach_task_self (),
		   (vm_address_t) _hurd_init_dtable,
		   _hurd_init_dtablesize * sizeof (_hurd_init_dtable[0]));
  _hurd_init_dtable = NULL;
  _hurd_init_dtablesize = 0;

  /* Initialize the remaining empty slots in the table.  */
  for (; i < _hurd_dtable.size; ++i)
    {
      _hurd_port_init (&_hurd_dtable.d[i].port, MACH_PORT_NULL);
      _hurd_port_init (&_hurd_dtable.d[i].ctty, MACH_PORT_NULL);
      _hurd_dtable.d[i].flags = 0;
    }
}

text_set_element (__libc_subinit, init_dtable);

/* Allocate a new file descriptor and install PORT in it.  FLAGS are as for
   `open'; only O_IGNORE_CTTY is meaningful, but all are saved.

   If the descriptor table is full, set errno, and return -1.
   If DEALLOC is nonzero, deallocate PORT first.  */
int
_hurd_intern_fd (io_t port, int flags, int dealloc)
{
  int fd;
  struct _hurd_fd *d = _hurd_alloc_fd (&fd, 0);

  if (d == NULL)
    {
      if (dealloc)
	__mach_port_deallocate (__mach_task_self (), port);
      return -1;
    }

  _hurd_port2fd (d, port, flags);
  __spin_unlock (&d->port.lock);
  return fd;
}

/* Allocate a new file descriptor and return it, locked.
   If the table is full, set errno and return NULL.  */
struct _hurd_fd *
_hurd_alloc_fd (int *fd, const int first_fd)
{
  int i;

  __mutex_lock (&hurd_dtable_lock);

  for (i = first_fd; i < _hurd_dtable.size; ++i)
    {
      struct _hurd_fd *d = &_hurd_dtable.d[i];
      __spin_lock (&d->port.lock);
      if (d->port.port == MACH_PORT_NULL)
	{
	  __mutex_unlock (&hurd_dtable_lock);
	  if (fd != NULL)
	    *fd = i;
	  return d;
	}
      else
	__spin_unlock (&d->port.lock);
    }

  __mutex_unlock (&hurd_dtable_lock);

  errno = EMFILE;
  return NULL;
}


void
_hurd_port2fd (struct _hurd_fd *d, io_t port, int flags)
{
  io_t ctty;
  mach_port_t cttyid;
  int is_ctty = !(flags & O_IGNORE_CTTY) && ! __term_getctty (port, &cttyid);

  if (is_ctty)
    {
      /* This port is capable of being a controlling tty.
	 Is it ours?  */
      is_ctty &= __USEPORT (CTTYID, port == cttyid);
      __mach_port_deallocate (__mach_task_self (), cttyid);
#if 0
      struct _hurd_port *const id = &_hurd_ports[INIT_PORT_CTTYID];
      __spin_lock (&id->lock);
      if (id->port == MACH_PORT_NULL)
	/* We have no controlling tty, so make this one it.  */
	_hurd_port_locked_set (id, cttyid);
      else
	{
	  if (cttyid != id->port)
	    /* We have a controlling tty and this is not it.  */
	    is_ctty = 0;
	  /* Either we don't want CTTYID, or ID->port already is it.
	     So we don't need to change ID->port, and we
	     can release the reference to CTTYID.  */
	  __spin_unlock (&id->lock);
	  __mach_port_deallocate (__mach_task_self (), cttyid);
	}
#endif
    }

  if (is_ctty && ! __term_become_ctty (port, _hurd_pid, _hurd_pgrp,
				       _hurd_msgport, &ctty))
    {
      /* Operations on CTTY return EBACKGROUND when we are not a
	 foreground user of the tty.  */
      d->port.port = ctty;
      ctty = port;
    }
  else
    /* XXX if IS_CTTY, then this port is our ctty, but we are
       not doing ctty style i/o because term_become_ctty barfed.
       What to do?  */
    /* No ctty magic happening here.  */
    ctty = MACH_PORT_NULL;

  _hurd_port_set (&d->ctty, ctty);
}

/* Called by `getdport' to do its work.  */

static file_t
get_dtable_port (int fd)
{
  file_t dport;
  int err = _HURD_DPORT_USE (fd,
			     __mach_port_mod_refs (__mach_task_self (),
						   (dport = port),
						   MACH_PORT_RIGHT_SEND,
						   1));
  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  else
    return dport;
}

text_set_element (_hurd_getdport_fn, get_dtable_port);

/* Called on fork to install the dtable in NEWTASK.
   The dtable lock is held.  */

static error_t
fork_dtable (task_t newtask)
{
  error_t err;
  int i;

  err = 0;

  for (i = 0; !err && i < _hurd_dtable.size; ++i)
    {
      int dealloc, dealloc_ctty;
      io_t port = _hurd_port_get (&_hurd_dtable.d[i].port, &dealloc);
      io_t ctty = _hurd_port_get (&_hurd_dtable.d[i].ctty, &dealloc_ctty);

      if (port != MACH_PORT_NULL)
	err = __mach_port_insert_right (newtask, port, port,
					MACH_MSG_TYPE_COPY_SEND);
      if (!err && ctty != MACH_PORT_NULL)
	err = __mach_port_insert_right (newtask, ctty, ctty,
					MACH_MSG_TYPE_COPY_SEND);

      _hurd_port_free (&_hurd_dtable.d[i].port, &dealloc, port);
      _hurd_port_free (&_hurd_dtable.d[i].ctty, &dealloc_ctty, ctty);

      /* XXX for each fd with a cntlmap, reauth and re-map_cntl.  */
    }
  __mutex_unlock (&_hurd_dtable_lock);
  return err;
}

text_set_element (_hurd_fork_hook, fork_dtable);
text_set_element (_hurd_fork_locks, _hurd_dtable_lock);

/* Called to reauthenticate the dtable when the auth port changes.  */

static void
reauth_dtable (void)
{
  int i;

  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct _hurd_fd *const d = &_hurd_dtable.d[i];
      mach_port_t new, newctty;
      
      /* Take the descriptor cell's lock.  */
      __spin_lock (&d->port.lock);
      
      /* Reauthenticate the descriptor's port.  */
      if (d->port.port != MACH_PORT_NULL &&
	  ! __io_reauthenticate (d->port.port, _hurd_pid) &&
	  ! __USEPORT (AUTH, __auth_user_authenticate (port,
						       d->port.port, _hurd_pid,
						       &new)))
	{
	  /* Replace the port in the descriptor cell
	     with the newly reauthenticated port.  */

	  if (d->ctty.port != MACH_PORT_NULL &&
	      ! __io_reauthenticate (d->ctty.port, _hurd_pid) &&
	      ! __USEPORT (AUTH, __auth_user_authenticate (port,
							   d->ctty.port,
							   _hurd_pid,
							   &newctty)))
	    _hurd_port_set (&d->ctty, newctty);

	  _hurd_port_locked_set (&d->port, new);
	}
      else
	/* Lost.  Leave this descriptor cell alone.  */
	__spin_unlock (&d->port.lock);
    }

  __mutex_unlock (&_hurd_dtable_lock);
}

text_set_element (_hurd_reauth_hook, reauth_dtable);

static void
rectty_dtable (mach_port_t cttyid)
{
  int i;
  
  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct _hurd_fd *const d = &_hurd_dtable.d[i];
      mach_port_t newctty;

      if (cttyid == MACH_PORT_NULL)
	/* We now have no controlling tty at all.  */
	newctty = MACH_PORT_NULL;
      else
	_HURD_PORT_USE (&d->port,
			({ mach_port_t id;
			   /* Get the io object's cttyid port.  */
			   if (! __term_getctty (port, &id))
			     {
			       if (id == cttyid && /* Is it ours?  */
				   /* Get the ctty io port.  */
				   __term_become_ctty (port, _hurd_pid,
						       _hurd_pgrp,
						       _hurd_msgport,
						       &newctty))
				 /* XXX it is our ctty but the call failed? */
				 newctty = MACH_PORT_NULL;
			       __mach_port_deallocate
				 (__mach_task_self (), (mach_port_t) id);
			     }
			   else
			     newctty = MACH_PORT_NULL;
			   0;
			 }));

      /* Install the new ctty port.  */
      _hurd_port_set (&d->ctty, newctty);
    }

  __mutex_unlock (&_hurd_dtable_lock);
}

#if 0

#include <sys/ioctl.h>


/* Make FD be the controlling terminal.
   This function is called for `ioctl (fd, TCIOSCTTY)'.  */

static int
tiocsctty (int fd,
	   int request,		/* Always TCIOSCTTY.  */
	   void *arg)		/* Not used.  */
{
  mach_port_t cttyid;
  error_t err;

  /* Get FD's cttyid port, unless it is already ours.  */
  err = _HURD_DPORT_USE (fd,
			 ctty ? EADDRINUSE : __term_getctty (port, &cttyid));
  if (err == EADDRINUSE)
    /* FD is already the ctty.  Nothing to do.  */
    return 0;
  else if (err)
    return err;

  /* Make it our own.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], cttyid); /* Consumes ref.  */

  /* Reset all the ctty ports in all the descriptors.  */
  _HURD_PORT_USE (&_hurd_ports[INIT_PORT_CTTYID], (rectty_dtable (port), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocsctty, TIOCSCTTY);

#ifdef TIOCNOCTTY
/* Dissociate from the controlling terminal.  */

static int
tiocnoctty (int fd,
	    int request,	/* Always TIOCNOCTTY.  */
	    void *arg)		/* Not used.  */
{
  /* XXX should verify that FD is ctty and return EINVAL? */

  /* Clear our cttyid port cell.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], MACH_PORT_NULL);

  /* Reset all the ctty ports in all the descriptors.  */
  _HURD_PORT_USE (&_hurd_ports[INIT_PORT_CTTYID],
		  (rectty_dtable (MACH_PORT_NULL), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocnotty, TIOCNOTTY);
#endif

#endif
