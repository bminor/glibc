/* Implementing POSIX.1 signals under the Hurd.
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

#ifndef	_HURD_SIGNAL_H

#define	_HURD_SIGNAL_H	1
#include <features.h>
/* Make sure <signal.h> is going to define NSIG.  */
#ifndef __USE_GNU
#error "Must have `_GNU_SOURCE' feature test macro to use this file"
#endif

#define __need_NULL
#include <stddef.h>

#include <mach/mach_types.h>
#include <mach/port.h>
#include <mach/message.h>
#include <hurd/hurd_types.h>
#include <signal.h>
#include <errno.h>

#include <mutex.h>
#include <lock-intern.h>


/* Per-thread signal state.  */

struct hurd_sigstate
  {
    /* XXX should be in cthread variable (?) */
    thread_t thread;
    struct hurd_sigstate *next; /* Linked-list of thread sigstates.  */

    struct mutex lock;		/* Locks the rest of this structure.  */
    sigset_t blocked;
    sigset_t pending;
    struct sigaction actions[NSIG];
    struct sigaltstack sigaltstack;
    int sigcodes[NSIG];		/* Codes for pending signals.  */

    int suspended;		/* If nonzero, sig_post signals `arrived'.  */
#ifdef noteven
    struct condition arrived;
#endif

#if 0
    int vforked;		/* Nonzero if this thread is a vfork child.  */
    struct
      {
	process_t proc;
	file_t ccdir, cwdir, crdir, auth;
	mode_t umask;
	int ctty_fstype;
	fsid_t ctty_fsid;
	ino_t ctty_fileid;
	struct hurd_dtable *dtable;
	jmp_buf continuation;
      } *vfork_saved;
#endif

    /* Not locked.  Used only by this thread,
       or by signal thread with this thread suspended.  */
    volatile mach_port_t intr_port; /* Port interruptible RPC was sent on.  */
    volatile int intr_restart;	/* If nonzero, restart interrupted RPC.  */
  };

/* Linked list of states of all threads whose state has been asked for.  */

extern struct hurd_sigstate *_hurd_sigstates;

extern struct mutex _hurd_siglock; /* Locks _hurd_sigstates.  */

/* Get the sigstate of a given thread, taking its lock.  */

extern struct hurd_sigstate *_hurd_thread_sigstate (thread_t);

/* Thread listening on our message port; also called the "signal thread".  */

extern thread_t _hurd_msgport_thread;

/* Our message port.  We hold the receive right and _hurd_msgport_thread
   listens for messages on it.  We also hold a send right, for convenience.  */

extern mach_port_t _hurd_msgport;


/* Thread to receive process-global signals.  */

extern thread_t _hurd_sigthread;


/* Resource limit on core file size.  Enforced by hurdsig.c.  */
extern int _hurd_core_limit;

/* Initialize the signal code, and start the signal thread.  */

extern void _hurdsig_init (void);

/* Initialize proc server-assisted fault recovery for the signal thread.  */

extern void _hurdsig_fault_init (void);

/* Raise a signal as described by SIGNO and SIGCODE, on the thread whose
   sigstate SS points to.  If SS is a null pointer, this instead affects
   the calling thread.  */

extern void _hurd_raise_signal (struct hurd_sigstate *ss,
				int signo, int sigcode);

/* Translate a Mach exception into a signal (machine-dependent).  */

extern void _hurd_exception2signal (int exception, int code, int subcode,
				    int *signo, int *sigcode);


/* Make the thread described by SS take the signal described by SIGNO and
   SIGCODE.  SS->lock is held on entry, and released before return.  */

extern void _hurd_internal_post_signal (struct hurd_sigstate *ss,
					int signo, int sigcode);

/* Set up STATE to handle signal SIGNO by running HANDLER.  FLAGS is the
   `sa_flags' member from `struct sigaction'.  If the SA_ONSTACK bit is
   set, *ALTSTACK describes the alternate signal stack to use.  The handler
   is passed SIGNO, SIGCODE, and the returned `struct sigcontext' (which
   resides on the stack the handler will use, and which describes the state
   of the thread encoded in STATE before running the handler).  */

extern struct sigcontext *_hurd_setup_sighandler (int flags,
						  __sighandler_t handler,
						  struct sigaltstack *altstack,
						  int signo, int sigcode,
						  void *state);

/* Function run by the signal thread to receive from the signal port.  */

extern void _hurd_msgport_receive (void);

/* Return nonzero if STATE indicates a thread that is blocked in a mach_msg
   system call (machine-dependent).  If returning nonzero, set *PORT to
   the receive right that the thread is blocked on.  */

extern int _hurd_thread_state_msging_p (void *state, mach_port_t *port);

/* Set up STATE with a thread state that, when resumed, is
   like `longjmp (_hurd_sigthread_fault_env, 1)'.  */

extern void _hurd_initialize_fault_recovery_state (void *state);


/* Function run for SIGINFO when its action is SIG_DFL and the current
   process is the session leader.  */

extern void _hurd_siginfo_handler (int);


#ifdef notyet

/* Perform interruptible RPC CALL on PORT.
   The args in CALL should be constant or local variable refs.
   They may be evaluated many times, and must not change.
   PORT must not be deallocated before this RPC is finished.  */
#define	HURD_EINTR_RPC(port, call) \
  ({
    error_t __err;
    struct hurd_sigstate *__ss
      = _hurd_thread_sigstate (__mach_thread_self ());
    __mutex_unlock (&__ss->lock); /* Lock not needed.  */
    /* If we get a signal and should return EINTR, the signal thread will
       clear this.  The RPC might return EINTR when some other thread gets
       a signal, in which case we want to restart our call.  */
    __ss->intr_restart = 1;
    /* This one needs to be last.  A signal can arrive before here,
       and if intr_port were set before intr_restart is
       initialized, the signal thread would get confused.  */
    __ss->intr_port = (port);
    /* A signal may arrive here, after intr_port is set,
       but before the mach_msg system call.  The signal handler might do an
       interruptible RPC, and clobber intr_port; then it would not be set
       properly when we actually did send the RPC, and a later signal
       wouldn't interrupt that RPC.  So, _hurd_run_sighandler saves
       intr_port in the sigcontext, and sigreturn restores it.  */
  __do_call:
    switch (__err = (call))
      {
      case EINTR:		/* RPC went out and was interrupted.  */
      case MACH_SEND_INTERRUPTED: /* RPC didn't get out.  */
	if (__ss->intr_restart)
	  /* Restart the interrupted call.  */
	  goto __do_call;
	/* FALLTHROUGH */
      case MACH_RCV_PORT_DIED:
	/* Server didn't respond to interrupt_operation,
	   so the signal thread destroyed the reply port.  */
	__err = EINTR;
	break;
      }
    __ss->intr_port = MACH_PORT_NULL;
    __err;
  })

#endif /* notyet */

/* Mask of signals that cannot be caught, blocked, or ignored.  */
#define	_SIG_CANT_MASK	(__sigmask (SIGSTOP) | __sigmask (SIGKILL))

/* Do an RPC to a process's message port.

   Each argument is an expression which returns an error code; each
   expression may be evaluated several times.  FETCH_MSGPORT_EXPR should
   fetch the appropriate message port and store it in the local variable
   `msgport'.  FETCH_REFPORT_EXPR should fetch the appropriate message port
   and store it in the local variable `refport' (if no reference port is
   needed in the call, then FETCH_REFPORT_EXPR should be simply
   KERN_SUCCESS or 0).  Both of these are assumed to create user
   references, which this macro deallocates.  RPC_EXPR should perform the
   desired RPC operation using `msgport' and `refport'.

   The reason for the complexity is that a process's message port and
   reference port may change between fetching those ports and completing an
   RPC using them (usually they change only when a process execs).  The RPC
   will fail with MACH_SEND_INVALID_DEST if the msgport dies before we can
   send the RPC request; or with MIG_SERVER_DIED if the msgport was
   destroyed after we sent the RPC request but before it was serviced.  In
   either of these cases, we retry the entire operation, discarding the old
   message and reference ports and fetch them anew.  */

#define HURD_MSGPORT_RPC(fetch_msgport_expr, fetch_refport_expr, rpc_expr)   \
({									      \
    error_t __err;							      \
    mach_port_t msgport, refport = MACH_PORT_NULL;			      \
    do									      \
      {									      \
	/* Get the message port.  */					      \
	if (__err = (fetch_msgport_expr))				      \
	  break;							      \
	/* Get the reference port.  */					      \
	if (__err = (fetch_refport_expr))				      \
	  {								      \
	    /* Couldn't get it; deallocate MSGPORT and fail.  */	      \
	    __mach_port_deallocate (__mach_task_self (), msgport);	      \
	    break;							      \
	  }								      \
	__err = (rpc_expr);						      \
	__mach_port_deallocate (__mach_task_self (), msgport);		      \
	if (refport != MACH_PORT_NULL)					      \
	  __mach_port_deallocate (__mach_task_self (), refport);    	      \
      } while (__err != MACH_SEND_INVALID_DEST &&			      \
	       __err != MIG_SERVER_DIED);				      \
    __err;								      \
})

/* Some other parts of the library need to preempt signals, to detect
   errors that should not result in a POSIX signal.  For example, when
   some mapped region of memory is used, an extraneous SIGSEGV might be
   generated when the mapping server returns an error for a page fault.  */

struct hurd_signal_preempt
  {
    /* Function to examine a thread receiving a given signal.  The handler
       is called even for blocked signals.  This function is run in the
       signal thread, with THREAD's sigstate locked; it should be as simple
       and robust as possible.  THREAD is the thread which is about to
       receive the signal.  SIGNO and SIGCODE would be passed to the normal
       handler.

       If the return value is SIG_DFL, normal signal processing continues.
       If it is SIG_IGN, the signal is ignored.
       Any other value is used in place of the normal handler.  */
    sighandler_t (*handler) (thread_t thread, int signo, int sigcode);
    int first, last;		/* Range of sigcodes this handler wants.  */
    struct hurd_signal_preempt *next; /* Next handler on the chain. */
  };

extern struct hurd_signal_preempt *_hurd_signal_preempt[NSIG];
extern struct mutex _hurd_signal_preempt_lock;


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif

/* Initialize PREEMPTER with the information given and stick it in the
   chain of preempters for SIGNO.  */

_EXTERN_INLINE int
hurd_preempt_signals (struct hurd_signal_preempt *preempter,
		      int signo, int first_code, int last_code,
		      sighandler_t (*handler) (thread_t, int, int))
{
  if (signo <= 0 || signo >= NSIG)
    {
      errno = EINVAL;
      return -1;
    }
  preempter->first = first_code;
  preempter->last = last_code;
  preempter->handler = handler;
  __mutex_lock (&_hurd_signal_preempt_lock);
  preempter->next = _hurd_signal_preempt[signo];
  _hurd_signal_preempt[signo] = preempter;
  __mutex_unlock (&_hurd_signal_preempt_lock);
  return 0;
}

/* Remove PREEMPTER from the chain for SIGNO.  */

_EXTERN_INLINE int
hurd_unpreempt_signals (struct hurd_signal_preempt *preempter, int signo)
{
  struct hurd_signal_preempt *p, *lastp;
  if (signo <= 0 || signo >= NSIG)
    {
      errno = EINVAL;
      return -1;
    }
  __mutex_lock (&_hurd_signal_preempt_lock);
  for (p = _hurd_signal_preempt[signo], lastp = NULL;
       p != NULL; lastp = p, p = p->next)
    if (p == preempter)
      {
	(lastp == NULL ? _hurd_signal_preempt[signo] : lastp->next) = p->next;
	__mutex_unlock (&_hurd_signal_preempt_lock);
	return 0;
      }
  _hurd_signal_preempt[signo] = preempter;
  __mutex_unlock (&_hurd_signal_preempt_lock);
  errno = ENOENT;
  return -1;
}


#endif	/* hurd/signal.h */
