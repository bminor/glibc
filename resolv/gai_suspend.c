/* Copyright (C) 2001-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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
#include <netdb.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include <gai_misc.h>

int
__gai_suspend_time64 (const struct gaicb *const list[], int ent,
                      const struct __timespec64 *timeout)
{
  struct waitlist waitlist[ent];
  struct requestlist *requestlist[ent];
#ifndef DONT_NEED_GAI_MISC_COND
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
#endif
  int cnt;
  unsigned int cntr = 1;
  int none = 1;
  int result;

  /* Request the mutex.  */
  pthread_mutex_lock (&__gai_requests_mutex);

  /* There is not yet a finished request.  Signal the request that
     we are working for it.  */
  for (cnt = 0; cnt < ent; ++cnt)
    if (list[cnt] != NULL && list[cnt]->__return == EAI_INPROGRESS)
      {
	requestlist[cnt] = __gai_find_request (list[cnt]);

	if (requestlist[cnt] != NULL)
	  {
#ifndef DONT_NEED_GAI_MISC_COND
	    waitlist[cnt].cond = &cond;
#endif
	    waitlist[cnt].next = requestlist[cnt]->waiting;
	    waitlist[cnt].counterp = &cntr;
	    waitlist[cnt].sigevp = NULL;
	    waitlist[cnt].caller_pid = 0;	/* Not needed.  */
	    requestlist[cnt]->waiting = &waitlist[cnt];
	    none = 0;
	  }
      }

  struct __timespec64 ts;
  if (timeout != NULL)
    {
      __clock_gettime64 (CLOCK_MONOTONIC, &ts);
      ts.tv_sec += timeout->tv_sec;
      ts.tv_nsec += timeout->tv_nsec;
      if (ts.tv_nsec >= 1000000000)
	{
	  ts.tv_nsec -= 1000000000;
	  ts.tv_sec++;
	}
    }

  if (none)
    {
      if (cnt < ent)
	/* There is an entry which is finished.  */
	result = 0;
      else
	result = EAI_ALLDONE;
    }
  else
    {
      /* There is no request done but some are still being worked on.  */
      int oldstate;

      /* Since `pthread_cond_wait'/`pthread_cond_timedwait' are cancelation
	 points we must be careful.  We added entries to the waiting lists
	 which we must remove.  So defer cancelation for now.  */
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &oldstate);

#ifdef DONT_NEED_GAI_MISC_COND
      result = 0;
      GAI_MISC_WAIT (result, cntr, timeout == NULL ? NULL : &ts, 1);
#else
      struct timespec ts32 = valid_timespec64_to_timespec (ts);
      result = pthread_cond_timedwait (&cond, &__gai_requests_mutex,
                                       timeout == NULL ? NULL : &ts32);
#endif

      /* Now remove the entry in the waiting list for all requests
	 which didn't terminate.  */
      for (cnt = 0; cnt < ent; ++cnt)
	if (list[cnt] != NULL && list[cnt]->__return == EAI_INPROGRESS
	    && requestlist[cnt] != NULL)
	  {
	    struct waitlist **listp = &requestlist[cnt]->waiting;

	    /* There is the chance that we cannot find our entry anymore.
	       This could happen if the request terminated and restarted
	       again.  */
	    while (*listp != NULL && *listp != &waitlist[cnt])
	      listp = &(*listp)->next;

	    if (*listp != NULL)
	      *listp = (*listp)->next;
	  }

      /* Now it's time to restore the cancelation state.  */
      pthread_setcancelstate (oldstate, NULL);

#ifndef DONT_NEED_GAI_MISC_COND
      /* Release the conditional variable.  */
      if (pthread_cond_destroy (&cond) != 0)
	/* This must never happen.  */
	abort ();
#endif

      if (result != 0)
	{
	  /* An error occurred.  Possibly it's EINTR.  We have to translate
	     the timeout error report of `pthread_cond_timedwait' to the
	     form expected from `gai_suspend'.  */
	  if (__glibc_likely (result == ETIMEDOUT))
	    result = EAI_AGAIN;
	  else if (result == EINTR)
	    result = EAI_INTR;
	  else
	    result = EAI_SYSTEM;
	}
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__gai_requests_mutex);

  return result;
}

#if __TIMESIZE != 64
libanl_hidden_def (__gai_suspend_time64)

int
__gai_suspend (const struct gaicb *const list[], int ent,
	       const struct timespec *timeout)
{
  struct __timespec64 ts64;

  if (timeout != NULL)
    ts64 = valid_timespec_to_timespec64 (*timeout);

  return __gai_suspend_time64 (list, ent, timeout != NULL ? &ts64 : NULL);
}
#endif
weak_alias (__gai_suspend, gai_suspend)
