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
#include <sys/socket.h>
#include <hurd.h>

/* Put the local address of FD into *ADDR and its length in *LEN.  */
int
DEFUN(getsockname, (fd, addr, len),
      int fd AND struct sockaddr *addr AND size_t *len)
{
  return _HURD_DPORT_USE
    (fd,
     ({
       error_t err;
       addr_port_t aport;
       err = __socket_name (port, &aport);
       if (!err)
	 {
	   err = __socket_whatis_address (aport, addr, len);
	   __mach_port_deallocate (__mach_task_self (), aport);
	 }
       err ? __hurd_dfail (fd, err) : 0;
     }));
}
