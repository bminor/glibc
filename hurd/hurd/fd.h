/* File descriptors.
Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

#ifndef	_HURD_FD_H

#define	_HURD_FD_H	1
#include <features.h>

#include <hurd/hurd_types.h>
#include <hurd/port.h>


/* File descriptor structure.  */

struct hurd_fd
  {
    struct hurd_port port;	/* io server port.  */
    int flags;			/* fcntl flags; locked by port.lock.  */

    /* Normal port to the ctty.  When `port' is our ctty, this is a port to
       the same io object but which never returns EBACKGROUND; when not,
       this is nil.  */
    struct hurd_port ctty;
  };


/* Structure representing a file descriptor table.  */

struct hurd_dtable
  {
    int size;			/* Number of elts in `d' array.  */

    /* Uses of individual descriptors are not locked.  It is up to the user
       to synchronize descriptor operations on a single descriptor.  */
    struct hurd_fd *d;
  };

/* Current file descriptor table.  */

extern struct hurd_dtable _hurd_dtable;

/* If not NULL, points to the chain of users of `_hurd_dtable'.
   See <hurd/port.h>.  */

extern struct hurd_userlink *_hurd_dtable_users;

extern int _hurd_dtable_rlimit;	/* RLIM_OFILES: number of file descriptors.  */

/* This locks _hurd_dtable, _hurd_dtable_users, and _hurd_dtable_rlimit.  */
#ifdef noteven
extern struct mutex _hurd_dtable_lock;
#endif


/* Get a descriptor table structure to use.
   Pass this structure and ULINK to _hurd_dtable_free when done.  */

extern inline struct hurd_dtable
_hurd_dtable_get (struct hurd_userlink *ulink)
{
  struct hurd_dtable dtable;
  __mutex_lock (&_hurd_dtable_lock);
  _hurd_userlink_link (&_hurd_dtable_users, ulink);
  dtable = _hurd_dtable;
  __mutex_unlock (&_hurd_dtable_lock);
  return dtable;
}



/* Function to deallocate a descriptor table's `struct hurd_fd' array.
   This is expected to be either `free' or a null pointer.  */

extern void (*_hurd_dtable_deallocate) (void *);

/* Free a reference gotten with `DTABLE = _hurd_dtable_get (ULINK);' */

static inline void
_hurd_dtable_free (struct hurd_dtable dtable,
		   struct hurd_userlink *ulink)
{
  int dealloc;
  __mutex_lock (&_hurd_dtable_lock);
  dealloc = _hurd_userlink_unlink (ulink);
  __mutex_unlock (&_hurd_dtable_lock);
  if (dealloc && _hurd_dtable_deallocate)
    (*_hurd_dtable_deallocate) (dtable.d);
    
    /* _hurd_dtable_resizes is a symbol set.
       setrlimit.c gives it one element: free.
       If setrlimit is not linked in, *DEALLOC
       will never get set, so we will never get here.
       This hair avoids linking in free if we don't need it.  */
    (*_hurd_dtable_resizes.free) (dtable.d);
}


/* Return the descriptor cell for FD in DTABLE, locked.
   If FD is invalid or unused, return NULL.  */

extern inline struct hurd_fd *
_hurd_dtable_fd (int fd, struct hurd_dtable dtable)
{
  if (fd < 0 || fd >= dtable.size)
    return NULL;
  else
    {
      struct hurd_fd *cell = &dtable.d[fd];
      __spin_lock (&cell->port.lock);
      if (cell->port.port == MACH_PORT_NULL)
	{
	  __spin_unlock (&cell->port.lock);
	  return NULL;
	}
      return cell;
    }
}

struct hurd_fd_user
  {
    struct hurd_dtable dtable;
    struct hurd_fd *d;
  };

/* Returns the descriptor cell for FD, locked.  The passed DEALLOC word and
   returned structure hold onto the descriptor table to it doesn't move
   while you might be using a pointer into it.  */
static inline struct hurd_fd_user
_hurd_fd (int fd, int *dealloc)
{
  struct hurd_fd_user d;
  d.dtable = _hurd_dtable_use (dealloc);
  d.d = _hurd_dtable_fd (fd, d.dtable);
  if (d.d == NULL)
    _hurd_dtable_done (d.dtable, dealloc);
  return d;
}

static inline void
_hurd_fd_done (struct hurd_fd_user d, int *dealloc)
{
  _hurd_dtable_done (d.dtable, dealloc);
}


extern io_t __getdport (int);
extern io_t getdport (int);


/* Handle an error from an RPC on a file descriptor's port.  You should
   always use this function to handle errors from RPCs made on file
   descriptor ports.  Some errors are translated into signals.  */   

extern inline error_t
_hurd_fd_error (int fd, error_t err)
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
    }
  return err;
}

/* Handle error code ERR from an RPC on file descriptor FD's port.
   Set `errno' to the appropriate error code, and always return -1.  */

extern inline int
__hurd_dfail (int fd, error_t err)
{
  errno = _hurd_fd_error (fd, err);
  return -1;
}



/* Set up *FD to have PORT its server port, doing appropriate ctty magic.
   Does no locking or unlocking.  */

extern void _hurd_port2fd (struct hurd_fd *fd, io_t port, int flags);

/* Allocate a new file descriptor and install PORT in it (doing any
   appropriate ctty magic); consumes a user reference on PORT.  FLAGS are
   as for `open'; only O_NOCTTY is meaningful, but all are saved.

   If the descriptor table is full, set errno, and return -1.
   If DEALLOC is nonzero, deallocate PORT first.  */

extern int _hurd_intern_fd (io_t port, int flags, int dealloc);

/* Allocate a new file descriptor and return it, locked.
   The new descriptor will not be less than FIRST_FD.  */
extern struct hurd_fd *_hurd_alloc_fd (int *fd_ptr, int first_fd);


/* Evaluate EXPR with the variable `port' bound to the port to FD,
   and `ctty' bound to the ctty port.  */
   
#define	_HURD_DPORT_USE(fd, expr)					      \
  ({ int __dealloc_dt;							      \
     error_t __result;							      \
     struct hurd_fd_user __d = _hurd_fd (fd, &__dealloc_dt);		      \
     if (__d.d == NULL)							      \
       __result = EBADF;						      \
     else								      \
       {								      \
         struct hurd_port_userlink __ulink, __ctty_ulink;		      \
	 io_t port = _hurd_port_locked_get (&__d.d->port, &__ulink);	      \
	 io_t ctty = _hurd_port_locked_get (&__d.d->ctty, &__ctty_ulink);     \
	 __result = (expr);						      \
	 _hurd_port_free (&__d.d->port, &__ulink, port);		      \
	 if (ctty != MACH_PORT_NULL)					      \
	   _hurd_port_free (&__d.d->ctty, &__ctty_ulink, ctty);		      \
	 _hurd_fd_done (__d, &__dealloc_dt);				      \
       }								      \
      __result;								      \
   })									      \


#endif	/* hurd/fd.h */
