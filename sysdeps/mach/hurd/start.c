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

/* The first piece of initialized data.  */
int __data_start = 0;

volatile int errno;

char **__environ;

extern void EXFUN(__libc_init, (NOARGS));
extern void EXFUN(__mach_init, (NOARGS));
extern int EXFUN(main, (int argc, char **argv, char **envp));

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
  mach_port_t in_bootstrap, passed_bootstrap;
  char *args, *env;
  mach_port_t *dtable, *portarray;
  int *intarray;
  size_t argslen, envlen, dtablesize, portarraysize, intarraysize;
  int dealloc_args, dealloc_env;

  int argc, envc;
  char **argv;

  char *p;

  /* Basic Mach initialization, must be done before RPCs can be done.  */
  __mach_init ();

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    _exit (err);

  if (err = __exec_startup (in_bootstrap,
			    &args, &argslen, &dealloc_args,
			    &env, &envlen, &dealloc_env,
			    &dtable, &dtablesize,
			    &portarray, &portarraysize,
			    &intarray, &intarraysize,
			    &passed_bootstrap))
    _exit (err);

  /* When the user asks for the bootstrap port,
     he will get the one the exec server passed us.  */
  __task_set_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
			   passed_bootstrap);
  __mach_port_deallocate (__mach_task_self (), passed_bootstrap);

  argc = count (args, argslen);
  envc = count (env, envlen);

  if (err = __vm_allocate (__mach_task_self (),
			   &argv, round_page ((argc + 1 + envc + 1) *
					      sizeof (char *)),
			   1))
    _exit (err);
  __environ = &argv[argc + 1];

  makevec (args, argslen, argv);
  makevec (env, envlen, __environ);

  /* Tell the proc server where our args and environment are.  */
  __proc_setprocargs (portarray[INIT_PORT_PROC], argv, envp);

  /* Make signal thread, init umask and ctty info from intarray, etc.  XXX */

  __libc_init ();

  exit (main (argc, argv, envp));
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
