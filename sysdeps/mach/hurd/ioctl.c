/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <errno.h>
#include <sys/ioctl.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <stdarg.h>
#include <mach/notify.h>
#include <assert.h>

/* Symbol set of ioctl handler lists.  If there are user-registered
   handlers, one of these lists will contain them.  The other lists are
   handlers built into the library.  The definition of the set comes from
   hurdioctl.c.  */
extern const struct
  {
    size_t n;
    struct ioctl_handler *v[0];
  } _hurd_ioctl_handler_lists;


/* Perform the I/O control operation specified by REQUEST on FD.
   The actual type and use of ARG and the return value depend on REQUEST.  */
int
DEFUN(__ioctl, (fd, request),
      int fd AND unsigned long int request DOTS)
{
  /* Map individual type fields to Mach IPC types.  */
  static const int mach_types[] =
    { MACH_MSG_TYPE_CHAR, MACH_MSG_TYPE_INTEGER_16, MACH_MSG_TYPE_INTEGER_32,
      -1 };

  /* Extract the type information encoded in the request.  */
  unsigned int type = _IOC_TYPE (request);

  /* Message buffer.  */
  struct
    {
      mig_reply_header_t header;
      char data[_IOT_COUNT0 (type) * (_IOT_TYPE0 (request) << 1) +
		_IOT_COUNT1 (type) * (_IOT_TYPE1 (request) << 1) +
		_IOT_COUNT2 (type) * (_IOT_TYPE2 (request) << 1)];
    } msg;
  mach_msg_header_t *const m = &msg.header.Head;
  mach_msg_type_t *t = (mach_msg_type_t *) msg.data;
  mach_msg_id_t msgid;
  unsigned int reply_size;

  void *arg;

  error_t err;

#define io2mach_type(count, type) \
  ((mach_msg_type_t) { mach_types[type], (type << 1) * 8, count, 1, 0, 0 })

  /* Pack an argument into the message buffer.  */
  inline void in (unsigned int count, enum __ioctl_datum type)
    {
      if (count > 0)
	{
	  void *const p = &t[1];
	  const size_t len = count * ((unsigned int) type << 1);
	  *t = io2mach_type (count, type);
	  memcpy (p, arg, len);
	  arg += len;
	  t = p + ((len + sizeof (*t) - 1) / sizeof (*t) * sizeof (*t));
	}
    }

  /* Unpack the message buffer into the argument location.  */
  inline int out (unsigned int count, unsigned int type,
		  void *store, void **update)
    {
      if (count > 0)
	{
	  const size_t len = count * (type << 1);
	  union { mach_msg_type_t t; int i; } ipctype;
	  ipctype.t = io2mach_type (count, type);
	  if (*(int *) t != ipctype.i)
	    return 1;
	  ++t;
	  memcpy (store, t, len);
	  if (update != NULL)
	    *update += len;
	  t = (void *) t + ((len + sizeof (*t) - 1) / sizeof (*t) * sizeof *t);
	}
      return 0;
    }

  va_list ap;

  va_start (ap, request);
  arg = va_arg (ap, void *);
  va_end (ap);

  {
    /* Check for a registered handler for REQUEST.  */

    size_t i;
    const struct ioctl_handler *h;

    for (i = 0; i < _hurd_ioctl_handler_lists.n; ++i)
      for (h = _hurd_ioctl_handler_lists.v[i]; h != NULL; h = h->next)
	if (request >= h->first_request && request <= h->last_request)
	  /* This handler groks REQUEST.  Se lo puntamonos.  */
	  return (*h->handler) (fd, request, arg);
  }

  /* Compute the Mach message ID for the RPC from the group and command
     parts of the ioctl request.  */
  msgid = (100000 +
	   ((_IOC_GROUP (request) - 'f') * 4000) + _IOC_COMMAND (request));

  if (_IOC_INOUT (request) & IOC_IN)
    {
      /* Pack the argument data.  */
      in (_IOT_COUNT0 (type), _IOT_TYPE0 (type));
      in (_IOT_COUNT1 (type), _IOT_TYPE1 (type));
      in (_IOT_COUNT2 (type), _IOT_TYPE2 (type));
    }

  /* Compute the expected size of the reply.  There is a standard header
     consisting of the message header and the reply code.  Then, for out
     and in/out ioctls, there come the data with their type headers.  */
  reply_size = sizeof (mig_reply_header_t);

  if (_IOC_INOUT (request) & IOC_OUT)
    {
      inline void figure_reply (unsigned int count, enum __ioctl_datum type)
	{
	  if (count > 0)
	    {
	      /* Add the size of the type and data.  */
	      reply_size += sizeof (mach_msg_type_t) + (type << 1) * count;
	      /* Align it to word size.  */
	      reply_size += sizeof (mach_msg_type_t) - 1;
	      reply_size &= ~(sizeof (mach_msg_type_t) - 1);
	    }
	}
      figure_reply (_IOT_COUNT0 (request), _IOT_TYPE0 (request));
      figure_reply (_IOT_COUNT1 (request), _IOT_TYPE1 (request));
      figure_reply (_IOT_COUNT2 (request), _IOT_TYPE2 (request));
    }

  /* Send the RPC.  */

  err = HURD_DPORT_USE
    (fd,
     ({
       m->msgh_size = (char *) t - (char *) &msg;
       m->msgh_remote_port = port;
       m->msgh_local_port = __mig_get_reply_port ();
       m->msgh_seqno = 0;
       m->msgh_id = msgid;
       m->msgh_bits = MACH_MSGH_BITS (MACH_MSG_TYPE_COPY_SEND,
				      MACH_MSG_TYPE_MAKE_SEND_ONCE);
#ifdef notyet
       HURD_EINTR_RPC (port, __mach_msg (m, MACH_SEND_MSG|MACH_RCV_MSG,
					 m->msgh_size, sizeof (msg),
					 m->msgh_local_port,
					 MACH_MSG_TIMEOUT_NONE,
					 MACH_PORT_NULL));
#else
       __mach_msg (m, MACH_SEND_MSG|MACH_RCV_MSG, m->msgh_size, sizeof (msg),
		   m->msgh_local_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
#endif       
     }));

  switch (err)
    {
    case MACH_MSG_SUCCESS:
      break;
    case MACH_SEND_INVALID_REPLY:
    case MACH_RCV_INVALID_NAME:
      __mig_dealloc_reply_port ();
    default:
      return __hurd_fail (err);
    }

  if ((m->msgh_bits & MACH_MSGH_BITS_COMPLEX))
    {
      /* Allow no ports or VM.  */
      __mach_msg_destroy (m);
      /* Want to return a different error below for a different msgid.  */
      if (m->msgh_id == msgid + 100)
	return __hurd_fail (MIG_TYPE_ERROR);
    }

  if (m->msgh_id != msgid + 100)
    return __hurd_fail (m->msgh_id == MACH_NOTIFY_SEND_ONCE ?
			MIG_SERVER_DIED : MIG_REPLY_MISMATCH);

  if (m->msgh_size != reply_size &&
      m->msgh_size != sizeof (mig_reply_header_t))
    return __hurd_fail (MIG_TYPE_ERROR);

  if (*(int *) &msg.header.RetCodeType !=
      ((union { mach_msg_type_t t; int i; })
       { t: io2mach_type (1, _IOTS (sizeof msg.header.RetCode)) }).i)
    return __hurd_fail (MIG_TYPE_ERROR);
  err = msg.header.RetCode;

  t = (mach_msg_type_t *) msg.data;
  switch (err)
    {
    case 0:
      if (m->msgh_size != reply_size ||
	  out (_IOT_COUNT0 (type), _IOT_TYPE0 (type), arg, &arg) ||
	  out (_IOT_COUNT1 (type), _IOT_TYPE1 (type), arg, &arg) ||
	  out (_IOT_COUNT2 (type), _IOT_TYPE2 (type), arg, &arg))
	return __hurd_fail (MIG_TYPE_ERROR);
      return 0;

    case MIG_BAD_ID:
    case EOPNOTSUPP:
      /* The server didn't understand the RPC.  */
      err = ENOTTY;
    default:
      return __hurd_fail (err);
    }
}
