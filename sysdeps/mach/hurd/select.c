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

#include <ansidecl.h>
#include <sys/types.h>
#include <hurd.h>

static mach_port_t reply = MACH_PORT_NULL;

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  */
int
DEFUN(__select, (nfds, readfds, writefds, exceptfds, timeout),
      int nfds AND fd_set *readfds AND fd_set *writefds AND
      fd_set *exceptfds AND struct timeval *timeout)
{
  int i;
  mach_port_t port;
  int got;

  __mutex_lock (&_hurd_dtable.lock);

  if (nfds > _hurd_dtable.size)
    nfds = _hurd_dtable.size;

  if (timeout != NULL && timeout->tv_sec == 0 && timeval->tv_usec == 0)
    port = MACH_PORT_NULL;
  else
    {
      port = reply;
      reply = MACH_PORT_NULL;
      __mach_port_allocate (__mach_task_self (), MACH_PORT_RIGHT_RECEIVE, &port);
    }

  got = 0;
  for (i = 0; i < nfds; ++i)
    {
      int type = 0;
      if (readfds != NULL && FD_ISSET (i, readfds))
	type |= SELECT_READ;
      if (writefds != NULL && FD_ISSET (i, writefds))
	type |= SELECT_WRITE;
      if (exceptfds != NULL && FD_ISSET (i, exceptfds))
	type |= SELECT_URG;
      if (type != 0)
	{
	  int result;
	  error_t err = __io_select (_hurd_dtable.d[i], type, port, 0,
				     &result);
	  if (readfds != NULL && (result & SELECT_READ))
	    FD_SET (i, readfds);
	  else
	    FD_CLR (i, writefds);
	  if (writefds != NULL && (result & SELECT_WRITE))
	    FD_SET (i, writefds);
	  else
	    FD_CLR (i, writefds);
	  if (exceptfds != NULL && (result & SELECT_URG))
	    FD_SET (i, exceptfds);
	  else
	    FD_CLR (i, exceptfds);
	  got |= result;
	}
    }
}
