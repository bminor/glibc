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

extern void _hurd_longjmp_thread_state (struct machine_thread_state *,
					jmp_buf env, int value);


/* Things that want to be locked while forking.  */
const struct
  {
    size_t n;
    struct mutex *locks[0];
  } _hurd_fork_locks;

struct hook 
  {
    size_t n;
    error_t (*fn[0]) (task_t, process_t);
  };

static inline error_t
run_hooks (const struct hook *h, task_t t, process_t p)
{
  size_t i;
  error_t err;
  for (i = 0; i < h->n; ++i)
    if (err = (*h->fn[i]) (t, p))
      return err;
  return 0;
}

/* Things that want to be called when we have forked, with the above all
   locked.  They are passed the task port of the child.  */
const struct hook _hurd_fork_setup_hook;

/* Things to be run in the child fork.  */
const struct hook _hurd_fork_child_hook;


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

  if (! setjmp (env))
    {
      process_t newproc;
      task_t newtask;
      mach_port_t ports[_hurd_nports];
      thread_t thread;
      struct machine_thread_state state;

      /* Lock things that want to be locked before we fork.  */
      for (i = 0; i < _hurd_fork_locks.n; ++i)
	__mutex_lock (_hurd_fork_locks.locks[i]);

      newtask = MACH_PORT_NULL;
      thread = MACH_PORT_NULL;
      newproc = MACH_PORT_NULL;

      /* Lock all the port cells for the standard ports and extract the
         port names.  This ensures the ports won't change between the time
         we extract the names and the time we fork the new task (which
         inherits this address space).  We want to insert all the send
         rights into the child with the same names.  */
      for (i = 0; i < _hurd_nports; ++i)
	{
	  __spin_lock (&_hurd_ports[i].lock);
	  ports[i] = _hurd_ports[i].port;
	}

      /* Create the child task.  It will inherit a copy of our memory.  */
      if (err = __task_create (__mach_task_self (), 1, &newtask))
	goto lose;

      /* Unlock the standard port cells.  The child must unlock its own
	 copies too.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);

      /* Insert the ports used by the library into the child task.  */

      /* Copy the standard ports into the child task,
	 with the names that were the standard ports' names
	 in the parent task at the time the child task was created.  */
      for (i = 0; i < _hurd_nports; ++i)
	{
	  if (i == INIT_PORT_PROC)
	    {
	      /* Get the proc server port for the new task.  */
	      if (err = __USEPORT (PROC,
				   __proc_task2proc (port, newtask, &newproc)))
		goto lose;
	      err = __mach_port_insert_right (newtask, ports[i], newproc,
					      MACH_MSG_TYPE_COPY_SEND);
	    }
	  else
	    err = HURD_PORT_USE
	      (&_hurd_ports[i],
	       __mach_port_insert_right (newtask, ports[i], port,
					 MACH_MSG_TYPE_COPY_SEND));
	  if (err)
	    goto lose;
	}

      /* If we have a real dtable, the hooks will be taking care of it.
	 If not, copy the ports in the initial dtable.  */
      if (_hurd_init_dtable != NULL)
	for (i = 0; i < _hurd_init_dtablesize; ++i)
	  if (err = __mach_port_insert_right (newtask, _hurd_init_dtable[i],
					      _hurd_init_dtable[i],
					      MACH_MSG_TYPE_COPY_SEND))
	    goto lose;

      /* Run things to set other things up in the child task.  */
      if (err = run_hooks (&_hurd_fork_setup_hook, newtask, newproc))
	goto lose;

      /* Register the child with the proc server.  */
      if (err = __USEPORT (PROC, __proc_child (port, newtask)))
	goto lose;

      /* Create the child thread.  */
      if (err = __thread_create (newtask, &thread))
	goto lose;

      /* Set the child thread up to return 1 from the setjmp above.  */
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
      if (newproc != MACH_PORT_NULL)
	__mach_port_deallocate (__mach_task_self (), newproc);
    }
  else
    {
      /* We are the child task.  Unlock the standard port cells, which were
         locked in the parent when we copied its memory.  The parent has
         inserted send rights with the names that were in the cells then.  */
      for (i = 0; i < _hurd_nports; ++i)
	__spin_unlock (&_hurd_ports[i].lock);

      /* Run things that want to run in the child task to set up.  */
      err = run_hooks (&_hurd_fork_child_hook, MACH_PORT_NULL, MACH_PORT_NULL);

      pid = 0;
    }

  /* Unlock things we locked before creating the child task.
     They are locked in both the parent and child tasks.  */
  for (i = 0; i < _hurd_fork_locks.n; ++i)
    __mutex_unlock (_hurd_fork_locks.locks[i]);

  return err ? __hurd_fail (err) : pid;
}
