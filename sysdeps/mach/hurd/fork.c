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

#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/signal.h>
#include <setjmp.h>
#include "thread_state.h"
#include <sysdep.h>		/* For stack growth direction.  */
#include "set-hooks.h"

extern void _hurd_longjmp_thread_state (struct machine_thread_state *,
					jmp_buf env, int value);


/* Things that want to be locked while forking.  */
const struct
  {
    size_t n;
    struct mutex *locks[0];
  } _hurd_fork_locks;


/* Things that want to be called before we fork, to prepare the parent for
   task_create, when the new child task will inherit our address space.  */
DEFINE_HOOK (_hurd_fork_prepare_hook, (void));

/* Things that want to be called when we are forking, with the above all
   locked.  They are passed the task port of the child.  The child process
   is all set up except for doing proc_child, and has no threads yet.  */
DEFINE_HOOK (_hurd_fork_setup_hook, (void));

/* Things to be run in the child fork.  */
DEFINE_HOOK (_hurd_fork_child_hook, (void));

/* Things to be run in the parent fork.  */
DEFINE_HOOK (_hurd_fork_parent_hook, (void));


/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
pid_t
__fork (void)
{
  jmp_buf env;
  pid_t pid;
  size_t i;
  error_t err;
  thread_t thread_self = __mach_thread_self ();
  struct hurd_sigstate *ss;

  __mutex_lock (&_hurd_siglock);
  for (ss = _hurd_sigstates; ss != NULL; ss = ss->next)
    if (ss->thread == thread_self)
      break;
  if (ss)
    ss->critical_section = 1;

  if (! setjmp (env))
    {
      process_t newproc;
      task_t newtask;
      mach_port_t ports[_hurd_nports];
      thread_t thread, sigthread;
      struct machine_thread_state state;
      unsigned int statecount;
      mach_port_t *portnames = NULL;
      unsigned int nportnames = 0;
      mach_port_type_t *porttypes = NULL;
      unsigned int nporttypes = 0;
      thread_t *threads = NULL;
      unsigned int nthreads = 0;

      /* Run things that prepare for forking before we create the task.  */
      RUN_HOOKS (_hurd_fork_prepare_hook, ());

      /* Lock things that want to be locked before we fork.  */
      for (i = 0; i < _hurd_fork_locks.n; ++i)
	__mutex_lock (_hurd_fork_locks.locks[i]);
      
      newtask = MACH_PORT_NULL;
      thread = sigthread = MACH_PORT_NULL;
      newproc = MACH_PORT_NULL;

      /* Lock all the port cells for the standard ports while we copy the
	 address space.  We want to insert all the send rights into the
	 child with the same names.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_lock (&_hurd_ports[i].lock);

      /* Create the child task.  It will inherit a copy of our memory.  */
      if (err = __task_create (__mach_task_self (), 1, &newtask))
	goto lose;

      /* Fetch the names of all ports used in this task.  */
      if (err = __mach_port_names (__mach_task_self (),
				   &portnames, &nportnames,
				   &porttypes, &nporttypes))
	goto lose;
      if (nportnames != nporttypes)
	{
	  err = EGRATUITOUS;
	  goto lose;
	}

      /* Get send rights for all the threads in this task.
	 We want to avoid giving these rights to the child.  */
      if (err = __task_threads (__mach_task_self (), &threads, &nthreads))
	goto lose;

      /* Create the child main user thread and signal thread.  */
      if ((err = __thread_create (newtask, &thread))
	  (err = __thread_create (newtask, &sigthread)))
	goto lose;

      /* Get the child process's proc server port.  We will insert it into
	 the child with the same name as we use for our own proc server
	 port; and we will need it to set the child's message port.  */
      if (err = __proc_task2proc (_hurd_ports[INIT_PORT_PROC].port,
				  newtask, &newproc))
	goto lose;

      /* Insert all our port rights into the child task.  */
      for (i = 0; i < nportnames; ++i)
	{
	  if (MACH_PORT_TYPE (porttypes[i]) & MACH_PORT_TYPE_RECEIVE)
	    {
	      /* This is a receive right.  We want to give the child task
		 its own new receive right under the same name.  */
	      if (err = __mach_port_allocate_name (newtask,
						   MACH_PORT_RIGHT_RECEIVE,
						   portnames[i]))
		goto lose;
	      if (MACH_PORT_TYPE (porttypes[i]) & MACH_PORT_TYPE_SEND)
		{
		  /* Give the child as many send rights for its receive
		     right as we have for ours.  */
		  mach_port_urefs_t refs;
		  mach_port_t port;
		  mach_msg_type_name_t poly;
		  if (err = __mach_port_get_refs (__mach_task_self (),
						  portnames[i],
						  MACH_PORT_RIGHT_SEND,
						  &refs))
		    goto lose;
		  if (err = __mach_port_extract_right (newtask,
						       portnames[i],
						       MACH_MSG_TYPE_MAKE_SEND,
						       &port, &poly))
		    goto lose;
		  if (portnames[i] == _hurd_msgport)
		    {
		      /* We just created a receive right for the child's
			 message port and are about to insert send rights
			 for it.  Now, while we happen to have a send right
			 for it, give it to the proc server.  */
		      mach_port_t old;
		      if (err = __proc_setmsgport (newproc, port, &old))
			goto lose;
		      if (old != MACH_PORT_NULL)
			/* XXX what to do here? */
			__mach_port_deallocate (__mach_task_self (), old);
		    }
		  if (err = __mach_port_insert_right (newtask,
						      portnames[i],
						      port,
						      MACH_MSG_TYPE_MOVE_SEND))
		    goto lose;
		  if (refs > 1 &&
		      (err = __mach_port_mod_refs (newtask,
						   portnames[i],
						   MACH_PORT_RIGHT_SEND,
						   refs - 1)))
		    goto lose;
		}
	      if (MACH_PORT_TYPE (porttypes[i]) & MACH_PORT_TYPE_SEND_ONCE)
		{
		  /* Give the child a send-once right for its receive right,
		     since we have one for ours.  */
		  mach_port_t port;
		  mach_msg_type_name_t poly;
		  if (err = __mach_port_extract_right
		      (newtask,
		       portnames[i],
		       MACH_MSG_TYPE_MAKE_SEND_ONCE,
		       &port, &poly))
		    goto lose;
		  if (err = __mach_port_insert_right
		      (newtask,
		       portnames[i], port,
		       MACH_MSG_TYPE_MOVE_SEND_ONCE))
		    goto lose;
		}
	    }
	  else if (MACH_PORT_TYPE (porttypes[i]) &
		   (MACH_PORT_TYPE_SEND|MACH_PORT_TYPE_DEAD_NAME))
	    {
	      /* This is a send right or a dead name.
		 Give the child as many references for it as we have.  */
	      mach_port_urefs_t refs;
	      mach_port_t insert;
	      if (portnames[i] == newtask)
		/* Skip the name we use for the child's task port.  */
		continue;
	      if (portnames[i] == __mach_task_self ())
		/* For the name we use for our own task port,
		   insert the child's task port instead.  */
		insert = newtask;
	      else if (portnames[i] == _hurd_ports[INIT_PORT_PROC].port)
		{
		  /* Get the proc server port for the new task.  */
		  if (err = __USEPORT (PROC, __proc_task2proc (port, newtask,
							       &insert)))
		    goto lose;
		}
	      else if (portnames[i] == thread_self)
		/* For the name we use for our own thread port, insert the
		   thread port for the child main user thread.  We have one
		   extra user reference created at the beginning of this
		   function, accounted for by mach_port_names (and which
		   will thus be accounted for in the child below).  This
		   extra right gets consumed in the child by the store into
		   _hurd_sigthread in the child fork.  */
		insert = thread;
	      else if (portnames[i] == _hurd_msgport_thread)
		/* For the name we use for our signal thread's thread port,
		   insert the thread port for the child's signal thread.  */
		insert = sigthread;
	      else
		{
		  /* Skip the name we use for any of our own thread ports.  */
		  unsigned int j;
		  for (j = 0; j < nthreads; ++j)
		    if (portnames[i] == threads[j])
		      break;
		  if (j < nthreads)
		    continue;

		  insert = portnames[i];
		}
	      /* Find out how many user references we have for
		 the send right with this name.  */
	      if (err = __mach_port_get_refs (__mach_task_self (),
					      portnames[i],
					      MACH_PORT_RIGHT_SEND,
					      &refs))
		goto lose;
	      /* Insert the chosen send right into the child.  */
	      if (err = __mach_port_insert_right (newtask,
						  portnames[i],
						  insert,
						  MACH_MSG_TYPE_COPY_SEND))
		goto lose;
	      /* Give the child as many user references as we have.  */
	      if (refs > 1 &&
		  (err = __mach_port_mod_refs (newtask,
					       portnames[i],
					       MACH_PORT_RIGHT_SEND,
					       refs - 1)))
		goto lose;
	    }
	}

      /* Unlock the standard port cells.  The child must unlock its own
	 copies too.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);



      /* Register the child with the proc server.  */
      if (err = __USEPORT (PROC, __proc_child (port, newtask)))
	goto lose;

      /* Set the child signal thread up to run the msgport server function
	 using the same signal thread stack copied from our address space.
	 We fetch the state before longjmp'ing it so that miscellaneous
	 registers not affected by longjmp (such as i386 segment registers)
	 are in their normal default state.  */
      statecount = MACHINE_THREAD_STATE_COUNT;
      if (err = __thread_get_state (thread, MACHINE_THREAD_STATE_FLAVOR,
				    (int *) &state, &statecount))
	goto lose;
#if STACK_GROWTH_UP
      state.SP = __hurd_sigthread_stack_base;
#else
      state.SP = __hurd_sigthread_stack_end;
#endif      
      state.PC = (unsigned long int) _hurd_msgport_receive;
      if (err = __thread_set_state (sigthread, MACHINE_THREAD_STATE_FLAVOR,
				    (int *) &state, &statecount))
	goto lose;
      /* We do not thread_resume SIGTHREAD here because the child
	 fork needs to do more setup before it can take signals.  */

      /* Set the child user thread up to return 1 from the setjmp above.  */
      if (err = __thread_get_state (thread, MACHINE_THREAD_STATE_FLAVOR,
				    (int *) &state, &statecount))
	goto lose;
      _hurd_longjmp_thread_state (&state, env, 1);
      if (err = __thread_resume (thread))
	goto lose;

      /* Get the PID of the child from the proc server.  */
      err = __USEPORT (PROC, __proc_task2pid (port, newtask, &pid));

    lose:

      if (newtask != MACH_PORT_NULL)
	{
	  if (err)
	    __task_terminate (newtask);
	  __mach_port_deallocate (__mach_task_self (), newtask);
	}
      if (thread != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), thread);
      if (sigthread != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), sigthread);
      if (newproc != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), newproc);
      if (thread_self != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), thread_self);

      if (portnames)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) portnames,
			 nportnames * sizeof (*portnames));
      if (porttypes)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) porttypes,
			 nporttypes * sizeof (*porttypes));
      if (threads)
	{
	  for (i = 0; i < nthreads; ++i)
	    __mach_port_deallocate (__mach_task_self (), threads[i]);
	  __vm_deallocate (__mach_task_self (),
			   (vm_address_t) threads,
			   nthreads * sizeof (*threads));
	}

      /* Run things that want to run in the parent to restore it to
	 normality.  Usually prepare hooks and parent hooks are
	 symmetrical: the prepare hook arrests state in some way for the
	 fork, and the parent hook restores the state for the parent to
	 continue executing normally.  */
      if (err)
	/* We already have an error.  We still want to run these hooks, to
	   undo whatever the prepare hooks might have done, but ignore
	   these errors.  */
	run_hooks (&_hurd_fork_parent_hook, MACH_PORT_NULL, MACH_PORT_NULL);
      else
	err = run_hooks (&_hurd_fork_parent_hook,
			 MACH_PORT_NULL, MACH_PORT_NULL);
    }
  else
    {
      /* We are the child task.  Unlock the standard port cells, which were
         locked in the parent when we copied its memory.  The parent has
         inserted send rights with the names that were in the cells then.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);

      /* We are the only thread in this new task, so we will
	 take the task-global signals.  */
      _hurd_sigthread = thread_self;

      /* Free the sigstate structures for threads that existed in the
	 parent task but don't exist in this task (the child process).  */
      while (_hurd_sigstates != NULL)
	{
	  struct hurd_sigstate *next = _hurd_sigstates->next;
	  if (ss != _hurd_sigstates)
	    free (_hurd_sigstates);
	  _hurd_sigstates = next;
	}
      _hurd_sigstates = ss;
      _hurd_sigstates->next = NULL;

      /* Fetch our various new process IDs from the proc server.  */
      if (!err)
	err = __USEPORT (PROC, __proc_getpids (port, &_hurd_pid, &_hurd_ppid,
					       &_hurd_orphaned));
      if (!err)
	err = __USEPORT (PROC, __proc_getpgrp (port, _hurd_pid, &_hurd_pgrp));

      /* Run things that want to run in the child task to set up.  */
      RUN_HOOKS (_hurd_fork_child_hook, ());

      /* Set up proc server-assisted fault recovery for the signal thread.  */
      _hurdsig_fault_init ();

      /* Start the signal thread listening on the message port.  */
      if (!err)
	err = __thread_resume (_hurd_msgport_thread);

      pid = 0;
    }

  /* Unlock things we locked before creating the child task.
     They are locked in both the parent and child tasks.  */
  for (i = 0; i < _hurd_fork_locks.n; ++i)
    __mutex_unlock (_hurd_fork_locks.locks[i]);

  if (ss)
    ss->critical_section = 0;
  __mutex_unlock (&_hurd_siglock);

  return err ? __hurd_fail (err) : pid;
}
