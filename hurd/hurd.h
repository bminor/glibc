/* Copyright (C) 1993 Free Software Foundation, Inc.
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

/* Lightweight user references for ports.  */

/* Structure describing a cell containing a port.
   With the lock held, a user extracts PORT, and sets USER_DEALLOC to point
   to a word in his local storage.  PORT can then safely be used.  When
   PORT is no longer needed, with the lock held, the user examines
   USER_DEALLOC.  If it is the same address that user stored there, he
   extracts *USER_DEALLOC, clears USER_DEALLOC to NULL, and releases the
   lock.  If *USER_DEALLOC is set, the user deallocates the port he used.  */
struct _hurd_port
  {
    spin_lock_t lock;		/* Locks rest.  */
    mach_port_t port;		/* Port. */
    int *user_dealloc;		/* If not NULL, points to user's flag word.  */
  };

/* Evaluate EXPR with the variable `port' bound to the port in PORTCELL.  */
#define	_HURD_PORT_USE(portcell, expr)					      \
  ({ struct _hurd_port *const __p = &(portcell);			      \
     int __dealloc;							      \
     const mach_port_t port = _hurd_port_get (__p, &__dealloc);		      \
     __typeof(expr) __result = (expr);					      \
     _hurd_port_free (__p, &__dealloc);					      \
     __result; })

/* Initialize *PORT to INIT.  */
static inline void
_hurd_port_init (struct _hurd_port *port, mach_port_t init)
{
  __spin_lock_init (&port->lock);
  port->port = init;
  port->user_dealloc = NULL;
}

/* Get a reference to *PORT, which is locked.
   Pass return value and MYFLAG to _hurd_port_free when done.  */
static inline mach_port_t
_hurd_port_locked_get (struct _hurd_port *port, int *myflag)
{
  mach_port_t result;
  result = port->port;
  if (result != MACH_PORT_NULL)
    {
      port->user_dealloc = myflag;
      *myflag = 0;
    }
  __spin_unlock (&port->lock);
  return result;
}

/* Same, but locks PORT first.  */
static inline mach_port_t
_hurd_port_get (struct _hurd_port *port, int *myflag)
{
  __spin_lock (&port->lock);
  return _hurd_port_locked_get (port, myflag);
}

/* Free a reference gotten with
   `USED_PORT = _hurd_port_get (PORT, MYFLAG);' */
static inline void
_hurd_port_free (struct _hurd_port *port,
		 int *myflag, mach_port_t used_port)
{
  __spin_lock (&port->lock);
  if (port->user_dealloc == myflag)
    port->user_dealloc = NULL;
  __spin_unlock (&port->lock);
  if (*myflag)
    __mach_port_deallocate (__mach_task_self (), used_port);
}

/* Set *PORT's port to NEWPORT.  PORT is locked.  */
static inline void
_hurd_port_locked_set (struct _hurd_port *port, mach_port_t newport)
{
  mach_port_t old;
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

/* Same, but locks PORT first.  */
static inline void
_hurd_port_set (struct _hurd_port *port, mach_port_t newport)
{
  __spin_lock (&port->lock);
  return _hurd_port_locked_set (port, newport);
}

/* Basic ports and info, initialized by startup.  */
extern struct _hurd_port _hurd_proc, _hurd_auth;
extern struct _hurd_port _hurd_cwdir, _hurd_crdir;
extern struct _hurd_port _hurd_cttyid;
extern volatile mode_t _hurd_umask;

extern vm_address_t _hurd_stack_low, _hurd_stack_high; /* Not locked.  */

extern thread_t _hurd_msgport_thread;
extern mach_port_t _hurd_msgport; /* Locked by _hurd_siglock.  */

/* Not locked.  If we are using a real dtable,
   these are turned into that and then cleared at startup.
   If not, these are never changed after startup.  */
extern mach_port_t *_hurd_init_dtable;
extern size_t _hurd_init_dtablesize;

/* File descriptor table.  */
struct _hurd_fd
  {
    struct _hurd_port port;
    int flags;			/* fcntl flags; locked by port.lock.  */

    /* Normal port to the ctty.  Also locked by port.lock.
       (The ctty.lock is only ever used when the port.lock is held.)  */
    struct _hurd_port ctty;
  };

struct _hurd_dtable
  {
    int size;			/* Number of elts in `d' array.  */

    /* Uses of individual descriptors are not locked.  It is up to the user
       to synchronize descriptor operations on a single descriptor.  */

    struct _hurd_fd *d;
  };
extern struct mutex _hurd_dtable_lock; /* Locks next two.  */
extern struct _hurd_dtable _hurd_dtable;
extern int _hurd_dtable_rlimit;

/* If not NULL, pointed-to word is set when _hurd_dtable.d changes.
   User who set `user_dealloc' should free the _hurd_dtable.d value
   he used if his word is set when he is finished.
   If NULL, the old value of _hurd_dtable.d is freed by the setter.  */
int *_hurd_dtable_user_dealloc;

static inline struct _hurd_dtable
_hurd_dtable_use (int *dealloc)
{
  struct _hurd_dtable dtable;
  __mutex_lock (&_hurd_dtable_lock);
  _hurd_dtable_user_dealloc = dealloc;
  dtable = _hurd_dtable;
  __mutex_unlock (&_hurd_dtable_lock);
  return dtable;
}

struct _hurd_dtable_resizes
  {
    size_t n;
    void (*free) (void *);
    void *terminator;
  };
extern const struct _hurd_dtable_resizes _hurd_dtable_resizes;

static inline void
_hurd_dtable_done (struct _hurd_dtable dtable, int *dealloc)
{
  __mutex_lock (&_hurd_dtable_lock);
  if (_hurd_dtable_user_dealloc == dealloc)
    _hurd_dtable_user_dealloc = NULL;
  __mutex_unlock (&_hurd_dtable_lock);
  if (*dealloc)
    /* _hurd_dtable_resizes is a symbol set.
       setdtablesize.c gives it one element: free.
       If setdtablesize is not linked in, *DEALLOC
       will never get set, so we will never get here.
       This hair avoids linking in free if we don't need it.  */
    (*_hurd_dtable_resizes.free) (dtable);
}


/* Allocate a new file descriptor and install PORT, CTTY, and FLAGS in it.
   If table is full, deallocate PORT and CTTY, set errno, and return -1.  */
static inline int
_hurd_dalloc (io_t port, io_t ctty, int flags)
{
  int i;
  __mutex_lock (&hurd_dtable_lock);
  for (i = 0; i < _hurd_dtable.size; ++i)
    {
      struct _hurd_fd *d = &_hurd_dtable.d[i];
      __spin_lock (&d->port.lock);
      if (d->port.port == MACH_PORT_NULL)
	{
	  d->port.port = port;
	  d->port.user_dealloc = NULL;
	  d->ctty.port = ctty;
	  d->ctty.user_dealloc = NULL;
	  d->flags = flags;
	  __spin_unlock (&d->port.lock);
	  __mutex_unlock (&hurd_dtable_lock);
	  return i;
	}
      __spin_unlock (&d->port.lock);
    }
  __mutex_unlock (&hurd_dtable_lock);
  __mach_port_deallocate (__mach_task_self (), port);
  __mach_port_deallocate (__mach_task_self (), ctty);
  errno = EMFILE;
  return -1;
}

/* Return the descriptor cell for FD in DTABLE, locked.  */
static inline struct _hurd_fd *
_hurd_dtable_fd (int fd, struct _hurd_dtable dtable)
{
  if (fd < 0 || fd >= dtable.size)
    return NULL;
  else
    {
      struct _hurd_fd *cell = &dtable.d[fd];
      __spin_lock (&cell->port.lock);
      if (cell->port.port == MACH_PORT_NULL)
	{
	  __spin_unlock (&cell->port.lock);
	  return NULL;
	}
      return cell;
    }
}

struct _hurd_fd_user
  {
    struct _hurd_dtable dtable;
    struct _hurd_fd *d;
  };

/* Returns the descriptor cell for FD, locked.  */
static inline struct _hurd_fd_user
_hurd_fd (int fd, int *dealloc)
{
  struct _hurd_fd_user d;
  d.dtable = _hurd_dtable_use (dealloc);
  d.d = _hurd_dtable_fd (fd, dtable);
  if (d.d == NULL)
    _hurd_dtable_done (d.dtable, dealloc);
  return d;
}

static inline void
_hurd_fd_done (struct _hurd_fd_user d, int *dealloc)
{
  _hurd_dtable_done (d->dtable, dealloc);
}

/* Evaluate EXPR with the variable `port' bound to the port to FD,
   and `ctty' bound to the ctty port.  */
   
#define	_HURD_DPORT_USE(fd, expr)					      \
  ({ int __dealloc_dt;							      \
     struct _hurd_fd_user __d = _hurd_fd (fd, &__dealloc_dt);		      \
     if (__d.cell == NULL)						      \
       EBADF;								      \
     else								      \
       {								      \
	 int __dealloc = 0, __dealloc_ctty = 0;				      \
	 io_t port = _hurd_port_locked_get (&__d.d->port, &__dealloc);	      \
	 io_t ctty = _hurd_port_locked_get (&__d.d->ctty, &__dealloc_ctty);   \
	 __typeof (expr) __result;					      \
	 __result = (expr);						      \
	 _hurd_port_free (&__d.d->port, port, &__dealloc);		      \
	 if (ctty != MACH_PORT_NULL)					      \
	   _hurd_port_free (&__d.d->ctty, ctty, &__dealloc_ctty);	      \
	 _hurd_fd_done (__d, &__dealloc_dt);				      \
	 __result;							      \
       }								      \
   })									      \

static inline int
_hurd_dfail (int fd, error_t err)
{
  switch (err)
    {
    case MACH_SEND_INVALID_DEST: /* The server has disappeared!  */
      _hurd_raise_signal (NULL, SIGLOST, fd);
      break;
    case EPIPE:
      _hurd_raise_signal (NULL, SIGPIPE, fd);
      break;
    default:
      return __hurd_fail (err);
    }
}

/* Return the socket server for sockaddr domain DOMAIN.  */
extern socket_t _hurd_socket_server (int domain);

/* Return a receive right which will not be sent to.  */
extern mach_port_t _hurd_dead_recv (void);


/* Current process IDs.  */
extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;


/* User and group IDs.  */
extern mutex_t _hurd_idlock;
extern int _hurd_id_valid;	/* If _hurd_uid and _hurd_gid are valid.  */
extern struct idlist _hurd_uid, _hurd_gid;
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

/* Resource limit on core file size.  Enforced by hurdsig.c.  */
extern int _hurd_core_limit;

#ifdef notyet

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
    struct sigaltstack sigaltstack;
    int sigcodes[NSIG];		/* Codes for pending signals.  */

    int suspended;		/* If nonzero, sig_post signals `arrived'.  */
    struct condition arrived;

#if 0
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
#endif

    /* Not locked.  Used only by this thread,
       or by signal thread with this thread suspended.  */
    volatile mach_port_t intr_port; /* Port interruptible RPC was sent on.  */
    volatile int intr_restart;	/* If nonzero, restart interrupted RPC.  */
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
extern void _hurd_internal_post_signal (struct _hurd_sigstate *ss,
					int signo, int sigcode,
					sigset_t *restore_blocked);

/* Function run by the signal thread to receive from the signal port.  */
extern void _hurd_msgport_receive (void);


/* Perform interruptible RPC CALL on PORT.
   The args in CALL should be constant or local variable refs.
   They may be evaluated many times, and must not change.
   PORT must not be deallocated before this RPC is finished.  */
#define	HURD_EINTR_RPC(port, call) \
  ({
    error_t __err;
    struct _hurd_sigstate *__ss
      = _hurd_thread_sigstate (__mach_thread_self ());
    __mutex_unlock (&__ss->lock); /* Lock not needed.  */
    /* If we get a signal and should return EINTR, the signal thread will
       clear this.  The RPC might return EINTR when some other thread gets
       a signal, in which case we want to restart our call.  */
    __ss->intr_restart = 1;
    /* This one needs to be last.  A signal can arrive before here,
       and if intr_port were set before intr_restart is
       initialized, the signal thread would get confused.  */
    __ss->intr_port = (port);
    /* A signal may arrive here, after intr_port is set,
       but before the mach_msg system call.  The signal handler might do an
       interruptible RPC, and clobber intr_port; then it would not be set
       properly when we actually did send the RPC, and a later signal
       wouldn't interrupt that RPC.  So, _hurd_run_sighandler saves
       intr_port in the sigcontext, and sigreturn restores it.  */
  __do_call:
    switch (__err = (call))
      {
      case EINTR:		/* RPC went out and was interrupted.  */
      case MACH_SEND_INTERRUPTED: /* RPC didn't get out.  */
	if (__ss->intr_restart)
	  /* Restart the interrupted call.  */
	  goto __do_call;
	/* FALLTHROUGH */
      case MACH_RCV_PORT_DIED:
	/* Server didn't respond to interrupt_operation,
	   so the signal thread destroyed the reply port.  */
	__err = EINTR;
	break;
      }
    __ss->intr_port = MACH_PORT_NULL;
    __err;
  })

#endif /* notyet */

/* Calls to get and set basic ports.  */
extern process_t getproc (void);
extern file_t getccdir (void), getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern int setproc (process_t);
extern int setcwdir (file_t), setcrdir (file_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);
#define	setauth	__setauth	/* XXX */


extern error_t __hurd_path_split (file_t crdir, file_t cwdir,
				  const char *file,
				  file_t *dir, const char **name);
extern error_t hurd_path_split (file_t crdir, file_t cwdir,
				const char *file,
				file_t *dir, const char **name);
#define	hurd_path_split	__hurd_path_split /* XXX */
extern error_t __hurd_path_lookup (file_t crdir, file_t cwdir,
				   const char *file,
				   int flags, mode_t mode,
				   file_t *file);
extern error_t hurd_path_lookup (file_t crdir, file_t cwdir,
				 const char *file,
				 int flags, mode_t mode,
				 file_t *file);
#define	hurd_path_lookup __hurd_path_lookup /* XXX */

/* Returns a port to the directory, and sets *NAME to the file name.  */
extern file_t __path_split (const char *file, const char **name);
extern file_t path_split (const char *file, const char **name);
#define	path_split	__path_split

/* Looks up FILE with the given FLAGS and MODE (as for dir_pathtrans).  */
extern file_t __path_lookup (const char *file, int flags, mode_t mode);
extern file_t path_lookup (const char *file, int flags, mode_t mode);
#define path_lookup __path_lookup /* XXX */

/* Open a file descriptor on a port.  */
extern int openport (io_t port);

/* Inform the proc server we have exitted with STATUS, and kill the
   task thoroughly.  This function never returns, no matter what.  */
extern volatile void _hurd_exit (int status);


#endif	/* hurd.h */
