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
#include <stdlib.h>
#include <string.h>
#include <hurd.h>
#include <hurd/exec.h>
#include <sysdep.h>

/* The first piece of initialized data.  */
int __data_start = 0;

struct _hurd_port *_hurd_ports;
mode_t _hurd_umask;

mach_port_t *_hurd_init_dtable;
mach_msg_type_number_t _hurd_init_dtablesize;

vm_address_t _hurd_stack_base;
vm_size_t _hurd_stack_size;

volatile int errno;		/* XXX wants to be per-thread */

char **__environ;

extern void __mach_init (void);
extern void __libc_init (int argc, char **argv, char **envp);
extern int main (int argc, char **argv, char **envp);

void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
__NORETURN void (*_cthread_exit_routine) (int status);

static int split_args (char *, size_t, char **);


/* These communicate values from _start to start1,
   where we cannot use the stack for anything.  */
static char *args, *env;
static mach_port_t *portarray;
static int *intarray;
static mach_msg_type_number_t argslen, envlen, portarraysize, intarraysize;
static int flags;
static char **argv, **envp;
static int argc;


static volatile void
start1 (void)
{
  register int envc = 0;

  {
    /* Check if the stack we are now on is different from
       the one described by _hurd_stack_{base,size}.  */

    char dummy;
    const vm_address_t newsp = (vm_address_t) &dummy;

    if (_hurd_stack_size != 0 && (newsp < _hurd_stack_base ||
				  newsp - _hurd_stack_base > _hurd_stack_size))
      /* The new stack pointer does not intersect with the
	 stack the exec server set up for us, so free that stack.  */
      __vm_deallocate (__mach_task_self (),
		       _hurd_stack_base, _hurd_stack_size);
  }


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
	argc = split_args (args, argslen, NULL);
    }

  if (! envp)
    {
      if (! env)
	/* No environment passed; set __environ to { NULL }.  */
	envp = (char **) &env;
      else
	envc = split_args (env, envlen, NULL);
    }

  if (! argv && ! envp && argc + envc > 0)
    {
      /* There were some arguments or environment.
	 Allocate space for the vectors of pointers and fill them in.  */

      argv = __alloca ((argc + 1) * sizeof (char *));
      envp = __alloca ((envc + 1) * sizeof (char *));
      
      split_args (args, argslen, argv);
      split_args (env, envlen, envp);
    }

  if (portarray || intarray)
    /* Initialize library data structures, start signal processing, etc.  */
    _hurd_init (flags, argv, portarray, portarraysize, intarray, intarraysize);


  /* Random library initialization.  */
  __libc_init (argc, argv, __environ);


  /* Finally, run the user program.  */
  (_cthread_exit_routine != NULL ? *_cthread_exit_routine : exit)
    (main (argc, argv, __environ));

  /* Should never get here.  */
  LOSE;
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


/* Entry point.  The exec server started the initial thread in our task with
   this spot the PC, and a stack that is presumably big enough.  We do basic
   Mach initialization so mig-generated stubs work, and then do an exec_startup
   RPC on our bootstrap port, to which the exec server responds with the
   information passed in the exec call, as well as our original bootstrap port,
   and the base address and size of the preallocated stack.

   If using cthreads, we are given a new stack by cthreads initialization and
   deallocate the stack set up by the exec server.  On the new stack we call
   `start1' (above) to do the rest of the startup work.  Since the stack may
   disappear out from under us in a machine-dependent way, we use a pile of
   static variables to communicate the information from exec_startup to start1.
   This is unfortunate but preferable to machine-dependent frobnication to copy
   the state from the old stack to the new one.  */

void
_start (void)
{
  error_t err;
  mach_port_t in_bootstrap;
  vm_address_t stack_pointer, stack_base;
  vm_size_t stack_size;

  /* GET_SP (SP) should put the stack pointer in SP.  */

#ifndef	GET_SP
#error GET_SP not defined by sysdeps/mach/hurd/MACHINE/sysdep.h
#endif
  GET_SP (stack_pointer);

  /* Basic Mach initialization, must be done before RPCs can be done.  */
  __mach_init ();

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    LOSE;

  if (in_bootstrap != MACH_PORT_NULL)
    {
      /* Call the exec server on our bootstrap port and
	 get all our standard information from it.  */

      argslen = envlen = 0;
      _hurd_init_dtablesize = portarraysize = intarraysize = 0;

      err = __exec_startup (in_bootstrap,
			    &stack_base, &stack_size,
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


  /* Do cthreads initialization and switch to the cthread stack.  */

  if (_cthread_init_routine != NULL)
    CALL_WITH_SP (start1, (*_cthread_init_routine) ());
  else
    start1 ();

  /* Should never get here.  */
  LOSE;
}
