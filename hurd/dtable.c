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
#include <hurd.h>
#include <gnu-stabs.h>
#include <stdlib.h>
#include <limits.h>


struct _hurd_dtable _hurd_dtable;
struct mutex _hurd_dtable_lock;
int *_hurd_dtable_user_dealloc;

const struct _hurd_dtable_resizes _hurd_dtable_resizes;


/* Initialize the file descriptor table at startup.  */

static void
init_dtable (void)
{
  register size_t i;

  __mutex_init (&_hurd_dtable_lock);

  _hurd_dtable_user_dealloc = NULL;

  /* The initial size of the descriptor table is that of the passed-in
     table, rounded up to a multiple of OPEN_MAX descriptors.  */
  _hurd_dtable.size
    = (_hurd_init_dtablesize + OPEN_MAX - 1) / OPEN_MAX * OPEN_MAX;

  _hurd_dtable.d = malloc (_hurd_dtable.size * sizeof (*_hurd_dtable.d));
  if (_hurd_dtable.d == NULL)
    __libc_fatal ("hurd: Can't allocate file descriptor table\n");

  for (i = 0; i < _hurd_init_dtablesize; ++i)
    {
      struct _hurd_fd *const d = &_hurd_dtable.d[i];
      io_statbuf_t stb;
      io_t ctty;

      _hurd_port_init (&d->port, _hurd_init_dtable[i]);
      d->flags = 0;

      if (_hurd_ctty_fstype != 0 &&
	  /* We have a controlling tty.  Is this it?  */
	  ! __io_stat (d->port.port, &stb) &&
	  stb.stb_fstype == _hurd_ctty_fstype &&
	  stb.stb_fsid == _hurd_ctty_fsid &&
	  stb.stb_fileid == _hurd_ctty_fileid &&
	  /* This is a descriptor to our controlling tty.  */
	  ! __term_become_ctty (d->port.port, _hurd_pid, _hurd_pgrp,
				_hurd_sigport, &ctty))
	{
	  /* Operations on CTTY return EBACKGROUND when we are not a
	     foreground user of the tty.  */
	  d->port.port = ctty;
	  ctty = _hurd_init_dtable[i];
	}
      else
	/* No ctty magic happening here.  */
	ctty = MACH_PORT_NULL;

      _hurd_port_init (&d->ctty, ctty);
    }

  /* Initialize the remaining empty slots in the table.  */
  for (; i < _hurd_dtable.size; ++i)
    {
      _hurd_port_init (&_hurd_dtable.d[i].port, MACH_PORT_NULL);
      _hurd_port_init (&_hurd_dtable.d[i].ctty, MACH_PORT_NULL);
      _hurd_dtable.d[i].flags = 0;
    }

  /* Clear out the initial descriptor table.
     Everything must use _hurd_dtable now.  */
  __vm_deallocate (__mach_task_self (),
		   _hurd_init_dtable,
		   _hurd_init_dtablesize * sizeof (_hurd_init_dtable[0]));
  _hurd_init_dtable = NULL;
  _hurd_init_dtablesize = 0;
}

text_set_element (__libc_subinit, init_dtable);

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
      io_t port = _HURD_PORT_USE (&_hurd_dtable.d[i].port, &dealloc);
      io_t ctty = _HURD_PORT_USE (&_hurd_dtable.d[i].ctty, &dealloc_ctty);

      if (port != MACH_PORT_NULL)
	err = __mach_port_insert_right (newtask, port, port,
					MACH_PORT_COPY_SEND);
      if (!err && ctty != MACH_PORT_NULL)
	err = __mach_port_insert_right (newtask, ctty, ctty,
					MACH_PORT_COPY_SEND);

      _hurd_port_free (port, &dealloc);
      _hurd_port_free (ctty, &dealloc_ctty);

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
  int d;

  __mutex_lock (&_hurd_dtable_lock);

  for (d = 0; d < _hurd_dtable.size; ++d)
    {
      struct _hurd_fd *const d = &hurd_dtable.d[d];
      mach_port_t new, newctty;
      
      /* Take the descriptor cell's lock.  */
      __spin_lock (&cell->port.lock);
      
      /* Reauthenticate the descriptor's port.  */
      if (cell->port.port != MACH_PORT_NULL &&
	  ! __io_reauthenticate (cell->port.port) &&
	  ! _HURD_PORT_USE (&_hurd_auth,
			    __auth_user_authenticate (port,
						      cell->port.port, &new)))
	{
	  /* Replace the port in the descriptor cell
	     with the newly reauthenticated port.  */

	  if (cell->ctty.port != MACH_PORT_NULL &&
	      ! __io_reauthenticate (cell->ctty.port) &&
	      ! _HURD_PORT_USE (&_hurd_auth,
				__auth_user_authenticate (port,
							  cell->ctty.port,
							  &newctty)))
	    _hurd_port_set (&cell->ctty, newctty);

	  _hurd_port_locked_set (&cell->port, new);
	}
      else
	/* Lost.  Leave this descriptor cell alone.  */
	__spin_unlock (&cell->port.lock);
    }

  __mutex_unlock (&_hurd_dtable_lock);
}

text_set_element (_hurd_reauth_hook, reauth_dtable);
