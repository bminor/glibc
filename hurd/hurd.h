/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

/* This is here because this file includes some other headers
   and expects some non-ANSI, non-POSIX symbols to be defined.  */
#ifndef _GNU_SOURCE
#define	_GNU_SOURCE
#endif
#include <features.h>


/* Get types, macros, constants and function declarations
   for all Mach microkernel interaction.  */
#include <mach.h>
#include <mach/mig_errors.h>

/* Get types and constants necessary for Hurd interfaces.  */
#include <hurd/hurd_types.h>

/* Get MiG stub declarations for commonly used Hurd interfaces.  */
#include <hurd/auth.h>
#include <hurd/process.h>
#include <hurd/fs.h>
#include <hurd/io.h>
#include <hurd/msg.h>

#include <errno.h>
#define	__hurd_fail(err)	(errno = (err), -1)

#define __spin_lock(lockaddr) /* no-op XXX */
#define __spin_unlock(lockaddr) /* no-op XXX */

#define __mutex_lock(lockaddr) /* no-op XXX */
#define __mutex_unlock(lockaddr) /* no-op XXX */


/* Lightweight user references for ports.  */

/* Structure describing a cell containing a port.  With the lock held, a
   user extracts PORT, and attaches his own link (in local storage) to the
   USERS chain.  PORT can then safely be used.  When PORT is no longer
   needed, with the lock held, the user removes his link from the chain.
   If his link is the last, and PORT has changed since he fetched it, the
   user deallocates the port he used.  */
struct _hurd_port
  {
#ifdef noteven
    spin_lock_t lock;		/* Locks rest.  */
#endif
    struct _hurd_port_userlink *users; /* Chain of users; see below.  */
    mach_port_t port;		/* Port. */
  };

/* This structure is simply a doubly-linked list.
   Users of a port cell are recorded by their presence in the list.  */   
struct _hurd_port_userlink
  {
    struct _hurd_port_userlink *next, **prevp;
  };

/* Evaluate EXPR with the variable `port' bound to the port in PORTCELL.  */
#define	_HURD_PORT_USE(portcell, expr)					      \
  ({ struct _hurd_port *const __p = (portcell);				      \
     struct _hurd_port_userlink __link;					      \
     const mach_port_t port = _hurd_port_get (__p, &__link);		      \
     __typeof(expr) __result = (expr);					      \
     _hurd_port_free (__p, &__link, port);				      \
     __result; })

/* Initialize *PORT to INIT.  */
static inline void
_hurd_port_init (struct _hurd_port *port, mach_port_t init)
{
#ifdef noteven
  __spin_lock_init (&port->lock);
#endif
  port->users = NULL;
  port->port = init;
}

/* Get a reference to *PORT, which is locked.
   Pass return value and LINK to _hurd_port_free when done.  */
static inline mach_port_t
_hurd_port_locked_get (struct _hurd_port *port,
		       struct _hurd_port_userlink *link)
{
  mach_port_t result;
  result = port->port;
  if (result != MACH_PORT_NULL)
    {
      link->next = port->users;
      if (link->next)
	link->next->prevp = &link->next;
      link->prevp = &port->users;
      port->users = link;
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
   `USED_PORT = _hurd_port_get (PORT, LINK);' */
static inline void
_hurd_port_free (struct _hurd_port *port,
		 struct _hurd_port_userlink *link,
		 mach_port_t used_port)
{
  int dealloc;
  __spin_lock (&port->lock);
  /* We should deallocate USED_PORT if our chain has been detached from the
     cell (and thus has a nil `prevp'), and there is no next link
     representing another user reference to the same port we fetched.  */
  dealloc = ! link->next && ! link->prevp;
  /* Remove our link from the chain of current users.  */
  if (link->prevp)
    *link->prevp = link->next;
  if (link->next)
    link->next->prevp = link->prevp;
  __spin_unlock (&port->lock);
  if (dealloc)
    __mach_port_deallocate (__mach_task_self (), used_port);
}

/* Set *PORT's port to NEWPORT.  NEWPORT's reference is consumed by PORT->port.
   PORT->lock is locked.  */
static inline void
_hurd_port_locked_set (struct _hurd_port *port, mach_port_t newport)
{
  mach_port_t old;
  if (port->users == NULL)
    old = port->port;
  else
    {
      old = MACH_PORT_NULL;
      /* Detach the chain of current users from the cell.  The last user to
	 remove his link from that chain will deallocate the old port.  */
      port->users->prevp = NULL;
      port->users = NULL;
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
extern struct _hurd_port *_hurd_ports;
extern unsigned int _hurd_nports;
extern volatile mode_t _hurd_umask;

/* Shorthand macro for referencing _hurd_ports.  */
#define	__USEPORT(which, expr) \
  _HURD_PORT_USE (&_hurd_ports[INIT_PORT_##which], (expr))

/* Base address and size of the initial stack set up by the exec server.
   If using cthreads, this stack is deallocated in startup.
   Not locked.  */
extern vm_address_t _hurd_stack_base;
extern vm_size_t _hurd_stack_size;

extern thread_t _hurd_msgport_thread;
extern mach_port_t _hurd_msgport; /* Locked by _hurd_siglock.  */

/* Not locked.  If we are using a real dtable, these are turned into that
   and then cleared at startup.  If not, these are never changed after
   startup.  */
extern mach_port_t *_hurd_init_dtable;
extern mach_msg_type_number_t _hurd_init_dtablesize;

/* File descriptor table.  */


/* File descriptor structure.  */
struct _hurd_fd
  {
    struct _hurd_port port;	/* io server port.  */
    int flags;			/* fcntl flags; locked by port.lock.  */

    /* Normal port to the ctty.  When `port' is our ctty, this is a port to
       the same io object but which never returns EBACKGROUND; when not,
       this is nil.  */
    struct _hurd_port ctty;
  };

/* Set up *FD to have PORT its server port, doing appropriate ctty magic.
   Does no locking or unlocking.  */
extern void _hurd_port2fd (struct _hurd_fd *fd, io_t port, int flags);

/* Allocate a new file descriptor and install PORT in it (doing any
   appropriate ctty magic); consumes a user reference on PORT.  FLAGS are
   as for `open'; only O_NOCTTY is meaningful, but all are saved.

   If the descriptor table is full, set errno, and return -1.
   If DEALLOC is nonzero, deallocate PORT first.  */
extern int _hurd_intern_fd (io_t port, int flags, int dealloc);

/* Allocate a new file descriptor and return it, locked.
   The new descriptor will not be less than FIRST_FD.  */
extern struct _hurd_fd *_hurd_alloc_fd (int *fd_ptr, int first_fd);


struct _hurd_dtable
  {
    int size;			/* Number of elts in `d' array.  */

    /* Uses of individual descriptors are not locked.  It is up to the user
       to synchronize descriptor operations on a single descriptor.  */

    struct _hurd_fd *d;
  };

#ifdef noteven
extern struct mutex _hurd_dtable_lock; /* Locks next two.  */
#endif
extern struct _hurd_dtable _hurd_dtable;
extern int _hurd_dtable_rlimit;	/* RLIM_OFILES: number of file descriptors.  */

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
       setrlimit.c gives it one element: free.
       If setrlimit is not linked in, *DEALLOC
       will never get set, so we will never get here.
       This hair avoids linking in free if we don't need it.  */
    (*_hurd_dtable_resizes.free) (dtable.d);
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

/* Returns the descriptor cell for FD, locked.  The passed DEALLOC word and
   returned structure hold onto the descriptor table to it doesn't move
   while you might be using a pointer into it.  */
static inline struct _hurd_fd_user
_hurd_fd (int fd, int *dealloc)
{
  struct _hurd_fd_user d;
  d.dtable = _hurd_dtable_use (dealloc);
  d.d = _hurd_dtable_fd (fd, d.dtable);
  if (d.d == NULL)
    _hurd_dtable_done (d.dtable, dealloc);
  return d;
}

static inline void
_hurd_fd_done (struct _hurd_fd_user d, int *dealloc)
{
  _hurd_dtable_done (d.dtable, dealloc);
}

/* Evaluate EXPR with the variable `port' bound to the port to FD,
   and `ctty' bound to the ctty port.  */
   
#define	_HURD_DPORT_USE(fd, expr)					      \
  ({ int __dealloc_dt;							      \
     error_t __result;							      \
     struct _hurd_fd_user __d = _hurd_fd (fd, &__dealloc_dt);		      \
     if (__d.d == NULL)							      \
       __result = EBADF;						      \
     else								      \
       {								      \
	 int __dealloc, __dealloc_ctty;					      \
	 io_t port = _hurd_port_locked_get (&__d.d->port, &__dealloc);	      \
	 io_t ctty = _hurd_port_locked_get (&__d.d->ctty, &__dealloc_ctty);   \
	 __result = (expr);						      \
	 _hurd_port_free (&__d.d->port, &__dealloc, port);		      \
	 if (ctty != MACH_PORT_NULL)					      \
	   _hurd_port_free (&__d.d->ctty, &__dealloc_ctty, ctty);	      \
	 _hurd_fd_done (__d, &__dealloc_dt);				      \
       }								      \
      __result;								      \
   })									      \

static inline int
__hurd_dfail (int fd, error_t err)
{
  switch (err)
    {
    case MACH_SEND_INVALID_DEST: /* The server has disappeared!  */
#ifdef notyet
      _hurd_raise_signal (NULL, SIGLOST, fd);
#else
      abort ();
#endif
      break;
    case EPIPE:
#ifdef notyet
      _hurd_raise_signal (NULL, SIGPIPE, fd);
#else
      abort ();
#endif
      break;
    default:
      return __hurd_fail (err);
    }
}

/* Return the socket server for sockaddr domain DOMAIN.  */
extern socket_t _hurd_socket_server (int domain);


/* Current process IDs.  */
extern pid_t _hurd_pid, _hurd_ppid, _hurd_pgrp;
extern int _hurd_orphaned;
#ifdef noteven
extern struct mutex _hurd_pid_lock; /* Locks above.  */
#endif


/* User and group IDs.  */
struct _hurd_id_data
  {
#ifdef noteven
    mutex_t lock;
#endif

    int valid;			/* If following data are up to date.  */

    struct
      {
	uid_t *uids;
	gid_t *gids;
	unsigned int nuids, ngids;
      } gen, aux;

    auth_t rid_auth;		/* Cache used by access.  */
  };
extern struct _hurd_id_data _hurd_id;
/* Update _hurd_id (caller should be holding the lock).  */
extern error_t _hurd_check_ids (void);


/* Unix `data break', for brk and sbrk.
   If brk and sbrk are not used, this info will not be initialized or used.  */
extern vm_address_t _hurd_brk;	/* Data break.  */
extern vm_address_t _hurd_data_end; /* End of allocated space.  */
#ifdef noteven
extern struct mutex _hurd_brk_lock; /* Locks brk and data_end.  */
#endif
extern int _hurd_set_data_limit (const struct rlimit *);

/* Set the data break; the brk lock must
   be held, and is released on return.  */
extern int _hurd_set_brk (vm_address_t newbrk);

/* Resource limit on core file size.  Enforced by hurdsig.c.  */
extern int _hurd_core_limit;

#include <signal.h>

/* Per-thread signal state.  */
struct _hurd_sigstate
  {
    /* XXX should be in cthread variable (?) */
    thread_t thread;
    struct _hurd_sigstate *next; /* Linked-list of thread sigstates.  */

#ifdef noteven
    struct mutex lock;		/* Locks the rest of this structure.  */
#endif
    sigset_t blocked;
    sigset_t pending;
    struct sigaction actions[NSIG];
    struct sigaltstack sigaltstack;
    int sigcodes[NSIG];		/* Codes for pending signals.  */

    int suspended;		/* If nonzero, sig_post signals `arrived'.  */
#ifdef noteven
    struct condition arrived;
#endif

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


#ifdef notyet
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

/* Mask of signals that cannot be caught, blocked, or ignored.  */
#define	_SIG_CANT_MASK	(__sigmask (SIGSTOP) | __sigmask (SIGKILL))

/* Do an RPC to a process's message port.

   Each argument is an expression which returns an error code; each
   expression may be evaluated several times.  FETCH_MSGPORT_EXPR should
   fetch the appropriate message port and store it in the local variable
   `msgport'.  FETCH_REFPORT_EXPR should fetch the appropriate message port
   and store it in the local variable `refport' (if no reference port is
   needed in the call, then FETCH_REFPORT_EXPR should be simply
   KERN_SUCCESS or 0).  Both of these are assumed to create user
   references, which this macro deallocates.  RPC_EXPR should perform the
   desired RPC operation using `msgport' and `refport'.

   The reason for the complexity is that a process's message port and
   reference port may change between fetching those ports and completing an
   RPC using them (usually they change only when a process execs).  The RPC
   will fail with MACH_SEND_INVALID_DEST if the msgport dies before we can
   send the RPC request; or with MIG_SERVER_DIED if the msgport was
   destroyed after we sent the RPC request but before it was serviced.  In
   either of these cases, we retry the entire operation, discarding the old
   message and reference ports and fetch them anew.  */

#define _HURD_MSGPORT_RPC(fetch_msgport_expr, fetch_refport_expr, rpc_expr)   \
({									      \
    error_t __err;							      \
    mach_port_t msgport, refport = MACH_PORT_NULL;			      \
    do									      \
      {									      \
	/* Get the message port.  */					      \
	if (__err = (fetch_msgport_expr))				      \
	  break;							      \
	/* Get the reference port.  */					      \
	if (__err = (fetch_refport_expr))				      \
	  {								      \
	    /* Couldn't get it; deallocate MSGPORT and fail.  */	      \
	    mach_port_deallocate (__mach_task_self (), msgport);	      \
	    break;							      \
	  }								      \
	__err = (rpc_expr);						      \
	mach_port_deallocate (__mach_task_self (), msgport);		      \
	if (refport != MACH_PORT_NULL)					      \
	  mach_port_deallocate (__mach_task_self (), refport);    	      \
      } while (__err != MACH_SEND_INVALID_DEST &&			      \
	       __err != MIG_SERVER_DIED);				      \
    __err;								      \
})

/* Calls to get and set basic ports.  */
extern process_t getproc (void);
extern file_t getccdir (void), getcwdir (void), getcrdir (void);
extern auth_t getauth (void);
extern int setproc (process_t);
extern int setcwdir (file_t), setcrdir (file_t);

/* Does reauth with the proc server and fd io servers.  */
extern int __setauth (auth_t), setauth (auth_t);


extern error_t __hurd_path_split (file_t crdir, file_t cwdir,
				  const char *file,
				  file_t *dir, char **name);
extern error_t hurd_path_split (file_t crdir, file_t cwdir,
				const char *file,
				file_t *dir, char **name);
extern error_t __hurd_path_lookup (file_t crdir, file_t cwdir,
				   const char *file,
				   int flags, mode_t mode,
				   file_t *port);
extern error_t hurd_path_lookup (file_t crdir, file_t cwdir,
				 const char *filename,
				 int flags, mode_t mode,
				 file_t *port);

/* Returns a port to the directory, and sets *NAME to the file name.  */
extern file_t __path_split (const char *file, char **name);
extern file_t path_split (const char *file, char **name);

/* Looks up FILE with the given FLAGS and MODE (as for dir_pathtrans).  */
extern file_t __path_lookup (const char *file, int flags, mode_t mode);
extern file_t path_lookup (const char *file, int flags, mode_t mode);

/* Open a file descriptor on a port.  */
extern int openport (io_t port, int flags);

/* Execute a file, replacing the current program image.  */
extern error_t _hurd_exec (file_t file,
			   char *const argv[],
			   char *const envp[]);

/* Inform the proc server we have exitted with STATUS, and kill the
   task thoroughly.  This function never returns, no matter what.  */
extern volatile void _hurd_exit (int status);

/* Initialize the library data structures from the
   ints and ports passed to us by the exec server.
   Then vm_deallocate PORTARRAY and INTARRAY.  */
extern void _hurd_init (int flags, char **argv,
			mach_port_t *portarray, size_t portarraysize,
			int *intarray, size_t intarraysize);

/* Do startup handshaking with the proc server.  */
extern void _hurd_proc_init (char **argv);

/* Fetch the host privileged port and device master port from the proc
   server.  They are fetched only once and then cached in the variables
   below.  A special program that gets them from somewhere other than the
   proc server (such as a bootstrap filesystem) can set these variables at
   startup to install the ports.  */
extern kern_return_t get_privileged_ports (host_priv_t *host_priv_ptr,
					   device_t *device_master_ptr);
extern mach_port_t _hurd_host_priv, _hurd_device_master;

/* Convert between PIDs and task ports.  */
extern pid_t __task2pid (task_t), task2pid (task_t);
extern task_t __pid2task (pid_t), pid2task (pid_t);

/* User-registered handlers for specific `ioctl' requests.  */

struct ioctl_handler
  {
    int first_request, last_request; /* Range of handled request values.  */

    int (*handler) (int fd, int request, void *arg);

    struct ioctl_handler *next;	/* Next handler.  */
  };

/* Define a library-internal handler for ioctl commands
   between FIRST and LAST inclusive.  */

#define	_HURD_HANDLE_IOCTLS(handler, first, last)			      \
  static const struct ioctl_handler handler##_ioctl_handler =		      \
    { first, last, handler, NULL };					      \
  text_set_element (_hurd_ioctl_handler_lists, ##handler##_ioctl_handler)

/* Define a library-internal handler for a single ioctl command.  */
#define _HURD_HANDLE_IOCTL(handler, ioctl) \
  _HURD_HANDLE_IOCTLS (handler, (ioctl), (ioctl))


#endif	/* hurd.h */
