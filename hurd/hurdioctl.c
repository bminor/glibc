/* ioctl commands which must be done in the C library.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <sys/ioctl.h>

/* Symbol set of ioctl handler lists.  This definition is here so that when
   __ioctl refers to it, we will link in fionread et al (below).  */

const struct
  {
    size_t n;
    struct ioctl_handler *v[0];
  } _hurd_ioctl_handler_lists;

/* Find out how many bytes may be read from FD without blocking.  */

static int
fionread (int fd,
	  int request,		/* Always FIONREAD.  */
	  void *arg)
{
  error_t err;
  int amount;

  if (err = HURD_DPORT_USE (fd, __io_readable (port, &amount)))
    return __hurd_fail (err);

  *(int *) arg = amount;
  return 0;
}

_HURD_HANDLE_IOCTL (fionread, FIONREAD);

/* Make FD be the controlling terminal.
   This function is called for `ioctl (fd, TCIOSCTTY)'.  */

static int
tiocsctty (int fd,
	   int request,		/* Always TCIOSCTTY.  */
	   void *arg)		/* Not used.  */
{
  mach_port_t cttyid;
  error_t err;

  /* Get FD's cttyid port, unless it is already ours.  */
  err = HURD_DPORT_USE (fd,
			ctty ? EADDRINUSE : __term_getctty (port, &cttyid));
  if (err == EADDRINUSE)
    /* FD is already the ctty.  Nothing to do.  */
    return 0;
  else if (err)
    return __hurd_fail (err);

  /* Make it our own.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], cttyid); /* Consumes ref.  */

  /* Reset all the ctty ports in all the descriptors.  */
  __USEPORT (CTTYID, (rectty_dtable (port), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocsctty, TIOCSCTTY);

/* Dissociate from the controlling terminal.  */

static int
tiocnotty (int fd,
	   int request,		/* Always TIOCNOTTY.  */
	   void *arg)		/* Not used.  */
{
  mach_port_t fd_cttyid;
  error_t err;

  if (err = HURD_DPORT_USE (fd, __term_getctty (port, &fd_cttyid)))
    return __hurd_fail (err);

  if (__USEPORT (CTTYID, port != fd_cttyid))
    err = EINVAL;

  __mach_port_deallocate (__mach_task_+self (), fd_cttyid);

  if (err)
    return __hurd_fail (err);

  /* Clear our cttyid port cell.  */
  _hurd_port_set (&_hurd_ports[INIT_PORT_CTTYID], MACH_PORT_NULL);

  /* Reset all the ctty ports in all the descriptors.  */
				
  __USEPORT (CTTYID, (rectty_dtable (MACH_PORT_NULL), 0));

  return 0;
}
_HURD_HANDLE_IOCTL (tiocnotty, TIOCNOTTY);
