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

#include <hurd.h>

struct mutex _hurd_siglock;
int _hurd_stopped;

/* Port that receives signals and other miscellaneous messages.  */
mach_port_t _hurd_sigport;

/* Thread which receives task-global signals.  */
thread_t _hurd_sigthread;

/* Linked-list of per-thread signal state.  */
struct _hurd_sigstate *_hurd_sigstates;

static void
init_sig (void)
{
  __mutex_init (&_hurd_siglock);
}
text_set_element (__libc_subinit, init_sig);


struct _hurd_sigstate *
_hurd_thread_sigstate (thread_t thread)
{
  struct _hurd_sigstate *ss;
  __mutex_lock (&_hurd_siglock);
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    if (ss->thread == thread)
      break;
  if (ss == NULL)
    {
      ss = calloc (1, sizeof (*ss)); /* Zero-initialized.  */
      if (ss == NULL)
	__libc_fatal ("hurd: Can't allocate thread sigstate\n");
      ss->thread = thread;
      __mutex_init (&ss->lock);
      ss->next = _hurd_sigstates;
      _hurd_sigstates = ss;
    }
  __mutex_lock (&ss->lock);
  __mutex_unlock (&_hurd_siglock);
  return ss;
}

#include <hurd/core.h>

/* Limit on size of core files.  */
int _hurd_core_limit;

/* Call the core server to mummify us before we die.  */
static inline int
write_corefile (int signo, int sigcode)
{
  error_t err;
  mach_port_t coreserver;

  if (_hurd_core_limit == 0)
    /* User doesn't want a core.  */
    return 0;

  coreserver = _hurd_path_lookup (_SERVERS_CORE, 0, 0);
  if (coreserver == MACH_PORT_NULL)
    return 0;
  if (err = __dir_lookup (_hurd_ccdir, "core",
			  FS_LOOKUP_WRITE|FS_LOOKUP_CREATE,
			  0666 & ~_hurd_umask,
			  &file))
    return 0;
  err = __core_dump_task (coreserver,
			  __mach_task_self (),
			  __mach_host_self (),
			  file,
			  signo, sigcode);
  __mach_port_deallocate (__mach_task_self (), coreserver);
  if (!err && _hurd_core_limit != RLIM_INFINITY)
    {
      io_statbuf_t stb;
      err = io_stat (file, &stb);
      if (!err && stb.stb_size > _hurd_core_limit)
	err = EFBIG;
    }
  __mach_port_deallocate (__mach_task_self (), file);
  if (err)
    (void) __dir_unlink (_hurd_ccdir, "core");
  return !err;
}


extern const size_t _hurd_thread_state_count;

/* SS->thread is suspended.  Fills STATE in with its registers.
   SS->lock is held and kept.  */
static inline void
abort_rpcs (struct _hurd_sigstate *ss, int signo, void *state)
{
  if (ss->intr_port != MACH_PORT_NULL)
    {
      /* This is the address the PC will be at if the thread
	 is waiting for a mach_msg syscall to return.  */
      extern const int __mach_msg_trap_syscall_pc;
      extern error_t _hurd_thread_state (thread_t, void *state);
      extern int *_hurd_thread_pc (void *state);
      
      /* Abort whatever the thread is doing, and fetch its state.  */
      __thread_abort (ss->thread);
      _hurd_thread_state (ss->thread, state);

      if (_hurd_thread_pc (state) == &__mach_msg_trap_syscall_pc)
	{
	  /* The thread was waiting for the RPC to return.
	     Abort the operation.  The RPC will return POSIX_EINTR,
	     or mach_msg will return an interrupt error.  */
	  if (ss->intr_is_wait)
	    __proc_waitintr (ss->intr_port);
	  else
	    __io_intr (ss->intr_port);
	  ss->intr_restart = ss->actions[signo].sa_flags & SA_RESTART;
	}

      /* If the thread is anywhere before the system call trap,
	 it will start the operation after the signal is handled.
	 
	 If the thread is after the system call trap, but before it has
	 cleared SS->intr_port, the operation is already finished.  */
    }
}

static inline void
abort_all_rpcs (int signo, void *state)
{
  thread_t me = __mach_thread_self ();
  thread_t *threads;
  size_t nthreads;

  for (__task_threads (__mach_task_self (), &list, &nthreads);
       nthreads-- > 0;
       __mach_port_deallocate (__mach_task_self (), *nthreads++))
    if (*nthreads != me)
      {
	struct _hurd_sigstate *ss = _hurd_thread_sigstate (*nthreads);
	abort_rpcs (ss, signo, state);
	__mutex_unlock (&ss->lock);
      }
}

static inline void
return_sig_post (reply_port_t reply, int willstop)
{
  if (reply != MACH_PORT_NULL)
    __sig_post_reply (reply, POSIX_SUCCESS, willstop);
}

/* SS->lock is held.  */
error_t
_hurd_internal_post_signal (reply_port_t reply,
			    struct _hurd_sigstate *ss,
			    int signo,
			    int sigcode,
			    int orphaned,
			    int cttykill,
			    int *willstop,
			    sigset_t *restore_blocked)
{
  char thread_state[_hurd_thread_state_count];
  enum { stop, ignore, core, term } act;

  if (ss->actions[signo].sa_handler == SIG_DFL)
    switch (signo)
      {
      case SIGTTIN:
      case SIGTTOU:
      case SIGSTOP:
      case SIGTSTP:
	ss->pending &= ~sigmask (SIGCONT);
	act = stop;
	break;

      case SIGCONT:
	ss->pending &= ~(sigmask (SIGSTOP) | sigmask (SIGTSTP) |
			 sigmask (SIGTTIN) | sigmask (SIGTTOU));
	/* Fall through.  */
      case SIGIO:
      case SIGURG:
      case SIGCHLD:
      case SIGWINCH:
	act = ignore;
	break;

      case SIGQUIT:
      case SIGILL:
      case SIGTRAP:
      case SIGIOT:
      case SIGEMT:
      case SIGFPE:
      case SIGBUS:
      case SIGSEGV:
      case SIGSYS:
	act = core;
	break;

      default:
	act = term;
	break;
      }
  else if (ss->actions[signo].sa_handler == SIG_IGN)
    act = ignore;
  else
    act = handle;
  if (orphaned && (signo == SIGTTIN || signo == SIGTTOU) && act == stop)
    {
      signo = SIGKILL;
      act = term;
    }

  /* Handle receipt of a blocked signal.  */
  if (((sigmask (signo) & ss->blocked) && act != ignore) ||
      (signo != SIGKILL && _hurd_stopped))
    {
      if (!cttykill)
	__sigaddmember (signo, &ss->pending);
      /* Save the code to be given to the handler when SIGNO is unblocked.  */
      ss->sigcodes[signo] = sigcode;
      act = ignore;
    }

  if (restore_blocked != NULL)
    ss->blocked = *restore_blocked;

  switch (act)
    {
    case stop:
      return_sig_post (reply, WILLSTOP);
      __proc_dostop (_hurd_proc, __mach_thread_self ());
      __mutex_unlock (&ss->lock);
      abort_all_rpcs (signo, thread_state);
      __proc_markstop (_hurd_proc, signo);
      _hurd_stopped = 1;
      return MIG_NO_REPLY;	/* Already replied.  */

    case ignore:
      return_sig_post (reply, IGNBLK);
      break;

    case core:
    case term:
      return_sig_post (reply, CATCH);
      __proc_dostop (_hurd_proc, __mach_thread_self ());
      __mutex_unlock (&ss->lock);
      abort_all_rpcs (signo, thread_state);
      __proc_exit (_hurd_proc,
		   (W_EXITCODE (0, signo) |
		    (act == core && write_corefile (signo, sigcode) ?
		     WCOREDUMP : 0)));
      __task_terminate (__mach_task_self ());
      return MIG_NO_REPLY;	/* Yeah, right.  */

    case handle:
      return_sig_post (reply, CATCH);
      __thread_suspend (ss->thread);
      abort_rpcs (ss, signo, thread_state);
      {
	const sigset_t blocked = ss->blocked;
	ss->blocked |= __sigmask (signo) | ss->actions[signo].sa_mask;
	_hurd_run_sighandler (ss->thread, signo, sigcode,
			      ss->actions[signo].sa_handler,
			      ss->actions[signo].sa_flags,
			      blocked,
			      &ss->sigstack,
			      thread_state);
      }
      __thread_resume (ss->thread);
    }

  /* We get here only if we are handling or ignoring the signal;
     otherwise we are stopped or dead by now.  We still hold SS->lock.
     Check for pending signals, and loop to post them.  */
  for (signo = 1; signo < NSIG; ++signo)
    if (__sigismember (signo, &ss->pending))
      {
	__sigdelmember (signo, &ss->pending);
	return _hurd_internal_post_signal (MACH_PORT_NULL, ss,
					   signo, ss->sigcodes[signo],
					   0, 0, willstop);
      }

  __mutex_unlock (&ss->lock);
  return MIG_NO_REPLY;
}

/* Called by the proc server to send a signal.  */
static error_t
sig_post (sigthread_t me,
	  mig_reply_port_t reply,
	  int signo,
	  int orphaned,
	  int cttykill,
	  int *willstop)
{
  struct _hurd_sigstate *ss;

  if (signo < 0 || signo >= NSIG)
    return POSIX_EINVAL;

  ss = _hurd_thread_sigstate (_hurd_sigthread);
  if (ss->suspended)
    {
      ss->suspend_reply = reply;
      __condition_broadcast (&ss->arrived);
      return MIG_NO_REPLY;
    }

  return _hurd_internal_post_signal (reply,
				     signo, 0, orphaned, cttykill, willstop,
				     NULL);
}

#include "_Xsig_post.c"

asm (".stabs \"__hurd_sigport_ids\",23,0,0,23000"); /* XXX */
text_set_element (_hurd_sigport_routines, _Xsig_post);


/* Called by the exception handler to take a signal.  */
void
_hurd_exc_post_signal (thread_t thread, int signo, int sigcode)
{
  int ignore;
  (void) _hurd_internal_post_signal (MACH_PORT_NULL,
				     _hurd_thread_sigstate (thread),
				     signo, sigcode,
				     0, 0, &ignore,
				     NULL);
}
