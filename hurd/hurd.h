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

#define	__hurd_errno(err)	???
#define	__hurd_fail(err)	(errno = __hurd_errno (err), -1)


/* Basic ports and info, set by startup.  */
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
    struct mutex lock;		/* Locks the table.  */
    int size;
    struct
      {
	io_t server;
	int isctty;		/* Is a port to the controlling tty.  */
	int flags;		/* fcntl flags.  */

	/* References to `server'.
	   Just being live counts as one ref.
	   When you decrement `refs' and it becomes zero,
	   you should do a condition_broadcast on `free'.  */
	size_t refs;
	struct condition free;
	spin_lock_t reflock;	/* Locks refs.  */
      } *d;
  };
extern struct _hurd_dtable _hurd_dtable;

/* Allocate a new file descriptor.  The dtable lock must be held.  */
static inline int
_hurd_dalloc (void)
{
  int i;
  for (i = 0; i < _hurd_dtable.size; ++i)
    if (_hurd_dtable.d[i] == MACH_PORT_NULL)
      {
	_hurd_dtable.d[i].isctty = -1;
	return i;
      }
  errno = EMFILE;
  return -1;
}

/* Return the server port for FD, giving it a ref.
   On error, errno is set and MACH_PORT_NULL is returned.  */
static inline file_t
_hurd_dport (int fd)
{
  __mutex_lock (&_hurd_dtable.lock);
  if (fd < 0 || fd >= _hurd_dtable.size ||
      _hurd_dtable.d[fd].server == MACH_PORT_NULL)
    {
      __mutex_unlock (&_hurd_dtable.lock);
      errno = EBADF;
      return MACH_PORT_NULL;
    }
  __spin_lock (&_hurd_dtable.d[fd].reflock);
  ++_hurd_dtable.d[fd].refs;
  __spin_unlock (&_hurd_dtable.d[fd].reflock);
  return _hurd_dtable.d[fd].server;
}

static inline void
_hurd_dfree (int fd)
{
}


/* Return nonzero if FD is a descriptor to our controlling terminal.
   FD must be locked.  */
static __inline int
_hurd_ctty_check (int fd)
{
  extern int _hurd_hasctty;
  if (!_hurd_hasctty)
    return 0;
  if (_hurd_dtable.d[fd].isctty == -1)
    {
      io_statbuf_t stb;
      _hurd_dtable.d[fd].isctty
	= (!__io_stat (_hurd_dtable.d[fd].server, &stb) &&
	   stb.stb_fstype = _hurd_ctty_fstype &&
	   stb.stb_fsid.val[0] = _hurd_ctty_fsid.val[0] &&
	   stb.stb_fsid.val[1] = _hurd_ctty_fsid.val[1] &&
	   stb.stb_file_id = _hurd_ctty_fileid);
    }
  return _hurd_dtable.d[fd].isctty;
}


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
extern mutex_t _hurd_brk_lock;	/* Locks brk and data_end.  */
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
    struct _hurd_sigstate *next;

    struct mutex lock;
    sigset_t blocked;
    sigset_t pending;
    struct sigaction actions[NSIG];
    struct sigstack sigstack;
    int sigcodes[NSIG];		/* Codes for pending signals.  */

    int suspended;		/* If nonzero, sig_post signals ARRIVED.  */
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
      } vfork_saved;

    /* Not locked.  Used only by this thread,
       or by signal thread with this thread suspended.  */
    mach_port_t intr_port;	/* Port an interruptible RPC was sent on.  */
    int intr_is_wait;		/* Interruptible RPC was wait, not io.  */
    int intr_restart;		/* If nonzero, restart interrupted RPC.  */
  };
extern struct _hurd_sigstate *_hurd_sigstates;
extern mutex_t _hurd_siglock;	/* Locks _hurd_sigstates.  */
extern struct _hurd_sigstate *_hurd_thread_sigstate (thread_t);

/* Thread to receive process-global signals.  */
extern thread_t _hurd_sigthread;

/* Called by the machine-dependent exception handler.  */
extern void _hurd_exc_post_signal (thread_t, int sig, int code);

/* SS->lock is held on entry, and released before return.  */
extern void _hurd_internal_post_signal (reply_port_t,
					struct _hurd_sigstate *ss,
					int signo, int sigcode,
					int orphaned, int cttykill,
					int *willstop);

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
    __mutex_unlock (&ss->lock);	/* Lock not needed.  */
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
    ss->intr_port = MACH_PORT_NULL;
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

/* Returns the directory.  */
extern file_t __hurd_path_split (const char *path, const char **name);
#define	hurd_path_split	__hurd_path_split

extern file_t __hurd_path_lookup (const char *path, int flags, mode_t mode);
#define hurd_path_lookup __hurd_path_lookup


#endif	/* hurd.h */
