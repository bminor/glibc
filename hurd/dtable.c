/* Copyright (C) 1991 Free Software Foundation, Inc.
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

struct _hurd_dtable _hurd_dtable;
text_set_element (_hurd_dtable_set, _hurd_dtable);

static void
init_dtable (void)
{
  register size_t i;

  __mutex_init (&_hurd_dtable.lock);

  /* The initial size of the descriptor table is that of the passed-in
     table, rounded up to a multiple of OPEN_MAX descriptors.  */
  _hurd_dtable.size
    = (_hurd_init_dtablesize + OPEN_MAX - 1) / OPEN_MAX * OPEN_MAX;

  _hurd_dtable.d = malloc (_hurd_init_dtablesize * sizeof (*_hurd_dtable.d));
  if (_hurd_dtable.d == NULL)
    __libc_fatal ("hurd: Can't allocate descriptor table\n");

  for (i = 0; i < _hurd_init_dtablesize; ++i)
    {
      _hurd_dtable.d[i] = _hurd_init_dtable[i];
      __spin_lock_init (&_hurd_dtable.d[i].reflock);
      _hurd_dtable.d[i].refs = 0;
      __condition_init (&_hurd_dtable.d[i].free);
    }
  for (; i < _hurd_dtable.size; ++i)
    {
      _hurd_dtable.d[i].server = MACH_PORT_NULL;
      __spin_lock_init (&_hurd_dtable.d[i].reflock);
      _hurd_dtable.d[i].refs = 0;
      __condition_init (&_hurd_dtable.d[i].free);
    }

  _hurd_init_dtable = NULL;
  _hurd_init_dtablesize = 0;
}

text_set_element (__libc_subinit, init_dtable);

/* Called on fork to install the dtable in NEWTASK.  */

static error_t
fork_dtable (task_t newtask)
{
  int i;
  __mutex_lock (&_hurd_dtable.lock);
  for (i = 0; i < _hurd_dtable.size; ++i)
    if (err = __mach_port_insert_right (newtask, _hurd_dtable.d[i].server,
					_hurd_dtable.d[i].server,
					MACH_PORT_COPY_SEND))
      {
	/* XXX for each fd with a cntlmap, reauth and re-map_cntl.  */
	__mutex_unlock (&_hurd_dtable.lock);
	return 1;
      }
  __mutex_unlock (&_hurd_dtable.lock);
  return 0;
}

text_set_element (_hurd_fork_hook, fork_dtable);
