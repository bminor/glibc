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

#include <ansidecl.h>
#include <hurd.h>
#include <hurd/term.h>
#include <hurd/fd.h>
#include <gnu-stabs.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <cthreads.h>		/* For `struct mutex'.  */


struct mutex _hurd_dtable_lock;
struct hurd_dtable _hurd_dtable;
int _hurd_dtable_rlimit;
struct hurd_userlink *_hurd_dtable_users;

void (*_hurd_dtable_deallocate) (void *);


/* Initialize the file descriptor table at startup.  */

static void
init_dtable (void)
{
  register size_t i;
  struct hurd_fd **dt;

  __mutex_init (&_hurd_dtable_lock);

  _hurd_dtable_users = NULL;

  /* The initial size of the descriptor table is that of the passed-in
     table, rounded up to a multiple of FOPEN_MAX descriptors.  */
  _hurd_dtable.size
    = (_hurd_init_dtablesize + FOPEN_MAX - 1) / FOPEN_MAX * FOPEN_MAX;
  _hurd_dtable_rlimit = _hurd_dtable.size;

  /* Allocate the vector of pointers.  */
  dt = _hurd_dtable.d = malloc (_hurd_dtable.size * sizeof (*_hurd_dtable.d));
  if (dt == NULL)
    __libc_fatal ("hurd: Can't allocate file descriptor table\n");

  /* Initialize the descriptor table.  */
  for (i = 0; i < _hurd_init_dtablesize; ++i)
    {
      if (_hurd_init_dtable[i] == MACH_PORT_NULL)
	/* An unused descriptor is marked by a null pointer.  */
	dt[i] = NULL;
      else
	{
	  /* Allocate a new file descriptor structure.  */
	  struct hurd_fd *new = malloc (sizeof (struct hurd_fd));
	  if (new == NULL)
	    __libc_fatal ("hurd: Can't allocate initial file descriptors\n");

	  /* Initialize the port cells.  */
	  _hurd_port_init (&new->port, MACH_PORT_NULL);
	  _hurd_port_init (&new->ctty, MACH_PORT_NULL);

	  /* Install the port in the descriptor.
	     This sets up all the ctty magic.  */
	  _hurd_port2fd (new, _hurd_init_dtable[i], 0);

	  dt[i] = new;
	}
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
    dt[i] = NULL;
}

text_set_element (_hurd_subinit, init_dtable);

/* XXX when the linker supports it, the following functions should all be
   elsewhere and just have text_set_elements here.  */

/* Called by `getdport' to do its work.  */

static file_t
get_dtable_port (int fd)
{
  file_t dport;
  int err = HURD_DPORT_USE (fd, __mach_port_mod_refs (__mach_task_self (),
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

file_t (*_hurd_getdport_fn) (int fd) = get_dtable_port;

#include <hurd/signal.h>

/* Called on fork to install the dtable in NEWTASK.  The dtable lock is
   held now and was taken before the child was created, copying our memory.
   Insert send rights for all of the normal io ports for fds, with the same
   names they have in our task.  We trust that none of the ports in the
   dtable were be changed while we have been holding the lock, so the port
   names copied by the child are still valid in our task.  */

static error_t
fork_parent_dtable (task_t newtask)
{
  error_t err;
  int i;

  err = 0;

  for (i = 0; !err && i < _hurd_dtable.size; ++i)
    {
      struct hurd_userlink ulink, ctty_ulink;
      io_t port = _hurd_port_get (&_hurd_dtable.d[i]->port, &ulink);
      io_t ctty = _hurd_port_get (&_hurd_dtable.d[i]->ctty, &ctty_ulink);

      /* If there is a ctty-special port (it will be in PORT),
	 insert only the normal io port.  The child will get a fresh
	 ctty-special port.  */
      if (ctty != MACH_PORT_NULL)
	err = __mach_port_insert_right (newtask, ctty, ctty,
					MACH_MSG_TYPE_COPY_SEND);
      else if (port != MACH_PORT_NULL)
	/* There is no ctty-special port; PORT is the normal io port.  */
	err = __mach_port_insert_right (newtask, port, port,
					MACH_MSG_TYPE_COPY_SEND);

      _hurd_port_free (&_hurd_dtable.d[i]->port, &ulink, port);
      _hurd_port_free (&_hurd_dtable.d[i]->ctty, &ctty_ulink, ctty);
    }
  return err;
}

/* We are in the child fork; the dtable lock is still held.
   The parent has inserted send rights for all the normal io ports,
   but we must recover ctty-special ports for ourselves.  */
static error_t
fork_child_dtable (void)
{
  error_t err;
  int i;

  err = 0;

  for (i = 0; !err && i < _hurd_dtable.size; ++i)
    {
      struct hurd_fd *d = _hurd_dtable.d[i];

      d->port.users = d->ctty.users = NULL;

      if (d->ctty.port)
	/* There was a ctty-special port in the parent.
	   We need to get one for ourselves too.  */
	  err = __term_become_ctty (d->ctty.port,
				    /* XXX no guarantee that init_pids hook
				       has been run BEFORE this one! */
				    _hurd_pid, _hurd_pgrp, _hurd_msgport,
				    &d->port.port);

      /* XXX for each fd with a cntlmap, reauth and re-map_cntl.  */
    }
  return err;
}

text_set_element (_hurd_fork_locks, _hurd_dtable_lock);
text_set_element (_hurd_fork_setup_hook, fork_parent_dtable);
text_set_element (_hurd_fork_child_hook, fork_child_dtable);

/* Called when our process group has changed.  */

static void
ctty_new_pgrp (void)
{
  int i;
  
  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct hurd_fd *const d = _hurd_dtable.d[i];
      struct hurd_userlink ulink, ctty_ulink;
      io_t port, ctty;

      if (d == NULL)
	/* Nothing to do for an unused descriptor cell.  */
	continue;

      port = _hurd_port_get (&d->port, &ulink);
      ctty = _hurd_port_get (&d->ctty, &ctty_ulink);

      if (ctty)
	{
	  /* This fd has a ctty-special port.  We need a new one, to tell
             the io server of our different process group.  */
	  io_t new;
	  if (! __term_become_ctty (ctty, _hurd_pid, _hurd_pgrp, _hurd_msgport,
				    &new))
	    _hurd_port_set (&d->port, new);
	}

      _hurd_port_free (&d->port, &ulink, port);
      _hurd_port_free (&d->ctty, &ctty_ulink, ctty);
    }

  __mutex_unlock (&_hurd_dtable_lock);
  HURD_CRITICAL_END;
}

text_set_element (_hurd_pgrp_changed_hook, ctty_new_pgrp);

/* Called to reauthenticate the dtable when the auth port changes.  */

static void
reauth_dtable (void)
{
  int i;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct hurd_fd *const d = _hurd_dtable.d[i];
      mach_port_t new, newctty;
      
      if (d == NULL)
	/* Nothing to do for an unused descriptor cell.  */
	continue;

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
  HURD_CRITICAL_END;
}

text_set_element (_hurd_reauth_hook, reauth_dtable);

#if 0

#include <hurd/signal.h>

static void
rectty_dtable (mach_port_t cttyid)
{
  int i;
  
  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct hurd_fd *const d = _hurd_dtable.d[i];
      mach_port_t newctty;

      if (d == NULL)
	/* Nothing to do for an unused descriptor cell.  */
	continue;

      if (cttyid == MACH_PORT_NULL)
	/* We now have no controlling tty at all.  */
	newctty = MACH_PORT_NULL;
      else
	HURD_PORT_USE (&d->port,
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
  HURD_CRITICAL_END;
}

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
  err = HURD_DPORT_USE (fd,
			ctty ? EADDRINUSE : __term_getctty (port, &cttyid));
  if (err == EADDRINUSE)
    /* FD is already the ctty.  Nothing to do.  */
    return 0;
  else if (err)
    return __hurd_fail (err);

  /* Make it our own.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], cttyid); /* Consumes ref.  */

  /* Reset all the ctty ports in all the descriptors.  */
  __USEPORT (CTTYID, (rectty_dtable (port), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocsctty, TIOCSCTTY);

/* Dissociate from the controlling terminal.  */

static int
tiocnotty (int fd,
	   int request,		/* Always TIOCNOTTY.  */
	   void *arg)		/* Not used.  */
{
  mach_port_t fd_cttyid;
  error_t err;

  if (err = HURD_DPORT_USE (fd, __term_getctty (port, &fd_cttyid)))
    return __hurd_fail (err);

  if (__USEPORT (CTTYID, port != fd_cttyid))
    err = EINVAL;

  __mach_port_deallocate (__mach_task_+self (), fd_cttyid);

  if (err)
    return __hurd_fail (err);

  /* Clear our cttyid port cell.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], MACH_PORT_NULL);

  /* Reset all the ctty ports in all the descriptors.  */
				
  __USEPORT (CTTYID, (rectty_dtable (MACH_PORT_NULL), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocnotty, TIOCNOTTY);

#endif
