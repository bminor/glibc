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
#include <errno.h>
#include <signal.h>


/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
int
DEFUN(sigsuspend, (set), CONST sigset_t *set)
{
  struct _hurd_sigstate *ss;
  sigset_t omask, pending;
  int sig;

  if (set != NULL)
    /* Crash before locking.  */
    *(volatile const sigset_t *) set;

  ss = _hurd_thread_sigstate (__mach_thread_self ());
  omask = ss->blocked;
  if (set != NULL)
    ss->blocked = *set & ~_SIG_CANT_BLOCK;

  ss->suspended = 1;
  while ((pending = ss->pending & ~ss->blocked) == 0)
    __condition_wait (&ss->arrived, &ss->lock);
  ss->suspended = 0;

  for (sig = 1; sig < NSIG; ++sig)
    if (pending & __sigmask (sig))
      {
	/* Post the first pending signal.
	   This call will handle any more pending signals,
	   and it will release SS->lock before returning.  */
	(void) _hurd_internal_post_signal (ss->suspend_reply,
					   ss,
					   signo,
					   ss->sigcodes[signo],
					   0, 0, &ignore,
					   &omask);
	/* We've been interrupted!  And a good thing, too.
	   Otherwise we'd never return.
	   That's right; this function always returns an error.  */
	errno = EINTR;
	return -1;
      }

  __libc_fatal ("sigsuspend: Woken up by nonexistent signal.\n");
}
