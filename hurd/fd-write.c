/* _hurd_fd_write -- write to a file descriptor; handles job control et al.
Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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
_hurd_fd_write (struct hurd_fd *fd, const void *buf, size_t *nbytes)
{
  error_t err;
  mach_msg_type_number_t wrote;
  int noctty;
  struct hurd_sigstate *ss;

  /* Note that __ioctl.c implements the same SIGTTOU behavior.
     Any changes here should be done there as well.  */

  /* Don't use the ctty io port if we are blocking or ignoring SIGTTOU.  */
  ss = _hurd_self_sigstate ();
  noctty = (__sigismember (&ss->blocked, SIGTTOU) ||
	    ss->actions[SIGTTOU].sa_handler == SIG_IGN);
  __mutex_unlock (&ss->lock);

  err = HURD_FD_PORT_USE
    (fd,
     ({
       const io_t ioport = (!noctty && ctty != MACH_PORT_NULL) ? ctty : port;
       do
	 {
	   err = __io_write (ioport, buf, *nbytes, -1, &wrote);
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

  if (! err)
    *nbytes = wrote;

  return err;
}
