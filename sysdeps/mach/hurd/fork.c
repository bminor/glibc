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

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <hurd.h>

const struct
  {
    size_t n;
    error_t (*fn[0]) (task_t);
  } _hurd_fork_hook;

/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
int
DEFUN_VOID(__fork)
{
  jmp_buf env;

  if (setjmp (env))
    /* Child fork.  Return zero.  */
    return (pid_t) 0;
  else
    {
      error_t err;
      process_t newproc;
      task_t newtask;
      mach_port_t sigport;
      thread_t *threads;
      size_t nthreads;
      char state[_hurd_thread_state_count];
      pid_t child;

      __mutex_lock (&_hurd_lock);

      newtask = MACH_PORT_NULL;
      sigport = MACH_PORT_NULL;
      newproc = MACH_PORT_NULL;

      if (err = __task_create (__mach_task_self (), 1, &newtask))
	goto lose;
      if (err = __mach_port_allocate (__mach_task_self (),
				      MACH_PORT_RIGHT_RECEIVE, &sigport))
	goto lose;

      if (err = __proc_register (_hurd_proc, newtask, sigport, &newproc))
	goto lose;

      if (err = __mach_port_insert_right (newtask, _hurd_sigport,
					  sigport, MACH_PORT_MOVE_RECEIVE))
	goto lose;

      if (err = __mach_port_insert_right (newtask, _hurd_proc, newproc,
					  MACH_PORT_MOVE_SEND)) 
	goto lose;

#define	cpysnd(port)							      \
      if (err = __mach_port_insert_right (newtask, (port), (port),	      \
					  MACH_PORT_COPY_SEND))		      \
	goto lose; else

      cpysnd (_hurd_ccdir);
      cpysnd (_hurd_crdir);
      cpysnd (_hurd_cwdir);

      __mutex_unlock (&_hurd_lock);

      for (i = 0; i < _hurd_fork_hook.n; ++i)
	if (err = (*_hurd_fork_hook.fn[i]) (newtask))
	  goto lose;

      __proc_dostop (_hurd_proc, __mach_thread_self ());

      if (err = __task_threads (__mach_task_self (), &threads, &nthreads))
	{
	  /* This is majorly bad.  We've stopped all other threads and now
	     have no way to start them up.  We can at least try to resume
	     the signal thread.  */
	  __thread_resume (_hurd_sigport_thread);
	  goto lose;
	}

      /* Suspend the task so we can resume the threads as we create them,
	 but have them all start at once at the end.  */
      __task_suspend (newtask);
      while (nthreads-- > 0)
	{
	  thread_t = *threads++;
	  thread_t new = MACH_PORT_NULL;

	  if (!err)
	    err = __thread_create (newtask, &new);
	  if (!err && t == __mach_thread_self ())
	    /* Make the thread in the new task do a longjmp to ENV.  */
	    err = _hurd_thread_longjmp (new, env, 1);
	  if (!err && t == _hurd_sigport_thread)
	    /* Give the thread port for the sigport thread the
	       same name in the child, so its copy of _hurd_sigport_thread
	       will be right.  */
	    err = __mach_port_insert_right (newtask, t, new,
					    MACH_PORT_COPY_SEND);

	  if (!err)
	    err = _hurd_thread_state (t, state);
	  __thread_resume (t);
	  if (!err)
	    {
	      err = _hurd_thread_set_state (new, state);
	      __thread_resume (new);
	    }
	  __mach_port_deallocate (__mach_task_self (), t);
	  if (new != MACH_PORT_NULL)
	    __mach_port_deallocate (__mach_task_self (), new);
	}
      if (!err)
	/* Start the task up.  */
	err = __task_resume (newtask);

      if (!err)
	err = __proc_task2pid (_hurd_proc, newtask, &child);

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
	return __hurd_fail (err);
      return child;
    }
}
