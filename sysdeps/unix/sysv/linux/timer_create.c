/* Copyright (C) 2003-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <jmpbuf-unwind.h>
#include <kernel-posix-cpu-timers.h>
#include <kernel-posix-timers.h>
#include <ldsodefs.h>
#include <libc-internal.h>
#include <libc-lock.h>
#include <pthreadP.h>
#include <shlib-compat.h>

struct timer_helper_thread_args_t
{
  /* The barrier is used to synchronize the arguments copy from timer_create
     and the SIGEV_THREAD thread and to instruct the thread to exit if the
     timer_create syscall fails.  */
  pthread_barrier_t b;
  struct sigevent *evp;
};

struct cleanup_args_t
{
  struct pthread_unwind_buf *cleanup_jmp_buf;
  jmp_buf jb;
};

/* Reset internal thread state if the callback issues pthread_exit.  It avoids
   recreating the thread and having possible unreported missed events due
   thread creation failure.  */
static void
timer_helper_thread_cleanup (void *arg)
{
  struct pthread *self = THREAD_SELF;

  /* Call destructors for the thread_local TLS variables.  */
  call_function_static_weak (__call_tls_dtors);

  /* Run the destructor for the thread-local data.  */
  __nptl_deallocate_tsd ();

  /* Clean up any state libc stored in thread-local variables.  */
  __libc_thread_freeres ();

  /* Reset internal TCB state.  */
  struct cleanup_args_t *args = arg;
  self->cleanup_jmp_buf = args->cleanup_jmp_buf;
  self->cleanup_jmp_buf->priv.data.prev = NULL;
  self->cleanup_jmp_buf->priv.data.cleanup = NULL;
  self->cleanup_jmp_buf->priv.data.canceltype = 0;
  self->cleanup = NULL;
  self->exc = (struct _Unwind_Exception) { 0 };
  self->cancelhandling = 0;
  self->nextevent = NULL;

  /* Re-initialize the TLS.  */
  _dl_allocate_tls_init (TLS_TPADJ (self), true);

  /* Reset to the expected initial signal mask.  */
  internal_sigset_t ss;
  internal_sigfillset (&ss);
  internal_sigdelset (&ss, SIGSETXID);
  internal_sigprocmask (SIG_SETMASK, &ss, NULL);

  /* There is no need to perform any additional cleanup by the frames.  */
  struct __jmp_buf_tag *env = args->jb;
  __longjmp (env[0].__jmpbuf, 1);
}

static void *
timer_helper_thread (void *arg)
{
  struct pthread *self = THREAD_SELF;
  struct timer_helper_thread_args_t *args = arg;
  struct cleanup_args_t clargs = {
    .cleanup_jmp_buf = self->cleanup_jmp_buf
  };

  void (*thrfunc) (sigval_t) = args->evp->sigev_notify_function;
  sigval_t sival = args->evp->sigev_value;

  __pthread_barrier_wait (&args->b);
  /* timer_create syscall failed.  */
  if (self->exiting)
    return 0;

  while (1)
    {
      siginfo_t si;
      while (__sigwaitinfo (&sigtimer_set, &si) < 0);

      if (si.si_code == SI_TIMER && !setjmp (clargs.jb))
	{
	  pthread_cleanup_push (timer_helper_thread_cleanup, &clargs);
	  thrfunc (sival);
	  pthread_cleanup_pop (0);
	}

      /* timer_delete will set the MSB and signal the thread.  */
      if (self->timerid < 0)
	break;
    }

  /* Clear the MSB bit set by timer_delete.  */
  INTERNAL_SYSCALL_CALL (timer_delete, timerid_clear (self->timerid));

  return NULL;
}

static int
timer_create_sigev_thread (clockid_t clockid, struct sigevent *evp,
			   timer_t *timerid, pthread_attr_t *attr)
{
  /* Block all signals in the helper thread but SIGSETXID.  */
  sigset_t ss;
  __sigfillset (&ss);
  __sigdelset (&ss, SIGSETXID);
  if (__pthread_attr_setsigmask_internal (attr, &ss) < 0)
    return -1;

  struct timer_helper_thread_args_t args;
  __pthread_barrier_init (&args.b, NULL, 2);
  args.evp = evp;

  pthread_t th;
  int r = __pthread_create (&th, attr, timer_helper_thread, &args);
  if (r != 0)
    {
      __set_errno (r);
      return -1;
    }

  struct pthread *pthr = (struct pthread *)th;
  struct sigevent kevp =
    {
      .sigev_value.sival_ptr = NULL,
      .sigev_signo = SIGTIMER,
      .sigev_notify = SIGEV_THREAD_ID,
      ._sigev_un = { ._tid = pthr->tid },
    };

  kernel_timer_t ktimerid;
  if (INLINE_SYSCALL_CALL (timer_create, clockid, &kevp, &ktimerid) < 0)
    {
      ktimerid = -1;
      /* On timer creation failure we need to signal the helper thread to
	 exit and we can not use the an negative timerid value after the
	 ptherad_barrier_wait because we can not distinguish between
	 a timer creation failure and request to delete a timer if it happens
	 to arrive quickly (for where two timers are create in sequence,
	 where first succeeds).

	 We re-use the 'exiting' member to signal the failure, it is set only
	 at pthread_create to avoid pthread_kill to send further signals.
	 Since the thread should not be user-visible, signal are only sent
	 during timer_delete.  */
      pthr->exiting = true;
    }
  pthr->timerid = ktimerid;
  /* Signal the thread to continue execution after it copies the arguments
     or exit if the timer can not be created.  */
  __pthread_barrier_wait (&args.b);

  if (ktimerid < 0)
    return -1;

  *timerid = pthread_to_timerid (th);

  return 0;
}

int
___timer_create (clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
  clockid_t syscall_clockid = (clock_id == CLOCK_PROCESS_CPUTIME_ID
			       ? PROCESS_CLOCK
			       : clock_id == CLOCK_THREAD_CPUTIME_ID
			       ? THREAD_CLOCK
			       : clock_id);

  switch (evp != NULL ? evp->sigev_notify : SIGEV_SIGNAL)
    {
    case SIGEV_NONE:
    case SIGEV_SIGNAL:
    case SIGEV_THREAD_ID:
      {
	struct sigevent kevp;
	if (evp == NULL)
	  {
	    kevp.sigev_notify = SIGEV_SIGNAL;
	    kevp.sigev_signo = SIGALRM;
	    kevp.sigev_value.sival_ptr = NULL;
	    evp = &kevp;
	  }

	kernel_timer_t ktimerid;
	if (INLINE_SYSCALL_CALL (timer_create, syscall_clockid, evp,
				 &ktimerid) == -1)
	  return -1;

	*timerid = kernel_timer_to_timerid (ktimerid);
      }
      break;
    case SIGEV_THREAD:
      {
	pthread_attr_t attr;
	if (evp->sigev_notify_attributes != NULL)
	  __pthread_attr_copy (&attr, evp->sigev_notify_attributes);
	else
	  __pthread_attr_init (&attr);
	__pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int r = timer_create_sigev_thread (syscall_clockid, evp, timerid,
					   &attr);

	if (&attr != evp->sigev_notify_attributes)
	  __pthread_attr_destroy (&attr);

	return r;
      }
    default:
      __set_errno (EINVAL);
      return -1;
    }

  return 0;
}
versioned_symbol (libc, ___timer_create, timer_create, GLIBC_2_34);
libc_hidden_ver (___timer_create, __timer_create)

#if TIMER_T_WAS_INT_COMPAT
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_3_3, GLIBC_2_34)
compat_symbol (librt, ___timer_create, timer_create, GLIBC_2_3_3);
# endif

# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
timer_t __timer_compat_list[OLD_TIMER_MAX];

int
__timer_create_old (clockid_t clock_id, struct sigevent *evp, int *timerid)
{
  timer_t newp;

  int res = __timer_create (clock_id, evp, &newp);
  if (res == 0)
    {
      int i;
      for (i = 0; i < OLD_TIMER_MAX; ++i)
	if (__timer_compat_list[i] == NULL
	    && ! atomic_compare_and_exchange_bool_acq (&__timer_compat_list[i],
						       newp, NULL))
	  {
	    *timerid = i;
	    break;
	  }

      if (__glibc_unlikely (i == OLD_TIMER_MAX))
	{
	  /* No free slot.  */
	  __timer_delete (newp);
	  __set_errno (EINVAL);
	  res = -1;
	}
    }

  return res;
}
compat_symbol (librt, __timer_create_old, timer_create, GLIBC_2_2);
# endif /* OTHER_SHLIB_COMPAT */

#else /* !TIMER_T_WAS_INT_COMPAT */
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_34)
compat_symbol (librt, ___timer_create, timer_create, GLIBC_2_2);
# endif
#endif /* !TIMER_T_WAS_INT_COMPAT */
