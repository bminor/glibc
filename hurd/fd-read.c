/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/fd.h>

error_t
_hurd_fd_read (struct hurd_fd *fd, void *buf, size_t *nbytes)
{
  error_t err;
  char *data;
  mach_msg_type_size_t nread;

  data = buf;
  err = HURD_FD_PORT_USE
    (fd,
     ({
       do
	 {
	   err = __io_read (ctty != MACH_PORT_NULL ? ctty : port,
			    &data, &nread, -1, *nbytes);
	   if (ctty != MACH_PORT_NULL && err == EBACKGROUND)
	     {
	       /* We are a background job and tried to read from the tty.
		  We should probably get a SIGTTIN signal.  */
	       struct hurd_sigstate *ss;
	       if (_hurd_orphaned)
		 /* Our process group is orphaned.  Don't stop; just fail.  */
		 err = EIO;
	       else
		 {
		   ss = _hurd_self_sigstate ();
		   if (__sigismember (&ss->blocked, SIGTTIN) ||
		       ss->actions[SIGTTIN].sa_handler == SIG_IGN)
		     /* We are blocking or ignoring SIGTTIN.  Just fail.  */
		     err = EIO;
		   __mutex_unlock (&ss->lock);
		 }
	       if (err == EBACKGROUND)
		 {
		   /* Send a SIGTTIN signal to our process group.  */
		   err = __USEPORT (CTTYID, _hurd_sig_post (0, SIGTTIN, port));
		   /* XXX what to do if error here? */
		   /* At this point we should have just run the handler for
		      SIGTTIN or resumed after being stopped.  Now this is
		      still a "system call", so check to see if we should
		      restart it.  */
		   __mutex_lock (&ss->lock);
		   if (!(ss->actions[SIGTTIN].sa_flags & SA_RESTART))
		     err = EINTR;
		   __mutex_unlock (&ss->lock);
		 }
	     }
	 } while (err == EBACKGROUND);
       err;
     }));

  if (err)
    return err;

  if (data != buf)
    {
      memcpy (buf, data, nread);
      __vm_deallocate (__mach_task_self (), (vm_address_t) data, nread);
    }

  *nbytes = nread;
  return 0;
}
