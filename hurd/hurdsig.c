/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <gnu-stabs.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <hurd/msg_reply.h>	/* For __sig_post_reply.  */

#ifdef noteven
struct mutex _hurd_siglock;
#endif
int _hurd_stopped;

/* Port that receives signals and other miscellaneous messages.  */
mach_port_t _hurd_msgport;

/* Thread listening on it.  */
thread_t _hurd_msgport_thread;

/* Thread which receives task-global signals.  */
thread_t _hurd_sigthread;

/* Linked-list of per-thread signal state.  */
struct hurd_sigstate *_hurd_sigstates;

struct hurd_sigstate *
_hurd_thread_sigstate (thread_t thread)
{
  struct hurd_sigstate *ss;
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
#ifdef noteve
      __mutex_init (&ss->lock);
#endif
      ss->next = _hurd_sigstates;
      _hurd_sigstates = ss;
    }
  __mutex_lock (&ss->lock);
  __mutex_unlock (&_hurd_siglock);
  return ss;
}

#include <hurd/fd.h>
#include <hurd/core.h>
#include <hurd/paths.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "thread_state.h"

jmp_buf _hurd_sigthread_fault_env;

/* Call the core server to mummify us before we die.
   Returns nonzero if a core file was written.  */
static int
write_corefile (int signo, int sigcode)
{
  error_t err;
  volatile mach_port_t coreserver;
  volatile file_t file;
  char *volatile name;
  char *volatile target;

  /* XXX RLIMIT_CORE */

  coreserver = MACH_PORT_NULL;
  if (!setjmp (_hurd_sigthread_fault_env))
    {
      name = getenv ("CORESERVER");
      if (name != NULL)
	coreserver = __path_lookup (name, 0, 0);
    }

  if (coreserver == MACH_PORT_NULL)
    coreserver = __path_lookup (_SERVERS_CORE, 0, 0);
  if (coreserver == MACH_PORT_NULL)
    return 0;

  file = MACH_PORT_NULL;
  name = NULL;
  if (!setjmp (_hurd_sigthread_fault_env))
    {
      name = getenv ("COREFILE");
      if (name != NULL)
	file = __path_lookup (name, O_WRONLY|O_CREAT, 0666 & ~_hurd_umask);
    }
  if (name == NULL || file == MACH_PORT_NULL)
    {
      name = (char *) "core";
      file = __path_lookup (name, O_WRONLY|O_CREAT, 0666 & ~_hurd_umask);
    }

  if (file == MACH_PORT_NULL)
    return 0;

  if (setjmp (_hurd_sigthread_fault_env))
    /* We bombed in getenv.  */
    target = NULL;
  else
    {
      target = getenv ("GNUTARGET");
      /* Fault now if TARGET is a bogus string.  */
      (void) strlen (target);
    }

  err = __core_dump_task (coreserver,
			  __mach_task_self (),
			  file,
			  signo, sigcode,
			  target);
  __mach_port_deallocate (__mach_task_self (), coreserver);
  __mach_port_deallocate (__mach_task_self (), file);
  if (err)
    (void) remove (name);
  return !err;
}


/* How long to give servers to respond to
   interrupt_operation before giving up on them.  */
mach_msg_timeout_t _hurd_interrupt_timeout = 1000; /* One second.  */

/* SS->thread is suspended.  Fills STATE in with its registers.
   SS->lock is held and kept.  */
static inline void
abort_rpcs (struct hurd_sigstate *ss, int signo, void *state)
{
  if (ss->intr_port != MACH_PORT_NULL)
    {
      /* Abort whatever the thread is doing.
	 If it is in the mach_msg syscall doing the send,
	 the syscall will return MACH_SEND_INTERRUPTED.  */
      unsigned int count;
      __thread_abort (ss->thread);
      __thread_get_state (ss->thread, MACHINE_THREAD_STATE_FLAVOR,
			  state, &count);
      if (count != MACHINE_THREAD_STATE_COUNT)
	/* What kind of thread?? */
	return;			/* XXX */

      if (_hurd_thread_state_msging_p (state))
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

	  msg.header.msgh_remote_port = ss->intr_port;
	  msg.header.msgh_local_port = __mach_reply_port ();
	  msg.header.msgh_seqno = 0;
	  msg.header.msgh_id = 33000; /* interrupt_operation XXX */
	  err = __mach_msg (&msg.header,
			    MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_TIMEOUT,
			    sizeof (msg.header), sizeof (msg),
			    msg.header.msgh_local_port,
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
  mach_msg_type_number_t nthreads, i;

  __task_threads (__mach_task_self (), &threads, &nthreads);
  for (i = 0; i < nthreads; ++i)
    {
      if (threads[i] != me)
	{
	  struct hurd_sigstate *ss = _hurd_thread_sigstate (threads[i]);
	  abort_rpcs (ss, signo, state);
	  __mutex_unlock (&ss->lock);
	}
      __mach_port_deallocate (__mach_task_self (), threads[i]);
    }
}


struct hurd_signal_preempt *_hurd_signal_preempt[NSIG];
#ifdef noteven
struct mutex _hurd_signal_preempt_lock;
#endif

/* Deliver a signal.
   SS->lock is held on entry and released before return.  */
void
_hurd_internal_post_signal (struct hurd_sigstate *ss,
			    int signo,
			    int sigcode)
{
  struct machine_thread_state thread_state;
  enum { stop, ignore, core, term, handle } act;
  sighandler_t handler;
  struct hurd_signal_preempt *pe;
  sighandler_t (*preempt) (thread_t, int, int) = NULL;

  /* Check for a preempted signal.  */
  __mutex_lock (&_hurd_signal_preempt_lock);
  for (pe = _hurd_signal_preempt[signo]; pe != NULL; pe = pe->next)
    if (sigcode >= pe->first && sigcode <= pe->last)
      {
	preempt = pe->handler;
	break;
      }
  __mutex_unlock (&_hurd_signal_preempt_lock);

  handler = SIG_DFL;
  if (preempt)
    /* Let the preempting handler examine the thread.
       If it returns SIG_DFL, we run the normal handler;
       otherwise we use the handler it returns.  */
    handler = (*preempt) (ss->thread, signo, sigcode);
  if (handler != SIG_DFL)
    handler = ss->actions[signo].sa_handler;

  if (handler == SIG_DFL)
    /* Figure out the default action for this signal.  */
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

      case SIGINFO:
	if (_hurd_pgrp == _hurd_pid)
	  {
	    /* We are the session leader.  Since there is no user-specified
	       handler for SIGINFO, we use a default one which prints
	       something interesting.  We use the normal handler mechanism
	       instead of just doing it here to avoid the signal thread
	       faulting or blocking in this potentially hairy operation.  */
	    act = handle;
	    handler = _hurd_siginfo_handler;
	  }
	break;

      default:
	act = term;
	break;
      }
  else if (handler == SIG_IGN)
    act = ignore;
  else
    act = handle;

  if (_hurd_orphaned && act == stop &&
      (signo & (__sigmask (SIGTTIN) | __sigmask (SIGTTOU) |
		__sigmask (SIGTSTP))))
    {
      /* If we would ordinarily stop for a job control signal, but we are
	 orphaned so noone would ever notice and continue us again, we just
	 quietly die, alone and in the dark.  */
      sigcode = signo;
      signo = SIGKILL;
      act = term;
    }

  /* Handle receipt of a blocked signal.  */
  if ((__sigismember (&ss->blocked, signo) && act != ignore) ||
      (signo != SIGKILL && _hurd_stopped))
    {
      __sigaddset (&ss->pending, signo);
      /* Save the code to be given to the handler when SIGNO is unblocked.  */
      ss->sigcodes[signo] = sigcode;
      act = ignore;
    }

  /* Perform the chosen action for the signal.  */
  switch (act)
    {
    case stop:
      /* Stop all other threads and mark ourselves stopped.  */
      __USEPORT (PROC,
		 ({
		   /* Hold the siglock while stopping other threads to be
		      sure it is not held by another thread afterwards.  */
		   __mutex_unlock (&ss->lock);
		   __mutex_lock (&_hurd_siglock);
		   __proc_dostop (port, __mach_thread_self ());
		   __mutex_unlock (&_hurd_siglock);
		   abort_all_rpcs (signo, &thread_state);
		   __proc_mark_stop (port, signo);
		 }));
      _hurd_stopped = 1;

#ifdef noteven
      __mutex_lock (&ss->lock);
      if (ss->suspended)
	/* There is a sigsuspend waiting.  Tell it to wake up.  */
	__condition_signal (&ss->arrived);
      else
	__mutex_unlock (&ss->lock);
#endif

      return;

    case ignore:
      /* Nobody cares about this signal.  */
      break;

    case term:			/* Time to die.  */
    case core:			/* And leave a rotting corpse.  */
      /* Have the proc server stop all other threads in our task.  */
      __USEPORT (PROC, __proc_dostop (port, __mach_thread_self ()));
      /* Abort all server operations now in progress.  */
      abort_all_rpcs (signo, &thread_state);
      /* Tell proc how we died and then stick the saber in the gut.  */
      _hurd_exit (W_EXITCODE (0, signo) |
		  /* Do a core dump if desired.  Only set the wait status
                     bit saying we in fact dumped core if the operation was
                     actually succesful.  */
		  (act == core && write_corefile (signo, sigcode) ?
		   WCOREFLAG : 0));
      /* NOTREACHED */

    case handle:
      /* Call a handler for this signal.  */
      {
	struct sigcontext *scp;

	/* Stop the thread and abort its pending RPC operations.  */
	__thread_suspend (ss->thread);
	abort_rpcs (ss, signo, &thread_state);

	/* Call the machine-dependent function to set the thread up
	   to run the signal handler, and preserve its old context.  */
	scp = _hurd_setup_sighandler (ss->actions[signo].sa_flags,
				      handler,
				      &ss->sigaltstack,
				      signo, sigcode,
				      &thread_state);

	/* Set the machine-independent parts of the signal context.  */
	scp->sc_intr_port = ss->intr_port;
	scp->sc_mask = ss->blocked;

	/* Block SIGNO and requested signals while running the handler.  */
	ss->blocked |= __sigmask (signo) | ss->actions[signo].sa_mask;

	/* Start the thread running the handler.  */
	__thread_set_state (ss->thread, MACHINE_THREAD_STATE_FLAVOR,
			    (int *) &thread_state, MACHINE_THREAD_STATE_COUNT);
	__thread_resume (ss->thread);
	break;
      }
    }

  /* We get here only if we are handling or ignoring the signal;
     otherwise we are stopped or dead by now.  We still hold SS->lock.
     Check for pending signals, and loop to post them.  */
  for (signo = 1; signo < NSIG; ++signo)
    if (__sigismember (&ss->pending, signo))
      {
	__sigdelset (&ss->pending, signo);
	_hurd_internal_post_signal (ss, signo, ss->sigcodes[signo]);
	return;
      }

#ifdef noteven
  if (ss->suspended)
    /* There is a sigsuspend waiting.  Tell it to wake up.  */
    __condition_signal (&ss->arrived);
  else
    __mutex_unlock (&ss->lock);
#endif
}

/* Implement the sig_post RPC from <hurd/msg.defs>;
   sent when someone wants us to get a signal.  */
error_t
_S_sig_post (mach_port_t me,
	     mach_port_t reply,
	     int signo,
	     mach_port_t refport)
{
  struct hurd_sigstate *ss;

  if (signo < 0 || signo >= NSIG)
    return EINVAL;

  if (refport == __mach_task_self ())
    /* Can send any signal.  */
    goto win;

  /* Avoid needing to check for this below.  */
  if (refport == MACH_PORT_NULL)
    return EPERM;

  switch (signo)
    {
    case SIGINT:
    case SIGQUIT:
    case SIGTSTP:
    case SIGHUP:
    case SIGINFO:
      /* Job control signals can be sent by the controlling terminal.  */
      if (__USEPORT (CTTYID, port == refport))
	goto win;
      break;

    case SIGCONT:
      {
	/* A continue signal can be sent by anyone in the session.  */
	mach_port_t sessport;
	if (! __USEPORT (PROC, __proc_getsidport (port, &sessport)))
	  { 
	    int win = refport == sessport;
	    __mach_port_deallocate (__mach_task_self (), sessport);
	    if (win)
	      goto win;
	  }
      }
      break;

    case SIGIO:
    case SIGURG:
      {
	/* Any io object a file descriptor refers to might send us
	   one of these signals using its async ID port for REFPORT.

	   This is pretty wide open; it is not unlikely that some random
	   process can at least open for reading something we have open,
	   get its async ID port, and send us a spurious SIGIO or SIGURG
	   signal.  But BSD is actually wider open than that!--you can set
	   the owner of an io object to any process or process group
	   whatsoever and send them gratuitous signals.

	   Someday we could implement some reasonable scheme for
	   authorizing SIGIO and SIGURG signals properly.  */

	struct hurd_userlink dtable_ulink;
	struct hurd_dtable dt = _hurd_dtable_get (&dtable_ulink);
	int d;
	for (d = 0; d >= 0 && d < dt.size; ++d)
	  {
	    struct hurd_userlink ulink;
	    io_t port;
	    mach_port_t asyncid;
	    if (dt.d[d] == NULL)
	      continue;
	    port = _hurd_port_locked_get (&dt.d[d]->port, &ulink);
	    if (! __io_get_icky_async_id (port, &asyncid))
	      {
		if (refport == asyncid)
		  /* Break out of the loop on the next iteration.  */
		  d = -1;
		__mach_port_deallocate (__mach_task_self (), asyncid);
	      }
	    _hurd_port_free (&dt.d[d]->port, &ulink, port);
	  }
	_hurd_dtable_free (dt, &dtable_ulink);
	/* If we found a lucky winner, we've set D to -1 in the loop.  */
	if (d < 0)
	  goto win;
      }
    }

  /* If this signal is legit, we have done `goto win' by now.
     When we return the error, mig deallocates REFPORT.  */
  return EPERM;

 win:
  /* Deallocate the REFPORT right; we are done with it.  */
  __mach_port_deallocate (__mach_task_self (), refport);

  /* Get a hold of the designated signal-receiving thread.  */
  ss = _hurd_thread_sigstate (_hurd_sigthread);

  /* XXX POSIX.1-1990 p56 ll605-607 says we must deliver a signal
     before `kill' (and thus sig_post) can return.  */

  /* Send a reply indicating success to the signaller.  */
  __sig_post_reply (reply, 0);

  /* Post the signal.  */
  _hurd_internal_post_signal (ss, signo, 0);

  return MIG_NO_REPLY;		/* Already replied.  */
}

#include <mach/task_special_ports.h>

/* Initialize the message port and signal thread once.
   Do nothing on subsequent calls.  */

void
_hurdsig_init (void)
{
  error_t err;

    /* If _hurd_msgport is already set, we are in the child side of a fork.
       We already have a message port set up by in __fork.c the parent, and
       the parent took the siglock before we copied its memory, so it still
       appears locked for us (__fork will unlock it after we return).  */

  if (_hurd_msgport == MACH_PORT_NULL)
    {
#ifdef noteven
      __mutex_init (&_hurd_siglock); /* Initialize the signal lock.  */
#endif

      if (err = __mach_port_allocate (__mach_task_self (),
				      MACH_PORT_RIGHT_RECEIVE,
				      &_hurd_msgport))
	__libc_fatal ("hurd: Can't create message port receive right\n");

      /* Make a send right to the signal port.  */
      if (err = __mach_port_insert_right (__mach_task_self (),
					  _hurd_msgport,
					  _hurd_msgport,
					  MACH_MSG_TYPE_MAKE_SEND))
	__libc_fatal ("hurd: Can't create send right to message port\n");
    }

  /* Start the signal thread listening on the message port.  */

  if (err = __thread_create (__mach_task_self (), &_hurd_msgport_thread))
    __libc_fatal ("hurd: Can't create signal thread\n");

  if (err = __mach_setup_thread (__mach_task_self (), _hurd_msgport_thread,
				_hurd_msgport_receive))
    __libc_fatal ("hurd: Can't setup signal thread\n");
  if (err = __thread_resume (_hurd_msgport_thread))
    __libc_fatal ("hurd: Can't resume signal thread\n");
    
  /* Receive exceptions on the signal port.  */
  __task_set_special_port (__mach_task_self (),
			   TASK_EXCEPTION_PORT, _hurd_msgport);
}

/* Send exceptions for the signal thread to the proc server.
   It will forward the message on to our message port,
   and then restore the thread's state to code which
   does `longjmp (_hurd_sigthread_fault_env, 1)'.  */

void
_hurdsig_fault_init (void)
{
  error_t err;
  mach_port_t sigexc;
  struct machine_thread_state state;

  if (err = __mach_port_allocate (__mach_task_self (),
				  MACH_PORT_RIGHT_RECEIVE, &sigexc))
    __libc_fatal ("hurd: Can't create receive right for signal thread exc\n");

  /* Set up STATE with a thread state that will longjmp immediately.  */
  _hurd_initialize_fault_recovery_state (&state);

  __thread_set_special_port (_hurd_msgport_thread,
			     THREAD_EXCEPTION_PORT, sigexc);

  if (err = __USEPORT
      (PROC,
       __proc_handle_exceptions (port,
				 sigexc,
				 _hurd_msgport, MACH_MSG_TYPE_COPY_SEND,
				 MACHINE_THREAD_STATE_FLAVOR,
				 (int *) &state, MACHINE_THREAD_STATE_COUNT)))
    __libc_fatal ("hurd: proc won't handle signal thread exceptions\n");
}
				/* XXXX */
static void
reauth_proc (mach_port_t new)
{
  mach_port_t ignore;

  /* Reauthenticate with the proc server.  */
  if (! HURD_PORT_USE (&_hurd_ports[INIT_PORT_PROC],
		       __proc_reauthenticate (port, _hurd_pid) ||
		       __auth_user_authenticate (new, port, _hurd_pid,
						 &ignore))
      && ignore != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), ignore);
}
text_set_element (__hurd_reauth_hook, reauth_proc);

/* Setting up signals in the child for fork.  */

static error_t
hurdsig_fork (task_t newtask, process_t newproc)
{
  error_t err;
  mach_port_t msgport, oldmsgport;

  /* Create the message port to be used by the child.  */
  if (err = __mach_port_allocate (__mach_task_self (),
				  MACH_PORT_RIGHT_RECEIVE, &msgport))
    return err;

  /* Get us a send right for the child's message port.  */
  if (err = __mach_port_insert_right (__mach_task_self (), msgport,
				      msgport, MACH_MSG_TYPE_MAKE_SEND))
    {
      __mach_port_mod_refs (__mach_task_self (), msgport,
			    MACH_PORT_RIGHT_RECEIVE, -1);
      return err;
    }

  /* Move the receive right for the message port to the child,
     giving it the same name our message port has for us.  */
  if (err = __mach_port_insert_right (newtask, _hurd_msgport,
				      msgport, MACH_MSG_TYPE_MOVE_RECEIVE))
    {
      __mach_port_mod_refs (__mach_task_self (), msgport,
			    MACH_PORT_RIGHT_RECEIVE, -1);
      goto lose;
    }

  /* Give the proc server the new task's message port.  */
  if (err = __proc_setmsgport (newproc, msgport, &oldmsgport))
    goto lose;
  if (oldmsgport)
    /* Where did that come from?  */
    __mach_port_deallocate (__mach_task_self (), oldmsgport);

 lose:
  __mach_port_deallocate (__mach_task_self (), msgport);

  return err;
}

text_set_element (_hurd_fork_setup_hook, hurdsig_fork);
text_set_element (_hurd_fork_child_hook, _hurdsig_init);
text_set_element (_hurd_fork_child_hook, _hurdsig_fault_init);
