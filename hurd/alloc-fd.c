/* Copyright (C) 1994 Free Software Foundation, Inc.
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
#include <hurd/fd.h>
#include <stdlib.h>

/* Allocate a new file descriptor and return it, locked.  The new
   descriptor number will be no less than FIRST_FD.  If the table is full,
   set errno to EMFILE and return NULL.  If FIRST_FD is negative or bigger
   than the size of the table, set errno to EINVAL and return NULL.  */

struct hurd_fd *
_hurd_alloc_fd (int *fd, const int first_fd)
{
  int i;

  if (first_fd < 0)
    {
      errno = EINVAL;
      return NULL;
    }

  __mutex_lock (&_hurd_dtable_lock);

  for (i = first_fd; i < _hurd_dtable.size; ++i)
    {
      struct hurd_fd *d = _hurd_dtable.d[i];
      if (d == NULL)
	{
	  /* Allocate a new descriptor structure for this slot,
	     initializing its port cells to nil.  The test below will catch
	     and return this descriptor cell after locking it.  */
	  d = _hurd_new_fd (MACH_PORT_NULL, MACH_PORT_NULL);
	  if (d == NULL)
	    {
	      __mutex_unlock (&hurd_dtable_lock);
	      return NULL;
	    }
	}

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

  errno = first_fd < _hurd_dtable.size ? EMFILE : EINVAL;

  __mutex_unlock (&hurd_dtable_lock);

  return NULL;
}
