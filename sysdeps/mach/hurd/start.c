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

#include <ansidecl.h>
#include <errno.h>
#include <hurd.h>
#include <sysdep.h>

/* The first piece of initialized data.  */
int __data_start = 0;

struct _hurd_port _hurd_cwdir, _hurd_crdir;
mode_t _hurd_umask;
int _hurd_ctty_fstype;
fsid_t _hurd_ctty_fsid;
ino_t _hurd_ctty_fileid;

mach_port_t *_hurd_init_dtable;
size_t _hurd_init_dtablesize;

volatile int errno;		/* XXX wants to be per-thread */

char **__environ;

extern void EXFUN(__libc_init, (NOARGS));
extern void EXFUN(__mach_init, (NOARGS));
extern int EXFUN(main, (int argc, char **argv, char **envp));

extern void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
extern void (*_cthread_exit_routine) (int status);

#ifndef	SET_SP
#error "Machine-dependent cthread startup code needs to exist."
#endif

#ifndef	GET_STACK
#error "Machine-dependent stack startup code needs to exist."
#endif

#ifndef LOSE
#define	LOSE	__task_terminate (__mach_task_self ())
#endif

static int count (char *, size_t);
static void makevec (char *, size_t, char **);

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
  char **argv;

  char *p;

  GET_STACK (_hurd_stack_low, _hurd_stack_high);

  /* Basic Mach initialization, must be done before RPCs can be done.  */
  __mach_init ();

  if (_cthread_init_routine != NULL)
    {				/* XXXXXXXXXXXXXXXXXXXXXXXX */
      void *newsp = (*_cthread_init_routine) ();
      SET_SP (newsp);
      if (newsp < _hurd_stack_low || newsp > _hurd_stack_high)
	__vm_deallocate (__mach_task_self (),
			 _hurd_stack_low,
			 _hurd_stack_high - _hurd_stack_low);
    }

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    LOSE;

  if (in_bootstrap != MACH_PORT_NULL)
    {
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
      static char *noargs = NULL, *noenv = NULL;
      argc = 0;
      argv = &null;
      __environ = &noenv;
      _hurd_init_dtable = NULL;
      _hurd_init_dtablesize = 0;
      portarray = NULL;
      portarraysize = 0;
      intarray = NULL;
      intarraysize = 0;
    }
  else
    {
      /* Turn the block of null-separated strings we were passed for the
	 arguments and environment into vectors of pointers to strings.  */
      
      argc = count (args, argslen);
      envc = count (env, envlen);
      
      if (err = __vm_allocate (__mach_task_self (),
			       &argv, round_page ((argc + 1 + envc + 1) *
						  sizeof (char *)),
			       1))
	__libc_fatal ("hurd: Can't allocate space for argv and environ\n");
      __environ = &argv[argc + 1];
      
      makevec (args, argslen, argv);
      makevec (env, envlen, __environ);
    }

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
count (char *args, size_t argslen)
{
  char *p = args;
  size_t n = argslen;
  int argc;
  while (n > 0)
    {
      char *end = memchr (p, '\0', n);

      ++argc;

      if (end == NULL)
	/* The last argument is unterminated.  */
	break;

      n -= end + 1 - p;
      p = end + 1;
    }
  return argc;
}

static void
makevec (char *args, size_t argslen, char **argv)
{
  char *p = args;
  size_t n = argslen;
  int argc = 0;

  while (n > 0)
    {
      char *end = memchr (p, '\0', n);

      argv[argc++] = p;

      if (end == NULL)
	/* The last argument is unterminated.  */
	break;

      n -= end + 1 - p;
      p = end + 1;
    }

  argv[argc] = NULL;
}

/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.

   PORTARRAY and INTARRAY are vm_deallocate'd.  */

void
_hurd_init (char **argv,
	    mach_port_t *portarray, size_t portarraysize,
	    int *intarray, size_t intarraysize)
{
  int i;

  /* See what ports we were passed.  */
  for (i = 0; i < portarraysize; ++i)
    switch (i)
      {
#define	initport(upper, lower) \
      case INIT_PORT_##upper: \
	_hurd_port_init (&_hurd_##lower, portarray[i]); \
	break

	  /* Install the standard ports in their cells.  */
	initport (CWDIR, cwdir);
	initport (CRDIR, crdir);
	initport (AUTH, auth);

      case INIT_PORT_PROC:
	/* Install the proc port and tell the proc server we exist.  */
	_hurd_proc_init (portarray[i], argv);
	break;

      case INIT_PORT_BOOTSTRAP:
	/* When the user asks for the bootstrap port,
	   he will get the one the exec server passed us.  */
	__task_set_special_port (__mach_task_self (),
				 TASK_BOOTSTRAP_PORT, portarray[i]);
	/* FALLTHROUGH */

      default:
	/* Wonder what that could be.  */
	__mach_port_deallocate (__mach_task_self (), portarray[i]);
	break;
      }

  if (intarraysize > INIT_UMASK)
    _hurd_umask = intarray[INIT_UMASK] & 0777;
  else
    _hurd_umask = 0022;		/* XXX */
  if (intarraysize > INIT_CTTY_FILEID) /* Knows that these are sequential.  */
    {
      _hurd_ctty_fstype = intarray[INIT_CTTY_FSTYPE];
      _hurd_ctty_fsid.val[0] = intarray[INIT_CTTY_FSID1];
      _hurd_ctty_fsid.val[1] = intarray[INIT_CTTY_FSID2];
      _hurd_ctty_fileid = intarray[INIT_CTTY_FILEID];
    }


  /* All done with init ints and ports.  */
  __vm_deallocate (__mach_task_self (), intarray, nints * sizeof (int));
  __vm_deallocate (__mach_task_self (),
		   portarray, nports * sizeof (mach_port_t));
}
