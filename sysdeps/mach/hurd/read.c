/* Copyright (C) 1993 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
DEFUN(__read, (fd, buf, nbytes),
      int fd AND PTR buf AND size_t nbytes)
{
  error_t err;
  char *data;
  mach_msg_type_size_t nread;

  data = buf;
  _HURD_DPORT_USE
    (fd,
     ({
     call:
       err = __io_read (port, &data, &nread, -1, nbytes);
       if (ctty != MACH_PORT_NULL && err == EBACKGROUND)
	 {
#if 1
	   abort ();
#else
	   struct _hurd_sigstate *ss
	     = _hurd_thread_sigstate (__mach_thread_self ());
	   if (_hurd_orphaned ||
	       __sigismember (SIGTTIN, &ss->blocked) ||
	       ss->actions[SIGTTIN].sa_handler == SIG_IGN)
	     {
	       /* We are orphaned, or are blocking or ignoring SIGTTIN.
		  Return EOF instead of stopping.  */
	       __mutex_unlock (&ss->lock);
	       nread = 0;
	       err = 0;
	     }
	   else
	     {
	       const int restart = ss->actions[SIGTTIN].sa_flags & SA_RESTART;
	       _hurd_raise_signal (ss, SIGTTIN, 0); /* Unlocks SS->lock.  */
	       if (restart)
		 goto call;
	       else
		 err = EINTR;	/* XXX Is this right? */
	     }
#endif
	 }
       0;
     }));

  if (err)
    return __hurd_dfail (fd, err);

  if (data != buf)
    {
      memcpy (buf, data, nread);
      __vm_deallocate (__mach_task_self (), (vm_address_t) data, nread);
    }

  return nread;
}
