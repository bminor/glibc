/* Copyright (C) 1994 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <errno.h>
#include <sys/time.h>
#include <hurd.h>
#include <hurd/msg_request.h>

/* XXX Temporary cheezoid implementation of ITIMER_REAL/SIGALRM.  */

struct itimerval _hurd_itimerval; /* Current state of the timer.  */
mach_port_t _hurd_itimer_port;	/* Port the timer thread blocks on.  */
thread_t _hurd_itimer_thread;	/* Thread waiting for timeout.  */
struct timeval _hurd_itimer_started; /* Time the thread started waiting.  */

/* Set the timer WHICH to *NEW.  If OLD is not NULL,
   set *OLD to the old value of timer WHICH.
   Returns 0 on success, -1 on errors.  */
int
DEFUN(__setitimer, (which, new, old),
      enum __itimer_which which AND
      struct itimerval *new AND struct itimerval *old)
{
  switch (which)
    {
    default:
      return __hurd_fail (EINVAL);

    case ITIMER_VIRTUAL:
    case ITIMER_PROF:
      return __hurd_fail (ENOSYS);

    case ITIMER_REAL:
      break;
    }

  if ((new->it_value.tv_sec | new->it_value.tv_usec) == 0)
    {
      /* Disable the itimer.  */
      if (_hurd_itimer_thread)
	{
	  __thread_terminate (_hurd_itimer_thread);
	  _hurd_itimer_thread = MACH_PORT_NULL;
	}
      _hurd_itimerval = *new;
      return 0;
    }
}

/* Function run by the timer thread.  This code must be very careful not to
   ever require a MiG reply port.  */

static void
timer_thread (void)
{
  while (1)
    {
      error_t err;

      err = __mach_msg (NULL, MACH_RCV_MSG | MACH_RCV_TIMEOUT,
			0, 0, _hurd_itimer_port,
			_hurd_itimerval.it_value.tv_sec * 1000 +
			_hurd_itimerval.it_value.tv_usec / 1000,
			MACH_PORT_NULL);
      if (err == MACH_RCV_TIMED_OUT)
	/* We got the expected timeout.  Send a message to the signal
	   thread to tell it to post a SIGALRM signal.  We use
	   _hurd_itimer_port as the reply port just so we will block until
	   the signal thread has frobnicated things to reload the itimer or
	   has terminated this thread.  */
	__sig_post_request (_hurd_msgport,
			    _hurd_itimer_port, MACH_MSG_TYPE_MAKE_SEND_ONCE,
			    SIGALRM, __mach_task_self ());
    }
}
