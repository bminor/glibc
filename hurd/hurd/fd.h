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


/* Structure representing a file descriptor.  */

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
    struct hurd_fd **d;
  };

/* Current file descriptor table.  */

extern struct hurd_dtable _hurd_dtable;

/* If not NULL, points to the chain of users of `_hurd_dtable'.
   See <hurd/userlink.h>.  */

extern struct hurd_userlink *_hurd_dtable_users;

extern int _hurd_dtable_rlimit;	/* RLIM_OFILES: number of file descriptors.  */

/* This locks _hurd_dtable, _hurd_dtable_users, and _hurd_dtable_rlimit.  */
extern struct mutex _hurd_dtable_lock;

#include <lock-intern.h>

#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif

/* Get a descriptor table structure to use.
   Pass this structure and ULINK to _hurd_dtable_free when done.  */

_EXTERN_INLINE struct hurd_dtable
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

_EXTERN_INLINE void
_hurd_dtable_free (struct hurd_dtable dtable,
		   struct hurd_userlink *ulink)
{
  int dealloc;
  __mutex_lock (&_hurd_dtable_lock);
  dealloc = _hurd_userlink_unlink (ulink);
  __mutex_unlock (&_hurd_dtable_lock);
  if (dealloc && _hurd_dtable_deallocate)
    (*_hurd_dtable_deallocate) (dtable.d);
}


/* Return the descriptor cell for FD in DTABLE, locked.
   If FD is invalid or unused, return NULL.  */

_EXTERN_INLINE struct hurd_fd *
_hurd_dtable_fd (int fd, struct hurd_dtable dtable)
{
  if (fd < 0 || fd >= dtable.size)
    return NULL;
  else
    {
      struct hurd_fd *cell = dtable.d[fd];
      if (cell == NULL)
	/* No descriptor allocated at this index.  */
	return NULL;
      __spin_lock (&cell->port.lock);
      if (cell->port.port == MACH_PORT_NULL)
	{
	  /* The descriptor at this index has no port in it.
	     This happens if it existed before but was closed.  */
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

/* Returns the descriptor cell for FD, locked.  The passed ULINK structure
   and returned structure hold onto the descriptor table to it doesn't move
   while you might be using a pointer into it.  */

_EXTERN_INLINE struct hurd_fd_user
_hurd_fd_get (int fd, struct hurd_userlink *ulink)
{
  struct hurd_fd_user d;
  d.dtable = _hurd_dtable_get (ulink);
  d.d = _hurd_dtable_fd (fd, d.dtable);
  if (d.d == NULL)
    _hurd_dtable_free (d.dtable, ulink);
  return d;
}

/* Free a reference gotten with `D = _hurd_fd_get (FD, ULINK);'.
   The descriptor cell D.d should be unlocked before calling this function.  */

_EXTERN_INLINE void
_hurd_fd_free (struct hurd_fd_user d, struct hurd_userlink *ulink)
{
  _hurd_dtable_free (d.dtable, ulink);
}


/* Evaluate EXPR with the variable `descriptor' bound to a pointer to the
   locked file descriptor structure for FD.  */

#define	HURD_FD_USE(fd, expr)						      \
  ({ struct hurd_userlink __dt_ulink;					      \
     error_t __result;							      \
     struct hurd_fd_user __d = _hurd_fd_get (fd, &__dt_ulink);		      \
     if (__d.d == NULL)							      \
       __result = EBADF;						      \
     else								      \
       {								      \
	 struct hurd_fd *const descriptor = __d.d;			      \
	 __result = (expr);						      \
	 _hurd_fd_free (__d, &__dt_ulink);				      \
       }								      \
     __result; })

/* Evaluate EXPR with the variable `port' bound to the port to FD,
   and `ctty' bound to the ctty port.  */

#define HURD_DPORT_USE(fd, expr) \
  HURD_FD_USE ((fd), HURD_FD_PORT_USE (descriptor, (expr)))

/* Likewise, but FD is a pointer to the locked file descriptor structure.  */

#define	HURD_FD_PORT_USE(fd, expr)					      \
  ({ error_t __result;							      \
     struct hurd_fd *const __d = (fd);					      \
     struct hurd_userlink __ulink, __ctty_ulink;			      \
     io_t port = _hurd_port_locked_get (&__d->port, &__ulink);		      \
     io_t ctty = _hurd_port_locked_get (&__d->ctty, &__ctty_ulink);	      \
     __result = (expr);							      \
     _hurd_port_free (&__d->port, &__ulink, port);			      \
     if (ctty != MACH_PORT_NULL)					      \
       _hurd_port_free (&__d->ctty, &__ctty_ulink, ctty);		      \
     __result; })

#include <errno.h>
#include <hurd/signal.h>

/* Handle an error from an RPC on a file descriptor's port.  You should
   always use this function to handle errors from RPCs made on file
   descriptor ports.  Some errors are translated into signals.  */   

_EXTERN_INLINE error_t
_hurd_fd_error (int fd, error_t err)
{
  switch (err)
    {
    case MACH_SEND_INVALID_DEST: /* The server has disappeared!  */
      _hurd_raise_signal (NULL, SIGLOST, fd);
      break;
    case EPIPE:
      _hurd_raise_signal (NULL, SIGPIPE, fd);
      break;
    }
  return err;
}

/* Handle error code ERR from an RPC on file descriptor FD's port.
   Set `errno' to the appropriate error code, and always return -1.  */

_EXTERN_INLINE int
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
   as for `open'; only O_IGNORE_CTTY is meaningful, but all are saved.

   If the descriptor table is full, set errno, and return -1.
   If DEALLOC is nonzero, deallocate PORT first.  */

extern int _hurd_intern_fd (io_t port, int flags, int dealloc);

/* Allocate a new file descriptor in the table and return it, locked.  The
   new descriptor number will be no less than FIRST_FD.  If the table is
   full, set errno to EMFILE and return NULL.  If FIRST_FD is negative or
   bigger than the size of the table, set errno to EINVAL and return NULL.  */

extern struct hurd_fd *_hurd_alloc_fd (int *fd_ptr, int first_fd);

/* Allocate a new file descriptor structure and initialize its port cells
   with PORT and CTTY.  (This does not affect the descriptor table.)  */

extern struct hurd_fd *_hurd_new_fd (io_t port, io_t ctty);

/* Read and write data from a file descriptor; just like `read' and `write'.
   If successful, stores the amount actually read or written in *NBYTES.  */

extern error_t _hurd_fd_read (struct hurd_fd *fd, void *buf, size_t *nbytes);
extern error_t _hurd_fd_write (struct hurd_fd *fd,
			       const void *buf, size_t *nbytes);


/* User-registered handlers for specific `ioctl' requests.  */

#define	__need___va_list
#include <stdarg.h>

/* Structure that records an ioctl handler.  */

struct ioctl_handler
  {
    int first_request, last_request; /* Range of handled request values.  */

    int (*handler) (int fd, int request, __gnuc_va_list);

    struct ioctl_handler *next;	/* Next handler.  */
  };


/* Register HANDLER to handle ioctls with REQUEST values between
   FIRST_REQUEST and LAST_REQUEST inclusive.  Returns zero if successful.
   Return nonzero and sets `errno' for an error.  */

extern int hurd_register_ioctl_handler (int first_request, int last_request,
					int (*handler) (int fd, int request,
							__gnuc_va_list));


/* Define a library-internal handler for ioctl commands
   between FIRST and LAST inclusive.  */

#define	_HURD_HANDLE_IOCTLS(handler, first, last)			      \
  static const struct ioctl_handler handler##_ioctl_handler =		      \
    { first, last, handler, NULL };					      \
  text_set_element (_hurd_ioctl_handler_lists, ##handler##_ioctl_handler)

/* Define a library-internal handler for a single ioctl command.  */

#define _HURD_HANDLE_IOCTL(handler, ioctl) \
  _HURD_HANDLE_IOCTLS (handler, (ioctl), (ioctl))


#endif	/* hurd/fd.h */
