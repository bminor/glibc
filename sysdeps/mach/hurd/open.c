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

#include <ansidecl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <hurd.h>

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int
DEFUN(__open, (file, oflag), CONST char *file AND int oflag DOTS)
{
  error_t err;
  mode_t mode;
  int fl;
  io_t port, ctty;

  switch (oflag & O_ACCMODE)
    {
    case O_RDONLY:
      fl = FS_LOOKUP_READ;
      break;
    case O_WRONLY:
      fl = FS_LOOKUP_WRITE;
      break;
    case O_RDWR:
      fl = FS_LOOKUP_READ | FS_LOOKUP_WRITE;
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
      fl |= FS_LOOKUP_CREATE;
    }
  else
    mode = 0;

  if (oflag & O_NDELAY)
    fl |= FS_LOOKUP_NDELAY;
  if (oflag & O_APPEND)
    fl |= FS_LOOKUP_APPEND;
  if (oflag & O_CREATE)
    fl |= FS_LOOKUP_CREATE;
  if (oflag & O_TRUNC)
    fl |= FS_LOOKUP_TRUNC;
  if (oflag & O_EXCL)
    fl |= FS_LOOKUP_EXCL;

  port = __path_lookup (file, fl, mode);
  if (port == MACH_PORT_NULL)
    return -1;

  ctty = MACH_PORT_NULL;
  if (!(oflag & O_NOCTTY))
    {
      io_statbuf_t stb;
      
      err = __io_stat (port, &stb);
      if (!err)
	{
	  io_t fg_port = MACH_PORT_NULL;

	  __mutex_lock (&_hurd_ctty_lock);
	  if (_hurd_ctty_fstype == 0)
	    {
	      /* We have no controlling tty.
		 Try to make this it.  */
	      mach_port_t cttyid;
	      err = __term_getctty (port, &cttyid);
	      if (!err)
		{
		  err = __term_become_ctty (port, _hurd_pid, _hurd_pgrp,
					    _hurd_sigport, &fg_port);
		  if (err)
		    __mach_port_deallocate (__mach_task_self (), cttyid);
		}
	    }
	  else if (stb.stb_fstype == _hurd_ctty_fstype &&
		   stb.stb_fsid.val[0] == _hurd_ctty_fsid.val[0] &&
		   stb.stb_fsid.val[1] == _hurd_ctty_fsid.val[1] &&
		   stb.stb_fileno == _hurd_ctty_fileno)
	    /* This is our controlling tty.  */
	    err = __term_become_ctty (port, _hurd_pid, _hurd_pgrp,
				      _hurd_sigport, &fg_port);

	  if (fg_port != MACH_PORT_NULL)
	    {
	      int fd = _hurd_dalloc (fg_port, port, 0);
	      if (fd >= 0 && _hurd_ctty_fstype == 0)
		{
		  /* We have a new controlling tty.  */
		  _hurd_ctty_port = cttyid;
		  _hurd_ctty_fstype = stb.stb_fstype;
		  _hurd_ctty_fsid = stb.stb_fsid;
		  _hurd_ctty_fileno = stb.stb_fileno;
		}
	      __mutex_unlock (&_hurd_ctty_lock);
	      return fd;
	    }
	  else
	    __mutex_unlock (&_hurd_ctty_lock);
	}
    }

  return _hurd_dalloc (port, MACH_PORT_NULL, 0);
}
