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

#ifdef notevenclose

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <hurd.h>


/* Things that want to be locked while forking.  */
const struct
  {
    size_t n;
    mutex_t locks[0];
  } _hurd_fork_locks;

/* Things that want to be called when we have forked,
   with the above all locked.  */
const struct
  {
    size_t n;
    error_t (*fn[0]) (task_t);
  } _hurd_fork_hook;


/* Resume all other threads in the current task.  */
static inline void
start_threads (void)
{
  thread_t *threads, me = __mach_thread_self ();
  size_t nthreads, i;

  __task_threads (__mach_task_self (), &threads, &nthreads);

  for (i = 0; i < nthreads; ++i)
    {
      if (threads[i] != me)
	__thread_resume (threads[i]);
      __mach_port_deallocate (__mach_task_self (), threads[i]);
    }

  __vm_deallocate (__mach_task_self (), threads, nthreads * sizeof (*threads));
}

/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
pid_t
DEFUN_VOID(__fork)
{
  jmp_buf env;
  pid_t pid;

  if (! setjmp (env))
    {
      error_t err;
      process_t newproc;
      task_t newtask;
      mach_port_t sigport;
      thread_t *threads;
      size_t nthreads;
      char state[_hurd_thread_state_count];

      mach_port_t proc, auth, crdir, cwdir, ccdir;

      /* Lock things that want to be locked before we fork.  */

      mutex_t lockp;
      for (lockp = _hurd_fork_locks.locks; *lockp != NULL; ++lockp)
	__mutex_lock (*lockp);

      __spin_lock (&_hurd_proc.lock);
      __spin_lock (&_hurd_auth.lock);
      __spin_lock (&_hurd_crdir.lock);
      __spin_lock (&_hurd_cwdir.lock);
      __spin_lock (&_hurd_ccdir.lock);

      newtask = MACH_PORT_NULL;
      sigport = MACH_PORT_NULL;
      newproc = MACH_PORT_NULL;

      err = __task_create (__mach_task_self (), 1, &newtask);

      /* Save the names of the ports we will give to the child.  */
      proc = _hurd_proc.port;
      auth = _hurd_auth.port;
      crdir = _hurd_crdir.port;
      cwdir = _hurd_cwdir.port;
      ccdir = _hurd_ccdir.port;

      __spin_unlock (&_hurd_proc.lock);
      __spin_unlock (&_hurd_auth.lock);
      __spin_unlock (&_hurd_crdir.lock);
      __spin_unlock (&_hurd_cwdir.lock);
      __spin_unlock (&_hurd_ccdir.lock);

      if (err)
	goto lose;

      /* Suspend the task so we can resume the threads as we create them,
	 but have them all start at once at the end.  */
      if (err = __task_suspend (newtask))
	goto lose;

      /* Stop all other threads while we create the new task and extract
	 the port names its address space is using.  Otherwise another
	 thread could run just after we create the new task, and change the
	 port cells before we extract the ports from them.  Then we would
	 insert the ports into the new task with names other than the ones
	 it is expecting.

	 We keep _hurd_proc.lock held while calling the proc server to
	 avoid the deadlock situation which could result if another thread
	 were to take the lock and then be suspended.  */
	 
      __spin_lock (&_hurd_proc.lock);
      err = __proc_dostop (_hurd_proc.port, __mach_thread_self ());
      __spin_unlock (&_hurd_proc.lock);
      if (err)
	goto lose;

      if (err = __task_threads (__mach_task_self (), &threads, &nthreads))
	{
	  /* This is majorly bad.  We've stopped all other threads and now
	     have no way to start them up.  We can at least try to resume
	     the signal thread.  */
	  __thread_resume (_hurd_sigport_thread);
	  /* XXX ? */
	  goto lose;
	}

      /* Create a thread in the child task for each thread in the parent,
	 and copy the thread states.  The exception is the thread running
	 this code.  Its state is not copied to the child; instead,
	 the corresponding thread in the child returns from the `setjmp'
	 call above.  */
      for (i = 0; i < nthreads; ++i)
	{
	  const thread_t t = threads[i];
	  thread_t new = MACH_PORT_NULL;

	  if (err = __thread_create (newtask, &new))
	    break;

	  if (t == me)
	    /* Make the thread in the new task do a longjmp to ENV.  */
	    err = _hurd_thread_longjmp (new, env, 1);
	  else
	    {
	      /* Copy the thread state from parent to child,
		 and resume the thread in the parent task.  */
	      err = _hurd_thread_state (t, state);
	      if (err)
		(void) __thread_resume (t);
	      else
		{
		  err = __thread_resume (t);
		  if (err)
		    (void) _hurd_thread_set_state (new, state);
		  else
		    err = _hurd_thread_set_state (new, state);
		  if (err)
		    (void) __thread_resume (new);
		  else
		    err = __thread_resume (new);
		}
	    }

	  if (!err)
	    /* Give the thread port the same name in the child.  */
	    err = __mach_port_insert_right (newtask, t, new,
					    MACH_PORT_MOVE_SEND);
	  if (new != MACH_PORT_NULL)
	    __mach_port_deallocate (__mach_task_self (), new);

	  if (err)
	    break;
	}
      /* If we got an error in one of the threads,
	 resume and deallocate the remaining thread ports.  */
      while (i < nthreads)
	{
	  __thread_resume (threads[i]);
	  __mach_port_deallocate (__mach_task_self (), threads[i]);
	  ++i;
	}
      __vm_deallocate (__mach_task_self (),
		       threads, nthreads * sizeof (*threads));

      /* Create the message port to be used by the child.  */
      if (err = __mach_port_allocate (__mach_task_self (),
				      MACH_PORT_RIGHT_RECEIVE, &sigport))
	goto lose;

      /* Register the child with the proc server.  */
      if (err = _HURD_PORT_USE (&_hurd_proc,
				__proc_register (port,
						 newtask, sigport, &newproc)))
	goto lose;


      /* Insert the ports used by the library into the child task.  */

      /* Give the child's message port the same name the parent's
	 message port has in the parent task.  The child's signal
	 thread will listen on SIGPORT.  */
      if (err = __mach_port_insert_right (newtask, _hurd_sigport, sigport,
					  MACH_PORT_MOVE_RECEIVE))
	goto lose;

      /* Give the child's proc port the same name as the parent's
	 proc port has in the parent task.  */
      if (err = __mach_port_insert_right (newtask, proc, newproc,
					  MACH_PORT_MOVE_SEND)) 
	goto lose;

      /* Copy the standard ports into the child task,
	 with the names that were the standard ports' names
	 in the parent task at the time the child task was created.  */
#define	cpysnd(copy)							      \
      if (err = _HURD_PORT_USE						      \
	  (&_hurd_##copy,						      \
	   __mach_port_insert_right (newtask,				      \
				     copy, port,			      \
				     MACH_PORT_COPY_SEND)))		      \
	goto lose; else

      cpysnd (auth);
      cpysnd (ccdir);
      cpysnd (crdir);
      cpysnd (cwdir);

      /* Run things that want to be run to set
	 other things up in the child task.  */
      for (i = 0; i < _hurd_fork_hook.n; ++i)
	if (err = (*_hurd_fork_hook.fn[i]) (newtask))
	  goto lose;

      if (!err)
	/* Start the child task up.  */
	err = __task_resume (newtask);

      if (!err)
	/* Get the PID of the child from the proc server.  */
	err = _HURD_PORT_USE (&_hurd_proc,
			      __proc_task2pid (port, newtask, &pid));

    lose:

      if (newtask != MACH_PORT_NULL)
	{
	  if (err)
	    __task_terminate (newtask);
	  __mach_port_deallocate (__mach_task_self (), newtask);
	}
      if (sigport != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), sigport);
      if (newproc != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), newproc);

      if (err)
	{
	  errno = err;
	  pid = -1;
	}
    }
  else
    /* We are the child task.  */
    pid = 0;

  /* Unlock things we locked before creating the child task.
     They are locked in both the parent and child tasks.  */
  for (lockp = _hurd_fork_locks.locks; *lockp != NULL; ++lockp)
    __mutex_unlock (*lockp);

  return pid;
}

#endif
