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

/* The first piece of initialized data.  */
int __data_start = 0;

process_t _hurd_proc;
file_t _hurd_ccdir, _hurd_cwdir, _hurd_crdir;
mode_t _hurd_umask;
int _hurd_ctty_fstype;
fsid_t _hurd_ctty_fsid;
ino_t _hurd_ctty_fileid;
mach_port_t _hurd_sigport;
thread_t _hurd_sigport_thread;

mach_port_t *_hurd_init_dtable;
size_t _hurd_init_dtablesize;

volatile int errno;

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
  mach_port_t *portarray;
  int *intarray;
  size_t argslen, envlen, portarraysize, intarraysize;
  int dealloc_args, dealloc_env;

  int argc, envc;
  char **argv;

  char *p;

  GET_STACK (_hurd_stack_low, _hurd_stack_high);

  /* Basic Mach initialization, must be done before RPCs can be done.  */
  __mach_init ();

  if (_cthread_init_routine != NULL)
    SET_SP ((*_cthread_init_routine) ());

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    _exit (err);

  err = __exec_startup (in_bootstrap,
			&args, &argslen, &dealloc_args,
			&env, &envlen, &dealloc_env,
			&_hurd_init_dtable, &_hurd_init_dtablesize,
			&portarray, &portarraysize,
			&intarray, &intarraysize,
			&passed_bootstrap);
  __mach_port_deallocate (__mach_task_self (), in_bootstrap);
  if (err)
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
    __libc_fatal ("hurd: Can't allocate space for args and env\n");
  __environ = &argv[argc + 1];

  makevec (args, argslen, argv);
  makevec (env, envlen, __environ);

  __mutex_init (&_hurd_lock);
  for (i = 0; i < portarraysize; ++i)
    switch (i)
      {
      case INIT_PORT_PROC:
	_hurd_proc = portarray[i];
	break;
      case INIT_PORT_CCDIR:
	_hurd_ccdir = portarray[i];
	break;
      case INIT_PORT_CWDIR:
	_hurd_cwdir = portarray[i];
	break;
      case INIT_PORT_CRDIR:
	_hurd_crdir = portarray[i];
	break;
      case INIT_PORT_AUTH:
	_hurd_auth = portarray[INIT_PORT_AUTH];
	break;
      default:
	/* Wonder what that could be.  */
	__mach_port_deallocate (__mach_task_self (), portarray[i]);
	break;
      }

  if (intarraysize > INIT_UMASK)
    _hurd_umask = intarray[INIT_UMASK] & 0777;
  if (intarraysize > INIT_CTTY_FILEID) /* Knows that these are sequential.  */
    {
      _hurd_ctty_fstype = intarray[INIT_CTTY_FSTYPE];
      _hurd_ctty_fsid.val[0] = intarray[INIT_CTTY_FSID1];
      _hurd_ctty_fsid.val[1] = intarray[INIT_CTTY_FSID2];
      _hurd_ctty_fileid = intarray[INIT_CTTY_FILEID];
    }

  {
    struct _hurd_sigstate *ss;
    thread_t sigthread;
    mach_port_t oldsig, oldtask;
    int i;
    sigset_t ignored = nints > INIT_SIGIGN ? intarray[INIT_SIGIGN] : 0;
    ignored &= ~_SIG_CANT_IGNORE;

    ss = _hurd_thread_sigstate (__mach_thread_self ());
    ss->blocked = nints > INIT_SIGMASK ? intarray[INIT_SIGMASK] : 0;
    ss->blocked &= ~_SIG_CANT_BLOCK;
    __sigemptyset (&ss->pending);
    for (i = 1; i < NSIG; ++i)
      ss->actions[i].sa_handler
	= __sigismember (i, &ignored) ? SIG_IGN : SIG_DFL;
    __mutex_unlock (&ss->lock);

    if (err = __mach_port_allocate (__mach_task_self (),
				    MACH_PORT_RIGHT_RECEIVE,
				    &_hurd_sigport))
      __libc_fatal ("hurd: Can't create signal port receive right\n");

    if (err = __thread_create (__mach_task_self (), &sigthread))
      __libc_fatal ("hurd: Can't create signal thread\n");
    if (err = _hurd_start_sigthread (sigthread, _hurd_sigport_receive))
      __libc_fatal ("hurd: Can't start signal thread\n");
    _hurd_sigport_thread = sigthread;

    /* Make a send right to the signal port.  */
    if (err = __mach_port_insert_right (__mach_task_self (),
					_hurd_sigport,
					MACH_PORT_RIGHT_MAKE_SEND))
      __libc_fatal ("hurd: Can't create send right to signal port\n");

    /* Receive exceptions on the signal port.  */
    __task_set_special_port (__mach_task_self (),
			     TASK_EXCEPTION,
			     _hurd_sigport);

    /* Give the proc server our task and signal ports.  */
    __proc_setports (_hurd_proc, _hurd_sigport, __mach_task_self (),
		     &oldsig, &oldtask);
    if (oldsig != MACH_PORT_NULL)
      __mach_port_deallocate (__mach_task_self (), oldsig);
    if (oldtask != MACH_PORT_NULL)
      __mach_port_deallocate (__mach_task_self (), oldtask);

    /* Free the send right we gave to the proc server.  */
    __mach_port_deallocate (__mach_task_self (), _hurd_sigport);
  }

  /* Tell the proc server where our args and environment are.  */
  __proc_setprocargs (_hurd_proc, argv, __environ);

  __libc_init (argc, argv, __environ);

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
