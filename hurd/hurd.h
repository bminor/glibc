/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#ifndef	_HURD_H

#define	_HURD_H	1

#define	_GNU_SOURCE
#include <features.h>

#include <mach.h>
#include <hurd/hurd_types.h>
#include <hurd/process.h>
#include <hurd/fs.h>
#include <hurd/io.h>
#include <errno.h>

#define	__hurd_fail(err)	(errno = (err), -1)


/* Basic ports and info, initialized by startup.  */
extern mutex_t _hurd_lock;	/* Locks against change, but not reference.  */
extern process_t _hurd_proc;
extern file_t _hurd_ccdir, _hurd_cwdir, _hurd_crdir, _hurd_auth;
extern mode_t _hurd_umask;
extern int _hurd_ctty_fstype;
extern fsid_t _hurd_ctty_fsid;
extern ino_t _hurd_ctty_fileid;
extern vm_address_t _hurd_stack_low, _hurd_stack_high;
extern thread_t _hurd_sigport_thread;
extern mach_port_t _hurd_sigport; /* Locked by _hurd_siglock.  */

/* Not locked.  If we are using a real dtable,
   these are turned into that and then cleared at startup.
   If not, these are never changed after startup.  */
extern mach_port_t *_hurd_init_dtable;
extern size_t _hurd_init_dtablesize;

/* Lightweight user references for ports.  */

/* Structure describing a cell containing a port.
   With the lock held, a user extracts PORT, and sets USER_DEALLOC to point
   to a word in his local storage.  PORT can then safely be used.  When
   PORT is no longer needed, with the held held, the user examines
   USER_DEALLOC.  If it is the same address that user stored there, it
   extracts *USER_DEALLOC, clears USER_DEALLOC to NULL, and releases the
   lock.  If USER_DEALLOC was set to the user's pointer, and *USER_DEALLOC
   is set, the user deallocates the port he used.  */
struct _hurd_port
  {
    spin_lock_t lock;		/* Locks rest.  */
    mach_port_t port;		/* Port. */
    int *user_dealloc;		/* If not NULL, points to user's flag word.  */
  };

/* Initialize *PORT.  */
static inline void
_hurd_port_init (struct _hurd_port *port)
{
  __spin_lock_init (&port->lock);
  port->port = MACH_PORT_NULL;
  port->user_dealloc = NULL;
}

/* Get a reference to *PORT.
   Pass return value and MYFLAG to _hurd_port_free when done.  */
static inline mach_port_t
_hurd_port_get (struct _hurd_port *port, int *myflag)
{
  mach_port_t result;
  __spin_lock (&port->lock);
  result = port->port;
  if (result != MACH_PORT_NULL)
    port->user_dealloc = myflag;
  __spin_unlock (&port->lock);
  return result;
}

/* Free a reference to gotten with
   `USED_PORT = _hurd_port_get (PORT, MYFLAG);' */
static inline void
_hurd_port_free (struct _hurd_port *port,
int *myflag, mach_port_t used_port)
{
  __spin_lock (&port->lock);
  if (port->user_dealloc == myflag)
    port->user_dealloc = NULL;
  else
    myflag = NULL;
  __spin_unlock (&port->lock);
  if (myflag != NULL && *myflag)
    __mach_port_deallocate (__mach_task_self (), used_port);
}

/* Set *PORT's port to NEWPORT.  */
static inline void
_hurd_port_set (struct _hurd_port *port, mach_port_t newport)
{
  mach_port_t old;
  __spin_lock (&port->lock);
  if (port->user_dealloc == NULL)
    old = port->port;
  else
    {
      old = MACH_PORT_NULL;
      *port->user_dealloc = 1;
    }
  port->port = newport;
  __spin_unlock (&port->lock);
  if (old != MACH_PORT_NULL)
    __mach_port_deallocate (__mach_task_self (), old);
}

/* Get a reference for the send right in *VAR,
   made atomic by locking LOCK.  */
static inline mach_port_t
_hurd_getport (volatile const mach_port_t *var, mutex_t lock)
{
  mach_port_t port;
  __mutex_lock (lock);
  port = *var;
  __mach_port_mod_refs (__mach_task_self (), port,
			MACH_PORT_RIGHT_SEND, 1);
  __mutex_unlock (lock);
  return port;
}


/* File descriptor table.  */
struct _hurd_dtable
  {
    int size;			/* Number of elts in `d' array.  */
    /* Individual descriptors are not locked.  It is up to the user to
       synchronize descriptor operations on a single descriptor.  */
    struct
      {
	io_t server;
	int isctty;		/* Is a port to the controlling tty.  */
	int flags;		/* fcntl flags.  */
      } *d;
  };
extern struct _hurd_dtable _hurd_dtable;
extern struct mutex _hurd_dtable_lock; /* Locks _hurd_dtable.  */

/* Allocate a new file descriptor.  The dtable lock must be held.  */
static inline int
_hurd_dalloc (void)
{
  int i;
  for (i = 0; i < _hurd_dtable.size; ++i)
    if (_hurd_dtable.d[i] == MACH_PORT_NULL)
      {
	_hurd_dtable.d[i].isctty = -1;
	_hurd_dtable.d[i].flags = 0;
	return i;
      }
  errno = EMFILE;
  return -1;
}

/* Return the server port for FD.  Does not add a ref.  Takes the dtable lock.
   On error, sets errno and returns MACH_PORT_NULL.  */
static inline io_t
_hurd_dport (int fd)
{
  io_t port;
  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      errno = EBADF;
      port = MACH_PORT_NULL;
    }
  else
    port = _hurd_dtable.d[fd].server;
  __mutex_unlock (&_hurd_dtable.lock);
  return port;
}


/* Return the socket server for sockaddr domain DOMAIN.  */
extern socket_t _hurd_socket_server (int domain);


/* Current process IDs.  */
extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;


/* User and group IDs.  */
extern mutex_t _hurd_idlock;
extern int _hurd_id_valid;	/* Nonzero if _hurd_id is valid.  */
extern idblock_t _hurd_id;
extern auth_t _hurd_rid_auth;	/* Cache used by access.  */


/* Unix `data break', for brk and sbrk.
   If brk and sbrk are not used, this info will not be initialized or used.  */
extern vm_address_t _hurd_brk;	/* Data break.  */
extern vm_address_t _hurd_data_end; /* End of allocated space.  */
extern struct mutex _hurd_brk_lock; /* Locks brk and data_end.  */
extern int _hurd_set_data_limit (const struct rlimit *);

/* Set the data break; the brk lock must
   be held, and is released on return.  */
extern int _hurd_set_brk (vm_address_t newbrk);


/* Resource limit on core file size.  */
extern int _hurd_core_limit;


#include <signal.h>

/* Per-thread signal state.  */
struct _hurd_sigstate
  {
    thread_t thread;
    struct _hurd_sigstate *next; /* Linked-list of thread sigstates.  */

    struct mutex lock;		/* Locks the rest of this structure.  */
    sigset_t blocked;
    sigset_t pending;
    struct sigaction actions[NSIG];
    struct sigstack sigstack;
    int sigcodes[NSIG];		/* Codes for pending signals.  */

    int suspended;		/* If nonzero, sig_post signals `arrived'.  */
    struct condition arrived;
    mach_port_t suspend_reply;	/* Reply port for sig_post RPC.  */

    int vforked;		/* Nonzero if this thread is a vfork child.  */
    struct
      {
	process_t proc;
	file_t ccdir, cwdir, crdir, auth;
	mode_t umask;
	int ctty_fstype;
	fsid_t ctty_fsid;
	ino_t ctty_fileid;
	struct _hurd_dtable *dtable;
	jmp_buf continuation;
      } *vfork_saved;

    /* Not locked.  Used only by this thread,
       or by signal thread with this thread suspended.  */
    mach_port_t intr_port;	/* Port an interruptible RPC was sent on.  */
    int intr_is_wait;		/* Interruptible RPC was wait, not io.  */
    int intr_restart;		/* If nonzero, restart interrupted RPC.  */
  };
/* Linked list of states of all threads
   whose state has been inquired about.  */
extern struct _hurd_sigstate *_hurd_sigstates;
extern struct mutex _hurd_siglock; /* Locks _hurd_sigstates.  */
/* Get the sigstate of a given thread, taking its lock.  */
extern struct _hurd_sigstate *_hurd_thread_sigstate (thread_t);

/* Thread to receive process-global signals.  */
extern thread_t _hurd_sigthread;

/* Called by the machine-dependent exception handler.  */
extern void _hurd_exc_post_signal (thread_t, int sig, int code);

/* SS->lock is held on entry, and released before return.  */
extern void _hurd_internal_post_signal (reply_port_t,
					struct _hurd_sigstate *ss,
					int signo, int sigcode,
					sigset_t *restore_blocked);

/* Function run by the signal thread to receive from the signal port.  */
extern void _hurd_sigport_receive (void);

/* Set the signal-receiving thread.  */
extern int sigsetthread (thread_t);


/* Perform interruptible RPC CALL on PORT.
   The args in CALL should be constant or local variable refs.
   They may be evaluated many times, and must not change.
   PORT must not be deallocated before this RPC is finished.  */
#define	_HURD_EINTR_RPC(port, is_wait, call) \
  ({
    error_t __err;
    struct _hurd_sigstate *__ss
      = _hurd_thread_sigstate (__mach_thread_self ());
    __mutex_unlock (&__ss->lock); /* Lock not needed.  */
    __ss->intr_is_wait = (is_wait);
    __ss->intr_restart = 1;
    /* This one needs to be last.  A signal can arrive before here,
       and if intr_port were set before intr_is_wait and intr_restart are
       initialized, the signal thread would get confused.  */
    __ss->intr_port = (port);
    /* A signal may arrive here, after intr_port is set,
       but before the mach_msg system call.  The signal handler might do an
       interruptible RPC, and clobber intr_port; then it would not be set
       properly when we actually did send the RPC, and a later signal
       wouldn't interrupt that RPC.  So, _hurd_run_sighandler saves intr_port
       and intr_is_wait in the sigcontext, and sigreturn restores them.  */
  __do_call:
    switch (__err = (call))
      {
      case POSIX_EINTR:		/* RPC went out and was interrupted.  */
      case MACH_SEND_INTERRUPTED: /* RPC didn't get out.  */
      case MACH_RCV_INTERRUPTED: /* RPC pending.  */
	if (__ss->intr_restart)
	  /* Restart the interrupted call.  */
	  goto __do_call;
	/* Return EINTR.  */
	__err = POSIX_EINTR;
	break;
      }
    __ss->intr_port = MACH_PORT_NULL;
    __err;
  })

/* Calls to get and set basic ports.  */
extern process_t getproc (void);
extern file_t getccdir (void), getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern int setproc (process_t);
extern int setccdir (file_t), setcwdir (file_t), setcrdir (file_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);
#define	setauth	__setauth


/* Front-end to dir_pathtrans.  */
extern error_t __dir_lookup (file_t startdir, const char *name,
			     int flags, mode_t mode,
			     file_t *result);
#define	dir_lookup __dir_lookup

/* Returns a port to the directory, and sets *NAME to the file name.  */
extern file_t __hurd_path_split (const char *file, const char **name);
#define	hurd_path_split	__hurd_path_split

/* Looks up FILE with the given FLAGS and MODE (as for dir_pathtrans).  */
extern file_t __hurd_path_lookup (const char *file, int flags, mode_t mode);
#define hurd_path_lookup __hurd_path_lookup

/* Open a file descriptor on a port.  */
extern int openport (io_t port);


#endif	/* hurd.h */
