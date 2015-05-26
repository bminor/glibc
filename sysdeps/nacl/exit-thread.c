/* Call to terminate the current thread.  NaCl version.
   Copyright (C) 2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   <http://www.gnu.org/licenses/>.  */

#include <exit-thread.h>
#include <lowlevellock.h>
#include <nacl-interfaces.h>
#include <pthread-functions.h>
#include <stdbool.h>
#include <sysdep.h>
#include <tls.h>
#include <atomic.h>


/* A sysdeps/CPU/nacl/exit-thread.c file defines this function
   and then #include's this file.  */
static void call_on_stack (void *sp,
			   void (*func) (void *)
			     internal_function __attribute__ ((noreturn)),
			   void *arg)
  __attribute__ ((noreturn));


/* This is a trivial wrapper around the OS call to terminate the
   calling thread.  It just sugar to have a paranoidly definitely
   noreturn function.  */
static void __attribute__ ((noreturn))
do_thread_exit (volatile int32_t *stack_flag)
{
  __nacl_irt_thread.thread_exit ((int32_t *) stack_flag);

  /* That never returns unless something is severely and unrecoverably
     wrong.  If it ever does, try to make sure we crash.  */
  while (1)
    __builtin_trap ();
}


/* The complexity implemented here is only necessary when there are
   actually multiple threads in the program (that is, any other threads
   that will still exist when this one finishes exiting).  So detect
   that so as to short-circuit the complexity when it's superfluous.  */
static bool
multiple_threads (void)
{
#ifdef SHARED
  unsigned int *ptr_nthreads = __libc_pthread_functions.ptr_nthreads;
  PTR_DEMANGLE (ptr_nthreads);
#else
  extern unsigned int __nptl_nthreads __attribute ((weak));
  unsigned int *const ptr_nthreads = &__nptl_nthreads;
  if (ptr_nthreads == NULL)
    return false;
#endif

  /* We are called after nthreads has been decremented for the current
     thread.  So it's the count of threads other than this one.  */
  return *ptr_nthreads > 0;
}


#define EXIT_STACK_SIZE 256

struct exit_stack
{
  char stack[EXIT_STACK_SIZE] __attribute__ ((aligned (16)));
  volatile int32_t flag;
};
typedef struct exit_stack exit_stack_t;

static exit_stack_t initial_exit_stack;
static int exit_stack_lock = LLL_LOCK_INITIALIZER;

static exit_stack_t *
get_exit_stack (void)
{
  exit_stack_t *result;

  lll_lock (exit_stack_lock, LLL_PRIVATE);

  result = &initial_exit_stack;

  while (atomic_load_relaxed (&result->flag) != 0)
    {
      lll_unlock (exit_stack_lock, LLL_PRIVATE);
      __nacl_irt_basic.sched_yield ();
      lll_lock (exit_stack_lock, LLL_PRIVATE);
    }

  atomic_store_relaxed (&result->flag, 1);

  lll_unlock (exit_stack_lock, LLL_PRIVATE);

  return result;
}


/* This is called on the exit stack that is passed as the argument.
   Its mandate is to clear and futex-wake THREAD_SELF->tid and then
   exit, eventually releasing the exit stack for reuse.  */
static void internal_function __attribute__ ((noreturn))
exit_on_exit_stack (void *arg)
{
  exit_stack_t *stack = arg;
  struct pthread *pd = THREAD_SELF;

  /* Mark the thread as having exited and wake anybody waiting for it.
     After this, both PD itself and its real stack will be reclaimed
     and it's not safe to touch or examine them.  */
  pd->tid = 0;
  lll_futex_wake (&pd->tid, 1, LLL_PRIVATE);

  /* Now we can exit for real, and get off this exit stack.  The system
     will clear the flag some time after the exit stack is guaranteed not
     to be in use.  */
  do_thread_exit (&stack->flag);
}

static void __attribute__ ((noreturn))
exit_off_stack (void)
{
  exit_stack_t *stack = get_exit_stack ();
  void *sp = stack + 1;

  /* This switches onto the exit stack and then performs:
	exit_on_exit_stack (stack);
     How to accomplish that is, of course, machine-dependent.  */
  call_on_stack (sp, &exit_on_exit_stack, stack);
}

void
__exit_thread (bool detached)
{
  if (detached || !multiple_threads ())
    /* There is no other thread that cares when we exit, so life is simple.
       In the DETACHED case, we're not allowed to look at THREAD_SELF any
       more, so avoiding the complex path is not just an optimization.  */
    do_thread_exit (NULL);
  else
    /* We must exit in a way that wakes up pthread_join,
       i.e. clears and futex-wakes THREAD_SELF->tid.  */
    exit_off_stack ();
}
