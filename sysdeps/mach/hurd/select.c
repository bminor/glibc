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
#include <sys/types.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdlib.h>
#include <gnu-stabs.h>

#define SELECT_DONE_MSGID 23020	/* XXX */

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
  int *types;
  struct hurd_userlink *ulink;
  mach_port_t *ports;
  struct hurd_fd **cells;
  error_t err;


  /* If we're going to bomb, do it before we acquire any locks.  */
  if (readfds != NULL)
    *(volatile fd_set *) readfds = *readfds;
  if (writefds != NULL)
    *(volatile fd_set *) writefds = *writefds;
  if (exceptfds != NULL)
    *(volatile fd_set *) exceptfds = *exceptfds;

  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_dtable_lock);

  if (nfds > _hurd_dtablesize)
    nfds = _hurd_dtablesize;

  /* Collect the ports for interesting FDs.  */
  cells = __alloca (nfds * sizeof (*cells));
  ports = __alloca (nfds * sizeof (*ports));
  types = __alloca (nfds * sizeof (*types));
  ulink = __alloca (nfds * sizeof (*ulink));
  for (i = 0; i < nfds; ++i)
    {
      int type = 0;
      if (readfds != NULL && FD_ISSET (i, readfds))
	type |= SELECT_READ;
      if (writefds != NULL && FD_ISSET (i, writefds))
	type |= SELECT_WRITE;
      if (exceptfds != NULL && FD_ISSET (i, exceptfds))
	type |= SELECT_URG;
      types[i] = type;
      if (type)
	{
	  cells[i] = _hurd_dtable[i];
	  ports[i] = _hurd_port_get (&cells[i]->port, &ulink[i]);
	  if (ports[i] == MACH_PORT_NULL)
	    {
	      /* If one descriptor is bogus, we fail completely.  */
	      while (i-- > 0)
		_hurd_port_free (&cells[i]->port, &ulink[i], ports[i]);
	      errno = EBADF;
	      break;
	    }
	}
    }

  __mutex_unlock (&_hurd_dtable_lock);
  HURD_CRITICAL_END;

  if (i < nfds)
    return -1;

  if (timeout != NULL && timeout->tv_sec == 0 && timeout->tv_usec == 0)
    /* We just want to poll, so we don't need a receive right.  */
    port = MACH_PORT_NULL;
  else
    /* Get a port to receive the io_select_done message on.  */
    port = __mach_reply_port ();

  /* Send them all io_select calls.  */
  got = 0;
  err = 0;
  for (i = 0; i < nfds; ++i)
    if (types[i])
      {
	if (!err)
	  {
	    err = __io_select (ports[i], types[i],
			       port, MACH_MSG_TYPE_MAKE_SEND_ONCE,
			       i, &types[i]);
	    if (types[i])
	      ++got;
	  }
	_hurd_port_free (&cells[i]->port, &ulink[i], ports[i]);
      }

  /* If none were ready immediately, wait for a callback.  */
  if (!err && got == 0 && port != MACH_PORT_NULL)
    {
      /* Now wait for select_done messages on PORT,
	 timing out as appropriate.  */

      union
	{
	  mach_msg_header_t head;
	  struct
	    {
	      mach_msg_header_t head;
	      mach_msg_type_t result_type;
	      int result;
	      mach_msg_type_t tag_type;
	      int tag;
	    } request;
	  struct
	    {
	      mach_msg_header_t head;
	      mach_msg_type_t err_type;
	      error_t err;
	    } reply;
	} msg;
      const mach_msg_timeout_t to = (timeout != NULL ?
				     (timeout->tv_sec * 1000 +
				      timeout->tv_usec / 1000) :
				     0);
      mach_msg_option_t options = (timeout == NULL ? 0 : MACH_RCV_TIMEOUT);
      while ((err = __mach_msg (&msg.head,
				MACH_RCV_MSG | options,
				sizeof (msg.reply), sizeof (msg.request),
				port, to, MACH_PORT_NULL)) == MACH_MSG_SUCCESS)
	{
	  /* We got a message.  Decode it.  */
	  const mach_msg_type_t inttype =
	    { MACH_MSG_TYPE_INTEGER_32, 32, 1, 1, 0, 0 };
	  if (msg.head.msgh_id == SELECT_DONE_MSGID &&
	      !(msg.head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
	      *(int *) &msg.request.result_type == *(int *) &inttype &&
	      *(int *) &msg.request.tag_type == *(int *) &inttype &&
	      (msg.request.result & (SELECT_READ|SELECT_WRITE|SELECT_URG)) &&
	      msg.request.tag >= 0 && msg.request.tag < nfds)
	    {
	      /* This is a winning io_select_done message!
		 Record the readiness it indicates and send a reply.  */

	      if (types[msg.request.tag] == 0)
		/* This descriptor is ready and it was not before,
		   so we increment our count of ready descriptors.  */
		++got;
	      types[msg.request.tag] |= msg.request.result;
	      if (msg.head.msgh_remote_port != MACH_PORT_NULL)
		{
		  msg.head.msgh_id += 100;
		  msg.reply.err_type = inttype;
		  msg.reply.err = 0;
		  /* When we loop below, send this reply message in the
		     same operation that polls for another io_select_done
		     request message.  */
		  options |= MACH_SEND_MSG;
		  /* The reply message has no reply port of its own.  */
		  msg.head.msgh_bits &= ~MACH_MSGH_BITS_LOCAL_MASK;
		  msg.head.msgh_local_port = MACH_PORT_NULL;
		}
	    }
	  else
	    {
	      /* Randomness.  */
	      __mach_msg_destroy (msg);
	      continue;
	    }

	  /* Poll for another message.  */
	  timeout = 0;
	  options |= MACH_RCV_TIMEOUT;
	}

    if (err == MACH_RCV_TIMED_OUT)
      /* This is the normal value for ERR.  We might have timed out and
         read no messages.  Otherwise, after receiving the first message,
         we poll for more messages.  We receive with a timeout of 0 to
         effect a poll, so ERR is MACH_RCV_TIMED_OUT when the poll finds no
         message waiting.  */
      err = 0;
    }

  if (port != MACH_PORT_NULL)
    /* We must destroy the port if we made some select requests
       that might send notification on that port after we no longer care.
       If the port were reused, that notification could confuse the next
       select call to use the port.  The notification might be valid,
       but the descriptor may have changed to a different server.  */
    __mach_port_destroy (__mach_task_self (), port);

  if (err)
    return __hurd_fail (err);

  /* Set the user bitarrays.  */
  for (i = 0; i < nfds; ++i)
    {
      if (readfds != NULL)
	if (types[i] & SELECT_READ)
	  FD_SET (i, readfds);
	else
	  FD_CLR (i, readfds);
      if (writefds != NULL)
	if (types[i] & SELECT_WRITE)
	  FD_SET (i, writefds);
	else
	  FD_CLR (i, writefds);
      if (exceptfds != NULL)
	if (types[i] & SELECT_URG)
	  FD_SET (i, exceptfds);
	else
	  FD_CLR (i, exceptfds);
    }

  return got;
}
