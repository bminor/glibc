/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <sysdep.h>

/* The first piece of initialized data.  */
int __data_start = 0;

struct _hurd_port *_hurd_ports;
mode_t _hurd_umask;

mach_port_t *_hurd_init_dtable;
size_t _hurd_init_dtablesize;

volatile int errno;		/* XXX wants to be per-thread */

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif
char **__environ;

extern void __mach_init (void);
extern void __libc_init (int argc, char **argv, char **envp);
extern int main (int argc, char **argv, char **envp);

extern void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
extern void (*_cthread_exit_routine) (int status);

static int split_args (char *, size_t, char **);

/* Entry point.  The exec server started the initial thread in our task with
   this spot the PC, and a stack that is presumably big enough.
   The stack base address (|sp - this| is the size of the stack) is
   in the return-value register (%eax for i386, etc.).  */
void
_start (void)
{
  error_t err;
  mach_port_t in_bootstrap;
  char *args, *env;
  mach_port_t *portarray;
  int *intarray;
  size_t argslen, envlen, portarraysize, intarraysize;
  int flags;

  int argc, envc;
  char **argv, **envp;

  /* GET_STACK (LOW, HIGH) should put the boundaries of the allocated
     stack into LOW and HIGH.  */

#ifndef	GET_STACK
#error GET_STACK not defined by sysdeps/hurd/MACHINE/sysdep.h
#endif
  GET_STACK (_hurd_stack_low, _hurd_stack_high);

  /* Basic Mach initialization, must be done before RPCs can be done.  */
  __mach_init ();

  if (_cthread_init_routine != NULL)
    {
      /* Do cthreads initialization, and move to using its new stack.  */
      void *newsp = (*_cthread_init_routine) ();

      /* SET_SP (ADDR) should set the stack pointer to ADDR.  */
#ifndef	SET_SP
#error SET_SP not defined by sysdeps/mach/MACHINE/sysdep.h
#endif
      SET_SP (newsp);

      if (newsp < _hurd_stack_low || newsp > _hurd_stack_high)
	/* The new stack pointer does not intersect with the
	   stack the exec server set up for us, so free that stack.  */
	__vm_deallocate (__mach_task_self (),
			 _hurd_stack_low,
			 _hurd_stack_high - _hurd_stack_low);
    }

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    /* LOSE can be defined as the `halt' instruction or something
       similar which will cause the process to die in a characteristic
       way suggesting a bug.  */
#ifndef LOSE
#define	LOSE	__task_terminate (__mach_task_self ())
#endif
    LOSE;

  if (in_bootstrap != MACH_PORT_NULL)
    {
      /* Call the exec server on our bootstrap port and
	 get all our standard information from it.  */
      err = __exec_startup (in_bootstrap,
			    &flags,
			    &args, &argslen, &env, &envlen,
			    &_hurd_init_dtable, &_hurd_init_dtablesize,
			    &portarray, &portarraysize,
			    &intarray, &intarraysize);
      __mach_port_deallocate (__mach_task_self (), in_bootstrap);
    }

  if (err || in_bootstrap == MACH_PORT_NULL)
    {
      /* Either we have no bootstrap port, or the RPC to the exec server
	 failed.  Try to snarf the args in the canonical Mach way.
	 Hopefully either they will be on the stack as expected, or the
	 stack will be zeros so we don't crash.  Set all our other
	 variables to have empty information.  */

      /* SNARF_ARGS (ARGC, ARGV, ENVP) snarfs the arguments and environment
	 from the stack, assuming they were put there by the microkernel.  */
#ifndef SNARF_ARGS
#error SNARF_ARGS not defined by sysdeps/mach/MACHINE/sysdep.h
#endif
      SNARF_ARGS (argc, argv, envp);

      flags = 0;
      args = env = NULL;
      argslen = envlen = 0;
      _hurd_init_dtable = NULL;
      _hurd_init_dtablesize = 0;
      portarray = NULL;
      portarraysize = 0;
      intarray = NULL;
      intarraysize = 0;
    }
  else
    argv = envp = NULL;

  /* Turn the block of null-separated strings we were passed for the
     arguments and environment into vectors of pointers to strings.  */
      
  if (! argv)
    {
      if (! args)
	{
	  /* No arguments passed; set argv to { NULL }.  */
	  argc = 0;
	  argv = (char **) &args;
	}
      else
	argc = split_args (args, argslen, NULL) + 1;
    }

  if (! envp)
    {
      if (! env)
	{
	  /* No environment passed; set __environ to { NULL }.  */
	  envc = 0;
	  envp = (char **) &env;
	}
      else
	envc = split_args (env, envlen, NULL) + 1;
    }

  if (! argv && ! envp && argc + envc > 0)
    {
      /* There were some arguments or environment.
	 Allocate space for the vectors of pointers and fill them in.  */

      if (err = __vm_allocate (__mach_task_self (),
			       &argv, round_page ((argc + envc) *
						  sizeof (char *)),
			       1))
	__libc_fatal ("hurd: Can't allocate space for argv and environ\n");
      envp = &argv[argc + 1];
      
      split_args (args, argslen, argv);
      split_args (env, envlen, envp);
    }

  if (portarray || intarray)
    /* Initialize library data structures, start signal processing, etc.  */
    _hurd_init (argv,
		portarray, portarraysize,
		intarray, intarraysize);


  /* Random library initialization.  */
  __libc_init (argc, argv, __environ);


  /* Finally, run the user program.  */
  (_cthread_exit_routine != NULL ? *_cthread_exit_routine : exit)
    (main (argc, argv, __environ));
}

static int
split_args (char *args, size_t argslen, char **argv)
{
  char *p = args;
  size_t n = argslen;
  int argc = 0;

  while (n > 0)
    {
      char *end = memchr (p, '\0', n);

      if (argv)
	argv[argc] = p;
      ++argc;

      if (end == NULL)
	/* The last argument is unterminated.  */
	break;

      n -= end + 1 - p;
      p = end + 1;
    }

  if (argv)
    argv[argc] = NULL;
  return argc;
}
