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
#include <gnu-stabs.h>

static spin_lock_t select_port_lock;
static mach_port_t select_port;

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
      __spin_lock (&select_port_lock);
      if (select_port != MACH_PORT_NULL)
	{
	  port = select_port;
	  select_port = MACH_PORT_NULL;
	  __spin_unlock (&select_port_lock);
	}
      else
	{
	  __spin_unlock (&select_port_lock);
	  __mach_port_allocate (__mach_task_self (),
				MACH_PORT_RIGHT_RECEIVE, &port);
	}
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
      if (type)
	{
	  int result;
	  error_t err = __io_select (_hurd_dtable.d[i], type, port, i,
				     &result);
	  if (result & (SELECT_READ|SELECT_WRITE|SELECT_URG))
	    {
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
	      ++got;
	    }
	}
    }

  __mutex_unlock (&_hurd_dtable.lock);

  if (got == 0 && port != MACH_PORT_NULL)
    {
      struct
	{
	  mach_msg_header_t head;
	  mach_msg_type_t result_type;
	  int result;
	  mach_msg_type_t tag_type;
	  int tag;
	} msg;
      mach_msg_timeout_t timeout = (timeval != NULL ?
				    timeval->tv_sec * 1000 +
				    timeval->tv_usec :
				    0);
      mach_msg_option_t options = (timeval == NULL ? 0 : MACH_RCV_TIMEOUT);
    receive:
      switch (__mach_msg (&msg, MACH_RCV_MSG | options, 0, sizeof (msg),
			  port, timeout, MACH_PORT_NULL))
	{
	case MACH_MSG_SUCCESS:
	  {
	    const mach_msg_type_t inttype =
	      { MACH_MSG_TYPE_INTEGER_32, 32, 1, 1, 0, 0 };
	    if (msg.head.msgh_id == SELECT_DONE_MSGID &&
		!(msg.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
		*(int *) &msg.result_type == *(int *) &inttype &&
		*(int *) &msg.tag_type == *(int *) &inttype &&
		(msg.result & (SELECT_READ|SELECT_WRITE|SELECT_URG)) &&
		msg.tag >= 0 && msg.tag < nfds)
	      {
		if (readfds != NULL && (msg.result & SELECT_READ))
		  FD_SET (msg.tag, readfds);
		else
		  FD_CLR (msg.tag, writefds);
		if (writefds != NULL && (msg.result & SELECT_WRITE))
		  FD_SET (msg.tag, writefds);
		else
		  FD_CLR (msg.tag, writefds);
		if (exceptfds != NULL && (msg.result & SELECT_URG))
		  FD_SET (msg.tag, exceptfds);
		else
		  FD_CLR (msg.tag, exceptfds);
		++got;
		if (msg.msgh_remote_port != MACH_PORT_NULL)
		  {
		    msg.head.msgh_id += 100;
		    msg.result_type = inttype;
		    msg.result = POSIX_SUCCESS;
		    options = MACH_SEND_MSG | MACH_RCV_TIMEOUT;
		  }
	      }
	    else
	      {
		/* Randomness.  */
		__mach_msg_destroy (msg);
		options = MACH_RCV_TIMEOUT;
	      }

	    /* Poll for another message.  */
	    timeout = 0;
	    goto receive;
	  }

	case MACH_RCV_TIMED_OUT:
	  break;

	default:
	  errno = EIO;		/* XXX ??? */
	  got = -1;
	  break;
	}
    }

  if (port != MACH_PORT_NULL)
    {
      __spin_lock (&select_port_lock);
      if (select_port == MACH_PORT_NULL)
	{
	  select_port = port;
	  __spin_unlock (&select_port_lock);
	}
      else
	{
	  __spin_unlock (&select_port_lock);
	  __mach_port_destroy (port);
	}
    }

  return got;
}


static void
init_select_port_lock (void)
{
  __spin_lock_init (select_port_lock);
}

text_set_element (__libc_subinit, init_select_port_lock);
