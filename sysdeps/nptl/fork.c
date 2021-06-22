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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sysdep.h>
#include <libio/libioP.h>
#include <tls.h>
#include <hp-timing.h>
#include <ldsodefs.h>
#include <stdio-lock.h>
#include <atomic.h>
#include <pthreadP.h>
#include <register-atfork.h>
#include <arch-fork.h>
#include <futex-internal.h>
#include <malloc/malloc-internal.h>
#include <nss/nss_database.h>
#include <unwind-link.h>
#include <sys/single_threaded.h>
#include <list.h>

static void
fresetlockfiles (void)
{
  _IO_ITER i;

  for (i = _IO_iter_begin(); i != _IO_iter_end(); i = _IO_iter_next(i))
    if ((_IO_iter_file (i)->_flags & _IO_USER_LOCK) == 0)
      _IO_lock_init (*((_IO_lock_t *) _IO_iter_file(i)->_lock));
}

/* In case of a fork() call the memory allocation in the child will be
   the same but only one thread is running.  All stacks except that of
   the one running thread are not used anymore.  We have to recycle
   them.  */
static void
reclaim_stacks (void)
{
  struct pthread *self = (struct pthread *) THREAD_SELF;

  /* No locking necessary.  The caller is the only stack in use.  But
     we have to be aware that we might have interrupted a list
     operation.  */

  if (GL (dl_in_flight_stack) != 0)
    {
      bool add_p = GL (dl_in_flight_stack) & 1;
      list_t *elem = (list_t *) (GL (dl_in_flight_stack) & ~(uintptr_t) 1);

      if (add_p)
	{
	  /* We always add at the beginning of the list.  So in this case we
	     only need to check the beginning of these lists to see if the
	     pointers at the head of the list are inconsistent.  */
	  list_t *l = NULL;

	  if (GL (dl_stack_used).next->prev != &GL (dl_stack_used))
	    l = &GL (dl_stack_used);
	  else if (GL (dl_stack_cache).next->prev != &GL (dl_stack_cache))
	    l = &GL (dl_stack_cache);

	  if (l != NULL)
	    {
	      assert (l->next->prev == elem);
	      elem->next = l->next;
	      elem->prev = l;
	      l->next = elem;
	    }
	}
      else
	{
	  /* We can simply always replay the delete operation.  */
	  elem->next->prev = elem->prev;
	  elem->prev->next = elem->next;
	}

      GL (dl_in_flight_stack) = 0;
    }

  /* Mark all stacks except the still running one as free.  */
  list_t *runp;
  list_for_each (runp, &GL (dl_stack_used))
    {
      struct pthread *curp = list_entry (runp, struct pthread, list);
      if (curp != self)
	{
	  /* This marks the stack as free.  */
	  curp->tid = 0;

	  /* Account for the size of the stack.  */
	  GL (dl_stack_cache_actsize) += curp->stackblock_size;

	  if (curp->specific_used)
	    {
	      /* Clear the thread-specific data.  */
	      memset (curp->specific_1stblock, '\0',
		      sizeof (curp->specific_1stblock));

	      curp->specific_used = false;

	      for (size_t cnt = 1; cnt < PTHREAD_KEY_1STLEVEL_SIZE; ++cnt)
		if (curp->specific[cnt] != NULL)
		  {
		    memset (curp->specific[cnt], '\0',
			    sizeof (curp->specific_1stblock));

		    /* We have allocated the block which we do not
		       free here so re-set the bit.  */
		    curp->specific_used = true;
		  }
	    }
	}
    }

  /* Add the stack of all running threads to the cache.  */
  list_splice (&GL (dl_stack_used), &GL (dl_stack_cache));

  /* Remove the entry for the current thread to from the cache list
     and add it to the list of running threads.  Which of the two
     lists is decided by the user_stack flag.  */
  list_del (&self->list);

  /* Re-initialize the lists for all the threads.  */
  INIT_LIST_HEAD (&GL (dl_stack_used));
  INIT_LIST_HEAD (&GL (dl_stack_user));

  if (__glibc_unlikely (THREAD_GETMEM (self, user_stack)))
    list_add (&self->list, &GL (dl_stack_user));
  else
    list_add (&self->list, &GL (dl_stack_used));
}

pid_t
__libc_fork (void)
{
  pid_t pid;

  /* Determine if we are running multiple threads.  We skip some fork
     handlers in the single-thread case, to make fork safer to use in
     signal handlers.  POSIX requires that fork is async-signal-safe,
     but our current fork implementation is not.  */
  bool multiple_threads = __libc_single_threaded == 0;

  __run_fork_handlers (atfork_run_prepare, multiple_threads);

  struct nss_database_data nss_database_data;

  /* If we are not running multiple threads, we do not have to
     preserve lock state.  If fork runs from a signal handler, only
     async-signal-safe functions can be used in the child.  These data
     structures are only used by unsafe functions, so their state does
     not matter if fork was called from a signal handler.  */
  if (multiple_threads)
    {
      call_function_static_weak (__nss_database_fork_prepare_parent,
				 &nss_database_data);

      _IO_list_lock ();

      /* Acquire malloc locks.  This needs to come last because fork
	 handlers may use malloc, and the libio list lock has an
	 indirect malloc dependency as well (via the getdelim
	 function).  */
      call_function_static_weak (__malloc_fork_lock_parent);
    }

  pid = arch_fork (&THREAD_SELF->tid);

  if (pid == 0)
    {
      struct pthread *self = THREAD_SELF;

      /* See __pthread_once.  */
      __fork_generation += __PTHREAD_ONCE_FORK_GEN_INCR;

      /* Initialize the robust mutex list setting in the kernel which has
	 been reset during the fork.  We do not check for errors because if
	 it fails here, it must have failed at process startup as well and
	 nobody could have used robust mutexes.
	 Before we do that, we have to clear the list of robust mutexes
	 because we do not inherit ownership of mutexes from the parent.
	 We do not have to set self->robust_head.futex_offset since we do
	 inherit the correct value from the parent.  We do not need to clear
	 the pending operation because it must have been zero when fork was
	 called.  */
#if __PTHREAD_MUTEX_HAVE_PREV
      self->robust_prev = &self->robust_head;
#endif
      self->robust_head.list = &self->robust_head;
      INTERNAL_SYSCALL_CALL (set_robust_list, &self->robust_head,
			     sizeof (struct robust_list_head));

      /* Reset the lock state in the multi-threaded case.  */
      if (multiple_threads)
	{
	  __libc_unwind_link_after_fork ();

	  /* There is one thread running.  */
	  __nptl_nthreads = 1;

	  /* Initialize thread library locks.  */
	  GL (dl_stack_cache_lock) = LLL_LOCK_INITIALIZER;
	  __default_pthread_attr_lock = LLL_LOCK_INITIALIZER;

	  /* Release malloc locks.  */
	  call_function_static_weak (__malloc_fork_unlock_child);

	  /* Reset the file list.  These are recursive mutexes.  */
	  fresetlockfiles ();

	  /* Reset locks in the I/O code.  */
	  _IO_list_resetlock ();

	  call_function_static_weak (__nss_database_fork_subprocess,
				     &nss_database_data);
	}

      /* Reset the lock the dynamic loader uses to protect its data.  */
      __rtld_lock_initialize (GL(dl_load_lock));

      reclaim_stacks ();

      /* Run the handlers registered for the child.  */
      __run_fork_handlers (atfork_run_child, multiple_threads);
    }
  else
    {
      /* Release acquired locks in the multi-threaded case.  */
      if (multiple_threads)
	{
	  /* Release malloc locks, parent process variant.  */
	  call_function_static_weak (__malloc_fork_unlock_parent);

	  /* We execute this even if the 'fork' call failed.  */
	  _IO_list_unlock ();
	}

      /* Run the handlers registered for the parent.  */
      __run_fork_handlers (atfork_run_parent, multiple_threads);
    }

  return pid;
}
weak_alias (__libc_fork, __fork)
libc_hidden_def (__fork)
weak_alias (__libc_fork, fork)
