/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.  */
int
DEFUN(accept, (fd, addr, addr_len),
      int fd AND struct sockaddr *addr AND size_t *addr_len)
{
  socket_t new;
  addr_port_t aport;
  int d;
  error_t err = _HURD_DPORT_USE (fd, __socket_accept (port, &new, &aport));
  if (err)
    return __hurd_dfail (fd, err);
  if (addr != NULL)
    err = __socket_whatis_address (aport, addr, addr_len);
  __mach_port_deallocate (__mach_task_self (), aport);
  if (err)
    {
      __mach_port_deallocate (__mach_task_self (), new);
      return __hurd_dfail (fd, err);
    }

  return _hurd_intern_fd (new, 0, 1);
}
