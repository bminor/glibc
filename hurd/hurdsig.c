/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <gnu-stabs.h>

struct mutex _hurd_siglock;
int _hurd_stopped;

/* Port that receives signals and other miscellaneous messages.  */
mach_port_t _hurd_sigport;

/* Thread which receives task-global signals.  */
thread_t _hurd_sigthread;

/* Linked-list of per-thread signal state.  */
struct _hurd_sigstate *_hurd_sigstates;

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

/* Call the core server to mummify us before we die.
   Returns nonzero if a core file was written.  */
static int
write_corefile (int signo, int sigcode)
{
  error_t err;
  mach_port_t coreserver;
  file_t file;
  char *name;

  if (_hurd_core_limit == 0)
    /* User doesn't want a core.  */
    return 0;

  coreserver = __path_lookup (_SERVERS_CORE, 0, 0);
  if (coreserver == MACH_PORT_NULL)
    return 0;

  name = getenv ("COREFILE");
  if (name != NULL)
    file = __path_lookup (name, FS_LOOKUP_WRITE|FS_LOOKUP_CREATE,
			  0666 & ~_hurd_umask);
  if (name == NULL || file == MACH_PORT_NULL)
    {
      name = "core";
      file = __path_lookup (name, FS_LOOKUP_WRITE|FS_LOOKUP_CREATE,
			    0666 & ~_hurd_umask);
    }

  if (file == MACH_PORT_NULL)
    return 0;

  err = __core_dump_task (coreserver,
			  __mach_task_self (),
			  file,
			  signo, sigcode,
			  getenv ("GNUTARGET"));
  __mach_port_deallocate (__mach_task_self (), coreserver);
  if (!err && _hurd_core_limit != RLIM_INFINITY)
    {
      io_statbuf_t stb;
      err = __io_stat (file, &stb);
      if (!err && stb.stb_size > _hurd_core_limit)
	err = EFBIG;
    }
  __mach_port_deallocate (__mach_task_self (), file);
  if (err)
    (void) remove (name);
  return !err;
}


extern const size_t _hurd_thread_state_count;

/* How long to give servers to respond to
   interrupt_operation before giving up on them.  */
mach_msg_timeout_t _hurd_interrupt_timeout = 1000; /* One second.  */

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
      
      /* Abort whatever the thread is doing.
	 If it is in the mach_msg syscall doing the send,
	 the syscall will return MACH_SEND_INTERRUPTED.  */
      __thread_abort (ss->thread);
      _hurd_thread_state (ss->thread, state);

      if (_hurd_thread_pc (state) == &__mach_msg_trap_syscall_pc)
	{
	  /* The thread was waiting for the RPC to return.
	     Abort the operation.  The RPC will return EINTR.  */

	  struct
	    {
	      mach_msg_header_t header;
	      mach_msg_type_t type;
	      kern_return_t retcode;
	    } msg;
	  kern_return_t err;

	  msg.header.msgh_request_port = ss->intr_port;
	  msg.header.msgh_reply_port = __mach_reply_port ();
	  msg.header.msgh_seqno = 0;
	  msg.header.msgh_id = 33000; /* interrupt_operation XXX */
	  err = __mach_msg (&msg.header,
			    MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_TIMEOUT,
			    sizeof (msg.header), sizeof (msg),
			    msg.header.msgh_reply_port,
			    _hurd_interrupt_timeout,
			    MACH_PORT_NULL);
	  if (err != MACH_MSG_SUCCESS)
	    /* The interrupt didn't work.
	       Destroy the receive right the thread is blocked on.  */
	    __mach_port_destroy (__mach_task_self (),
				 /* XXX */
				 _hurd_thread_reply_port (ss->thread));
	  else
	    /* In case the server returned something screwy.  */
	    __mach_msg_destroy (&msg.header);

	  /* Tell the thread whether it should restart the
	     operation or return EINTR when it wakes up.  */
	  ss->intr_restart = ss->actions[signo].sa_flags & SA_RESTART;
	}

      /* If the thread is anywhere before the system call trap,
	 it will start the operation after the signal is handled.
	 
	 If the thread is after the system call trap, but before it has
	 cleared SS->intr_port, the operation is already finished.  */
    }
}

/* Abort the RPCs being run by all threads but this one;
   all other threads should be suspended.  */
static inline void
abort_all_rpcs (int signo, void *state)
{
  thread_t me = __mach_thread_self ();
  thread_t *threads;
  size_t nthreads, i;

  __task_threads (__mach_task_self (), &threads, &nthreads);
  for (i = 0; i < nthreads; ++i)
    {
      if (threads[i] != me)
	{
	  struct _hurd_sigstate *ss = _hurd_thread_sigstate (*nthreads);
	  abort_rpcs (ss, signo, state);
	  __mutex_unlock (&ss->lock);
	}
      __mach_port_deallocate (__mach_task_self (), threads[i]);
    }
}


/* Deliver a signal.
   SS->lock is held on entry and released before return.  */
void
_hurd_internal_post_signal (struct _hurd_sigstate *ss,
			    int signo,
			    int sigcode,
			    sigset_t *restore_blocked)
{
  char thread_state[_hurd_thread_state_count];
  enum { stop, ignore, core, term } act;

  if (ss->actions[signo].sa_handler == SIG_DFL)
    switch (signo)
      {
      case 0:
	/* A sig_post msg with SIGNO==0 is sent to
	   tell us to check for pending signals.  */
	act = ignore;
	break;

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
  if (_hurd_orphaned &&
      (signo == SIGTTIN || signo == SIGTTOU || signo == SIGTSTP) &&
      act == stop)
    {
      sigcode = signo;
      signo = SIGKILL;
      act = term;
    }

  /* Handle receipt of a blocked signal.  */
  if ((__sigismember (signo, &ss->blocked) && act != ignore) ||
      (signo != SIGKILL && _hurd_stopped))
    {
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
      _HURD_PORT_USE
	(&_hurd_proc,
	 ({
	   /* Hold the siglock while stopping other threads to be
	      sure it is not held by another thread afterwards.  */
	   __mutex_unlock (&ss->lock);
	   __mutex_lock (&_hurd_siglock);
	   __proc_dostop (port, __mach_thread_self ());
	   __mutex_unlock (&_hurd_siglock);
	   abort_all_rpcs (signo, thread_state);
	   __proc_markstop (port, signo);
	 }));
      _hurd_stopped = 1;

      __mutex_lock (&ss->lock);
      if (ss->suspended)
	/* There is a sigsuspend waiting.  Tell it to wake up.  */
	__condition_signal (&ss->arrived);
      else
	__mutex_unlock (&ss->lock);
	
      return;

    case ignore:
      break;

    case core:
    case term:
      _HURD_PORT_USE
	(&_hurd_proc,
	 ({
	   __proc_dostop (port, __mach_thread_self ());
	  abort_all_rpcs (signo, thread_state);
	  __proc_exit (port,
		       (W_EXITCODE (0, signo) |
			(act == core && write_corefile (signo, sigcode) ?
			 WCOREDUMP : 0)))
	  }));
      __task_terminate (__mach_task_self ());
      return;			/* Yeah, right.  */

    case handle:
      __thread_suspend (ss->thread);
      abort_rpcs (ss, signo, thread_state);
      {
	const sigset_t blocked = ss->blocked;
	ss->blocked |= __sigmask (signo) | ss->actions[signo].sa_mask;
	_hurd_run_sighandler (ss, signo, sigcode, blocked, thread_state);
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
	_hurd_internal_post_signal (ss, signo, ss->sigcodes[signo], NULL);
	return;
      }

  if (ss->suspended)
    /* There is a sigsuspend waiting.  Tell it to wake up.  */
    __condition_signal (&ss->arrived);
  else
    __mutex_unlock (&ss->lock);
}

/* Sent when someone wants us to get a signal.  */
error_t
__sig_post (sigthread_t me,
	    mig_reply_port_t reply,
	    int signo,
	    mach_port_t refport)
{
  struct _hurd_sigstate *ss;

  if (signo < 0 || signo >= NSIG)
    return EINVAL;

  if (refport == __mach_task_self ())
    /* Can send any signal.  */
    goto win;
  else if (refport == _hurd_cttyport)
    switch (signo)
      {
      case SIGINT:
      case SIGQUIT:
      case SIGTSTP:
      case SIGHUP:
	goto win;
      }
  else
    {
      static mach_port_t sessport = MACH_PORT_NULL;
      if (sessport == MACH_PORT_NULL)
	_HURD_PORT_USE (&_hurd_proc,
			__proc_getsidport (port, &sessport));
      if (sessport != MACH_PORT_NULL && refport == sessport &&
	  signo == SIGCONT)
	goto win;
    }

  /* XXX async io? */
  return EPERM;

 win:
  ss = _hurd_thread_sigstate (_hurd_sigthread);
  __sig_post_reply (reply, 0);
  _hurd_internal_post_signal (ss, signo, 0, NULL);
  return MIG_NO_REPLY;		/* Already replied.  */
}

/* Called by the exception handler to take a signal.  */
void
_hurd_exc_post_signal (thread_t thread, int signo, int sigcode)
{
  _hurd_internal_post_signal (_hurd_thread_sigstate (thread),
			      signo, sigcode, NULL);
}

#include <sysdep.h>

/* Handle signal SIGNO in the calling thread.
   If SS is not NULL it is the sigstate for the calling thread;
   SS->lock is held on entry and released before return.  */
void
_hurd_raise_signal (struct _hurd_sigstate *ss, int signo, int sigcode)
{
  jmp_buf env;
  struct sigcontext sc;

  if (ss == NULL)
    ss = _hurd_thread_sigstate (__mach_thread_self ());

  if (! setjmp (env))
    {
      register volatile void (*handler) (int signo, int sigcode,
					 struct sigcontext *scp);

      handler = (__typeof (handler)) ss->actions[signo].sa_handler;

      /* Set up SC to make setjmp return 1.  */
      _hurd_jmp_buf_sigcontext (env, &sc, 1);

      sc.sc_mask = ss->blocked;	/* Restored by sigreturn.  */
      sc.sc_onstack = ((ss->actions[signo].sa_flags & SA_ONSTACK) &&
		       !(ss->sigaltstack.ss_flags & SA_DISABLE));
      if (sc.sc_onstack)
	{
	  /* Switch to the signal stack.  */
	  ss->sigaltstack.ss_flags |= SA_ONSTACK;
	  SET_SP (ss->sigaltstack.ss_sp);
	}

      __mutex_unlock (&ss->lock);

      /* Call the handler.  */
      (*handler) (signo, sigcode, &sc);

      __sigreturn (&sc);	/* Does not return.   */
      LOSE;			/* Firewall.  */
    }
}

void
_hurdsig_init (void)
{
  thread_t sigthread;

  __mutex_init (&_hurd_siglock);

  if (_hurd_sigport == MACH_PORT_NULL)
    if (err = __mach_port_allocate (__mach_task_self (),
				    MACH_PORT_RIGHT_RECEIVE,
				    &_hurd_sigport))
      __libc_fatal ("hurd: Can't create signal port receive right\n");

  if (err = __thread_create (__mach_task_self (), &sigthread))
    __libc_fatal ("hurd: Can't create signal thread\n");
  if (err = _hurd_start_sigthread (sigthread, _hurd_sigport_receive))
    __libc_fatal ("hurd: Can't start signal thread\n");
  _hurd_sigport_thread = sigthread;

  /* Make a send right to the signal port.  */
  if (err = __mach_port_insert_right (__mach_task_self (),
				      _hurd_sigport,
				      MACH_PORT_RIGHT_MAKE_SEND))
    __libc_fatal ("hurd: Can't create send right to signal port\n");

  /* Receive exceptions on the signal port.  */
  __task_set_special_port (__mach_task_self (),
			   TASK_EXCEPTION,
			   _hurd_sigport);
}

/* Make PROCSERVER be our proc server port.
   Tell the proc server that we exist.  */

void
_hurd_proc_init (process_t procserver, char **argv)
{
  mach_port_t oldsig, oldtask;

  _hurd_port_init (&_hurd_proc, procserver);

  /* Tell the proc server where our args and environment are.  */
  __proc_setprocargs (procserver, argv, __environ);

  /* Initialize the signal code; Mach exceptions will become signals.
     This sets _hurd_sigport; it must be run before _hurd_proc_init.  */
  _hurdsig_init ();

  /* Give the proc server our task and signal ports.  */
  __proc_setports (procserver,
		   _hurd_sigport, __mach_task_self (),
		   &oldsig, &oldtask);
  if (oldsig != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldsig);
  if (oldtask != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), oldtask);
}
