/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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

ssize_t
DEFUN(__write, (fd, buf, nbytes),
      int fd AND CONST PTR buf AND size_t nbytes)
{
  error_t err;
  size_t wrote;

  _HURD_DPORT_USE
    (fd,
     ({
     call:
       err = __io_write (port, buf, nbytes, -1, &wrote);
       if (ctty != MACH_PORT_NULL && err == EBACKGROUND)
	 {
#if 1
	   abort ();
#else
	   struct _hurd_sigstate *ss
	     = _hurd_thread_sigstate (__mach_thread_self ());
	   if (_hurd_orphaned ||
	       __sigismember (SIGTTOU, &ss->blocked) ||
	       ss->actions[SIGTTOU].sa_handler == SIG_IGN)
	     {
	       /* We are orphaned, or are blocking or ignoring SIGTTOU.
		  Return EIO instead of stopping.  */
	       __mutex_unlock (&ss->lock);
	       err = EIO;
	     }
	   else
	     {
	       const int restart = ss->actions[SIGTTOU].sa_flags & SA_RESTART;
	       _hurd_raise_signal (ss, SIGTTOU, 0); /* Unlocks SS->lock.  */
	       if (restart)
		 goto call;
	       else
		 err = EINTR;	/* XXX Is this right? */
	     }
#endif
	 }
     }));

  if (err)
    return __hurd_dfail (fd, err);

  return wrote;
}

