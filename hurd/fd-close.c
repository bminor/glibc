/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd/fd.h>

/* FD is locked.  Close and unlock it, so it can be reallocated.  */

error_t
_hurd_fd_close (struct hurd_fd *fd)
{
  /* Clear the descriptor's port cells.
     This deallocates the ports if noone else is still using them.  */

  _hurd_port_set (&fd->ctty, MACH_PORT_NULL);
  _hurd_port_locked_set (&fd->port, MACH_PORT_NULL);

  return 0;
}
