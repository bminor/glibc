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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <pthreadP.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>
#include <list.h>
#include <version.h>
#include <shlib-compat.h>
#include <lowlevellock.h>
#include <futex-internal.h>
#include <kernel-features.h>
#include <libc-pointer-arith.h>
#include <pthread_mutex_conf.h>

/* Size and alignment of static TLS block.  */
size_t __static_tls_size;
size_t __static_tls_align_m1;

/* Version of the library, used in libthread_db to detect mismatches.  */
static const char nptl_version[] __attribute_used__ = VERSION;

/* For asynchronous cancellation we use a signal.  This is the handler.  */
static void
sigcancel_handler (int sig, siginfo_t *si, void *ctx)
{
  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (sig != SIGCANCEL
      || si->si_pid != __getpid()
      || si->si_code != SI_TKILL)
    return;

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


/* When using __thread for this, we do it in libc so as not
   to give libpthread its own TLS segment just for this.  */
extern void **__libc_dl_error_tsd (void) __attribute__ ((const));


/* This can be set by the debugger before initialization is complete.  */
static bool __nptl_initial_report_events __attribute_used__;

void
__pthread_initialize_minimal_internal (void)
{
  /* Partial initialization of the TCB already happened in TLS_INIT_TP
     and __tls_init_tp.  */
  struct pthread *pd = THREAD_SELF;

  /* Before initializing GL (dl_stack_user), the debugger could not
     find us and had to set __nptl_initial_report_events.  Propagate
     its setting.  */
  THREAD_SETMEM (pd, report_events, __nptl_initial_report_events);

  struct sigaction sa;
  __sigemptyset (&sa.sa_mask);

  /* Install the cancellation signal handler.  If for some reason we
     cannot install the handler we do not abort.  Maybe we should, but
     it is only asynchronous cancellation which is affected.  */
  sa.sa_sigaction = sigcancel_handler;
  sa.sa_flags = SA_SIGINFO;
  (void) __libc_sigaction (SIGCANCEL, &sa, NULL);

  /* Install the handle to change the threads' uid/gid.  */
  sa.sa_sigaction = __nptl_setxid_sighandler;
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  (void) __libc_sigaction (SIGSETXID, &sa, NULL);

  /* The parent process might have left the signals blocked.  Just in
     case, unblock it.  We reuse the signal mask in the sigaction
     structure.  It is already cleared.  */
  __sigaddset (&sa.sa_mask, SIGCANCEL);
  __sigaddset (&sa.sa_mask, SIGSETXID);
  INTERNAL_SYSCALL_CALL (rt_sigprocmask, SIG_UNBLOCK, &sa.sa_mask,
			 NULL, __NSIG_BYTES);

  /* Get the size of the static and alignment requirements for the TLS
     block.  */
  size_t static_tls_align;
  _dl_get_tls_static_info (&__static_tls_size, &static_tls_align);

  /* Make sure the size takes all the alignments into account.  */
  if (STACK_ALIGN > static_tls_align)
    static_tls_align = STACK_ALIGN;
  __static_tls_align_m1 = static_tls_align - 1;

  __static_tls_size = roundup (__static_tls_size, static_tls_align);

  /* Determine the default allowed stack size.  This is the size used
     in case the user does not specify one.  */
  struct rlimit limit;
  if (__getrlimit (RLIMIT_STACK, &limit) != 0
      || limit.rlim_cur == RLIM_INFINITY)
    /* The system limit is not usable.  Use an architecture-specific
       default.  */
    limit.rlim_cur = ARCH_STACK_DEFAULT_SIZE;
  else if (limit.rlim_cur < PTHREAD_STACK_MIN)
    /* The system limit is unusably small.
       Use the minimal size acceptable.  */
    limit.rlim_cur = PTHREAD_STACK_MIN;

  /* Make sure it meets the minimum size that allocate_stack
     (allocatestack.c) will demand, which depends on the page size.  */
  const uintptr_t pagesz = GLRO(dl_pagesize);
  const size_t minstack = pagesz + __static_tls_size + MINIMAL_REST_STACK;
  if (limit.rlim_cur < minstack)
    limit.rlim_cur = minstack;

  /* Round the resource limit up to page size.  */
  limit.rlim_cur = ALIGN_UP (limit.rlim_cur, pagesz);
  lll_lock (__default_pthread_attr_lock, LLL_PRIVATE);
  __default_pthread_attr.internal.stacksize = limit.rlim_cur;
  __default_pthread_attr.internal.guardsize = GLRO (dl_pagesize);
  lll_unlock (__default_pthread_attr_lock, LLL_PRIVATE);

#ifdef SHARED
  GL(dl_make_stack_executable_hook) = &__make_stacks_executable;
#endif

  /* Register the fork generation counter with the libc.  */
  __libc_pthread_init (__reclaim_stacks);
}
strong_alias (__pthread_initialize_minimal_internal,
	      __pthread_initialize_minimal)


/* This function is internal (it has a GLIBC_PRIVATE) version, but it
   is widely used (either via weak symbol, or dlsym) to obtain the
   __static_tls_size value.  This value is then used to adjust the
   value of the stack size attribute, so that applications receive the
   full requested stack size, not diminished by the TCB and static TLS
   allocation on the stack.  Once the TCB is separately allocated,
   this function should be removed or renamed (if it is still
   necessary at that point).  */
size_t
__pthread_get_minstack (const pthread_attr_t *attr)
{
  return GLRO(dl_pagesize) + __static_tls_size + PTHREAD_STACK_MIN;
}
