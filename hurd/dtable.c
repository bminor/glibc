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

#include <hurd.h>
#include <gnu-stabs.h>
#include <stdlib.h>
#include <limits.h>


/* Initialize the file descriptor table at startup.  */

static void
init_dtable (void)
{
  register size_t i;

  __mutex_init (&_hurd_dtable_lock);

  /* The initial size of the descriptor table is that of the passed-in
     table, rounded up to a multiple of OPEN_MAX descriptors.  */
  _hurd_dtable.size
    = (_hurd_init_dtablesize + OPEN_MAX - 1) / OPEN_MAX * OPEN_MAX;

  _hurd_dtable.d = malloc (_hurd_dtable.size * sizeof (*_hurd_dtable.d));
  if (_hurd_dtable.d == NULL)
    __libc_fatal ("hurd: Can't allocate file descriptor table\n");

  for (i = 0; i < _hurd_init_dtablesize; ++i)
    {
      __typeof (_hurd_dtable.d[i]) *const d = &_hurd_dtable.d[i];
      io_statbuf_t stb;
      io_t fg_port;

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
				_hurd_sigport, &fg_port))
	{
	  /* Operations on FG_PORT return EBACKGROUND when we are not a
	     foreground user of the tty.  Operations on D->ctty never
	     return EBACKGROUND.  */
	  d->ctty = d->port.port;
	  d->port.port = fg_port;
	}
      else
	/* No ctty magic happening here.  */
	d->ctty = MACH_PORT_NULL;
    }

  /* Initialize the remaining empty slots in the table.  */
  for (; i < _hurd_dtable.size; ++i)
    {
      _hurd_port_init (&_hurd_dtable.d[i].port, MACH_PORT_NULL);
      _hurd_dtable.d[i].ctty = MACH_PORT_NULL;
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

/* Called on fork to install the dtable in NEWTASK.  */

static error_t
fork_dtable (task_t newtask)
{
  int i;
  __mutex_lock (&_hurd_dtable_lock);
  for (i = 0; i < _hurd_dtable.size; ++i)
    if (err = _HURD_PORT_USE (&_hurd_dtable.d[i],
			      __mach_port_insert_right (newtask, port, port,
							MACH_PORT_COPY_SEND)))
      {
	/* XXX for each fd with a cntlmap, reauth and re-map_cntl.  */
	__mutex_unlock (&_hurd_dtable.lock);
	return err;
      }
  __mutex_unlock (&_hurd_dtable_lock);
  return 0;
}

text_set_element (_hurd_fork_hook, fork_dtable);
