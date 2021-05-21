/* Copyright (C) 2002-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include "pthreadP.h"
#include <atomic.h>
#include <sysdep.h>
#include <unistd.h>
#include <unwind-link.h>
#include <stdio.h>
#include <gnu/lib-names.h>
#include <sys/single_threaded.h>

/* For asynchronous cancellation we use a signal.  This is the core
   logic of the signal handler.  */
static void
sigcancel_handler (void)
{
  struct pthread *self = THREAD_SELF;

  int oldval = THREAD_GETMEM (self, cancelhandling);
  while (1)
    {
      /* We are canceled now.  When canceled by another thread this flag
	 is already set but if the signal is directly send (internally or
	 from another process) is has to be done here.  */
      int newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      if (oldval == newval || (oldval & EXITING_BITMASK) != 0)
	/* Already canceled or exiting.  */
	break;

      int curval = THREAD_ATOMIC_CMPXCHG_VAL (self, cancelhandling, newval,
					      oldval);
      if (curval == oldval)
	{
	  /* Set the return value.  */
	  THREAD_SETMEM (self, result, PTHREAD_CANCELED);

	  /* Make sure asynchronous cancellation is still enabled.  */
	  if ((newval & CANCELTYPE_BITMASK) != 0)
	    /* Run the registered destructors and terminate the thread.  */
	    __do_cancel ();

	  break;
	}

      oldval = curval;
    }
}

/* This is the actually installed SIGCANCEL handler.  It adds some
   safety checks before performing the cancellation.  */
void
__nptl_sigcancel_handler (int sig, siginfo_t *si, void *ctx)
{
  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (sig != SIGCANCEL
      || si->si_pid != __getpid()
      || si->si_code != SI_TKILL)
    return;

  sigcancel_handler ();
}
libc_hidden_def (__nptl_sigcancel_handler)

int
__pthread_cancel (pthread_t th)
{
  volatile struct pthread *pd = (volatile struct pthread *) th;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

#ifdef SHARED
  /* Trigger an error if libgcc_s cannot be loaded.  */
  {
    struct unwind_link *unwind_link = __libc_unwind_link_get ();
    if (unwind_link == NULL)
      __libc_fatal (LIBGCC_S_SO
		    " must be installed for pthread_cancel to work\n");
  }
#endif
  int result = 0;
  int oldval;
  int newval;
  do
    {
    again:
      oldval = pd->cancelhandling;
      newval = oldval | CANCELING_BITMASK | CANCELED_BITMASK;

      /* Avoid doing unnecessary work.  The atomic operation can
	 potentially be expensive if the bug has to be locked and
	 remote cache lines have to be invalidated.  */
      if (oldval == newval)
	break;

      /* If the cancellation is handled asynchronously just send a
	 signal.  We avoid this if possible since it's more
	 expensive.  */
      if (CANCEL_ENABLED_AND_CANCELED_AND_ASYNCHRONOUS (newval))
	{
	  /* Mark the cancellation as "in progress".  */
	  if (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling,
						    oldval | CANCELING_BITMASK,
						    oldval))
	    goto again;

	  if (pd == THREAD_SELF)
	    /* This is not merely an optimization: An application may
	       call pthread_cancel (pthread_self ()) without calling
	       pthread_create, so the signal handler may not have been
	       set up for a self-cancel.  */
	    sigcancel_handler ();
	  else
	    {
	      /* The cancellation handler will take care of marking the
		 thread as canceled.  */
	      pid_t pid = __getpid ();

	      int val = INTERNAL_SYSCALL_CALL (tgkill, pid, pd->tid,
					       SIGCANCEL);
	      if (INTERNAL_SYSCALL_ERROR_P (val))
		result = INTERNAL_SYSCALL_ERRNO (val);
	    }

	  break;
	}

	/* A single-threaded process should be able to kill itself, since
	   there is nothing in the POSIX specification that says that it
	   cannot.  So we set multiple_threads to true so that cancellation
	   points get executed.  */
	THREAD_SETMEM (THREAD_SELF, header.multiple_threads, 1);
#ifndef TLS_MULTIPLE_THREADS_IN_TCB
	__libc_multiple_threads = 1;
#endif
    }
  /* Mark the thread as canceled.  This has to be done
     atomically since other bits could be modified as well.  */
  while (atomic_compare_and_exchange_bool_acq (&pd->cancelhandling, newval,
					       oldval));

  return result;
}
versioned_symbol (libc, __pthread_cancel, pthread_cancel, GLIBC_2_34);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_34)
compat_symbol (libpthread, __pthread_cancel, pthread_cancel, GLIBC_2_0);
#endif

/* Ensure that the unwinder is always linked in (the __pthread_unwind
   reference from __do_cancel is weak).  Use ___pthread_unwind_next
   (three underscores) to produce a strong reference to the same
   file.  */
PTHREAD_STATIC_FN_REQUIRE (___pthread_unwind_next)
