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
_hurd_fd_write (struct hurd_fd *fd, const void *buf, size_t *nbytes)
{
  error_t err;
  mach_msg_type_number_t wrote;
  int noctty;

#ifdef notyet
  struct _hurd_sigstate *ss = _hurd_self_sigstate ();

  /* Don't use the ctty io port if we are orphaned, or are blocking or
     ignoring SIGTTOU.  */
  noctty = (_hurd_orphaned ||
	    __sigismember (SIGTTOU, &ss->blocked) ||
	    ss->actions[SIGTTOU].sa_handler == SIG_IGN);
  __mutex_unlock (&ss->lock);
#else
  noctty = 1;
#endif
  
  err = HURD_FD_PORT_USE
    (fd,
     ({
     call:
       err = __io_write (noctty ? port : ctty, buf, *nbytes, -1, &wrote);
       if (!noctty && ctty != MACH_PORT_NULL && err == EBACKGROUND)
	 {
#if 1
	   abort ();
#else
	   int restart;
	   __mutex_lock (&ss->lock);
	   restart = ss->actions[SIGTTOU].sa_flags & SA_RESTART;
	   _hurd_raise_signal (ss, SIGTTOU, 0); /* Unlocks SS->lock.  */
	   if (restart)
	     goto call;
	   else
	     err = EINTR;	/* XXX Is this right? */
#endif
	 }
       err;
     }));

  if (! err)
    *nbytes = wrote;

  return err;
}
