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
#include <hurd/signal.h>
#include <stdarg.h>
#include <mach/notify.h>
#include <assert.h>

/* Symbol set of ioctl handler lists.  If there are user-registered
   handlers, one of these lists will contain them.  The other lists are
   handlers built into the library.  The definition of the set comes from
   hurdioctl.c.  */
extern struct 
  {
    size_t n;
    struct ioctl_handler *v[0];
  } *const _hurd_ioctl_handlers;


#define typesize(type)	(1 << (type))


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
#define io2mach_type(count, type) \
  ((mach_msg_type_t) { mach_types[type], typesize (type) * 8, count, 1, 0, 0 })

  /* Extract the type information encoded in the request.  */
  unsigned int type = _IOC_TYPE (request);

  /* Message buffer.  */
  struct
    {
      mig_reply_header_t header;
      char data[3 * sizeof (mach_msg_type_t) +
		_IOT_COUNT0 (type) * typesize (_IOT_TYPE0 (type)) +
		_IOT_COUNT1 (type) * typesize (_IOT_TYPE1 (type)) +
		_IOT_COUNT2 (type) * typesize (_IOT_TYPE2 (type))];
    } msg;
  mach_msg_header_t *const m = &msg.header.Head;
  mach_msg_type_t *t = &msg.header.RetCodeType;
  mach_msg_id_t msgid;
  unsigned int reply_size;

  void *arg;

  error_t err;

  struct hurd_sigstate *ss;
  int noctty;

  /* Send the RPC already packed up in MSG to IOPORT
     and decode the return value.  */
  inline error_t send_rpc (io_t ioport)
    {
      error_t err;

      m->msgh_size = (char *) t - (char *) &msg;
      m->msgh_remote_port = ioport;
      m->msgh_local_port = __mig_get_reply_port ();
      m->msgh_seqno = 0;
      m->msgh_id = msgid;
      m->msgh_bits = MACH_MSGH_BITS (MACH_MSG_TYPE_COPY_SEND,
				     MACH_MSG_TYPE_MAKE_SEND_ONCE);
      err = HURD_EINTR_RPC (ioport, __mach_msg (m, MACH_SEND_MSG|MACH_RCV_MSG,
						m->msgh_size, sizeof (msg),
						m->msgh_local_port,
						MACH_MSG_TIMEOUT_NONE,
						MACH_PORT_NULL));
      switch (err)
	{
	case MACH_MSG_SUCCESS:
	  break;
	case MACH_SEND_INVALID_REPLY:
	case MACH_RCV_INVALID_NAME:
	  __mig_dealloc_reply_port ();
	default:
	  return err;
	}

      if ((m->msgh_bits & MACH_MSGH_BITS_COMPLEX))
	{
	  /* Allow no ports or VM.  */
	  __mach_msg_destroy (m);
	  /* Want to return a different error below for a different msgid.  */
	  if (m->msgh_id == msgid + 100)
	    return MIG_TYPE_ERROR;
	}

      if (m->msgh_id != msgid + 100)
	return (m->msgh_id == MACH_NOTIFY_SEND_ONCE ?
		MIG_SERVER_DIED : MIG_REPLY_MISMATCH);

      if (m->msgh_size != reply_size &&
	  m->msgh_size != sizeof (mig_reply_header_t))
	return MIG_TYPE_ERROR;

      if (*(int *) &msg.header.RetCodeType !=
	  ((union { mach_msg_type_t t; int i; })
	   { t: io2mach_type (1, _IOTS (sizeof msg.header.RetCode)) }).i)
	return MIG_TYPE_ERROR;
      return msg.header.RetCode;
    }

  va_list ap;

  va_start (ap, request);
  arg = va_arg (ap, void *);
  va_end (ap);

  {
    /* Check for a registered handler for REQUEST.  */

    size_t i;
    const struct ioctl_handler *h;

    for (i = 0; i < _hurd_ioctl_handlers->n; ++i)
      for (h = _hurd_ioctl_handlers->v[i]; h != NULL; h = h->next)
	if (request >= h->first_request && request <= h->last_request)
	  /* This handler groks REQUEST.  Se lo puntamonos.  */
	  return (*h->handler) (fd, request, arg);
  }

  /* Compute the Mach message ID for the RPC from the group and command
     parts of the ioctl request.  */
  msgid = 100000 + ((_IOC_GROUP (request) - 'f') * 4000); /* Base subsystem */
  /* Because of MiG's poorly chosen algorithm of adding 100 to a request
     msgid to produce the reply msgid, we cannot just add the command part
     of the ioctl request to the subsystem base msgid.  For ioctl requests
     past 99, we must skip blocks of 100 msgids to allow for the reply
     msgids corresponding to the earlier requests.  */
  if (_IOC_COMMAND (request) >= 100)
    msgid += 100;
  if (_IOC_COMMAND (request) >= 200)
    msgid += 100;
  msgid += _IOC_COMMAND (request);

  if (_IOC_INOUT (request) & IOC_IN)
    {
      /* Pack an argument into the message buffer.  */
      void in (unsigned int count, enum __ioctl_datum type)
	{
	  if (count > 0)
	    {
	      void *p = &t[1];
	      const size_t len = count * typesize ((unsigned int) type);
	      *t = io2mach_type (count, type);
	      memcpy (p, arg, len);
	      arg += len;
	      p += len;
	      p = (void *) (((unsigned long int) p + sizeof (*t) - 1)
			    & ~(sizeof (*t) - 1));
	      t = p;
	    }
	}

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
	      reply_size += sizeof (mach_msg_type_t) + typesize (type) * count;
	      /* Align it to word size.  */
	      reply_size += sizeof (mach_msg_type_t) - 1;
	      reply_size &= ~(sizeof (mach_msg_type_t) - 1);
	    }
	}
      figure_reply (_IOT_COUNT0 (type), _IOT_TYPE0 (type));
      figure_reply (_IOT_COUNT1 (type), _IOT_TYPE1 (type));
      figure_reply (_IOT_COUNT2 (type), _IOT_TYPE2 (type));
    }

  /* Note that fd-write.c implements the same SIGTTOU behavior.
     Any changes here should be done there as well.  */

  /* Don't use the ctty io port if we are blocking or ignoring SIGTTOU.  */
  ss = _hurd_self_sigstate ();
  noctty = (__sigismember (&ss->blocked, SIGTTOU) ||
	    ss->actions[SIGTTOU].sa_handler == SIG_IGN);
  __mutex_unlock (&ss->lock);

  err = HURD_DPORT_USE
    (fd,
     ({
       const io_t ioport = (!noctty && ctty != MACH_PORT_NULL) ? ctty : port;
       do
	 {
	   /* The actual hair to send the RPC is in the inline `send_rpc'
	      function (above), to avoid horrendous indentation.  */
	   err = send_rpc (ioport);
	   if (ioport == ctty && err == EBACKGROUND)
	     {
	       if (_hurd_orphaned)
		 /* Our process group is orphaned, so we never generate a
		    signal; we just fail.  */
		 err = EIO;
	       else
		 {
		   /* Send a SIGTTOU signal to our process group.  */
		   err = __USEPORT (CTTYID, _hurd_sig_post (0, SIGTTOU, port));
		   /* XXX what to do if error here? */
		   /* At this point we should have just run the handler for
		      SIGTTOU or resumed after being stopped.  Now this is
		      still a "system call", so check to see if we should
		      restart it.  */
		   __mutex_lock (&ss->lock);
		   if (!(ss->actions[SIGTTOU].sa_flags & SA_RESTART))
		     err = EINTR;
		   __mutex_unlock (&ss->lock);
		 }
	     }
	 } while (err == EBACKGROUND);
       err;
     }));

  t = (mach_msg_type_t *) msg.data;
  switch (err)
    {
      /* Unpack the message buffer into the argument location.  */
      int out (unsigned int count, unsigned int type,
	       void *store, void **update)
	{
	  if (count > 0)
	    {
	      const size_t len = count * typesize (type);
	      union { mach_msg_type_t t; int i; } ipctype;
	      ipctype.t = io2mach_type (count, type);
	      if (*(int *) t != ipctype.i)
		return 1;
	      ++t;
	      memcpy (store, t, len);
	      if (update != NULL)
		*update += len;
	      t = (void *) (((unsigned long int) t + len + sizeof (*t) - 1)
			    & ~(sizeof (*t) - 1));
	    }
	  return 0;
	}

    case 0:
      if (m->msgh_size != reply_size ||
	  ((_IOC_INOUT (request) & IOC_OUT) &&
	   (out (_IOT_COUNT0 (type), _IOT_TYPE0 (type), arg, &arg) ||
	    out (_IOT_COUNT1 (type), _IOT_TYPE1 (type), arg, &arg) ||
	    out (_IOT_COUNT2 (type), _IOT_TYPE2 (type), arg, &arg))))
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
