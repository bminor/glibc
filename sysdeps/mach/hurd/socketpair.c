/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
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
#include <hurd/socket.h>
#include <hurd/fd.h>
#include <unistd.h>

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
int
DEFUN(socketpair, (domain, type, protocol, fds),
      int domain AND enum __socket_type type AND int protocol AND int fds[2])
{
  error_t err;
  int d1, d2;

  d1 = socket (domain, type, protocol);
  if (d1 < 0)
    return -1;
  d2 = socket (domain, type, protocol);
  if (d2 < 0)
    {
      err = errno;
      (void) close (d1);
      return __hurd_fail (err);
    }

  if (err = HURD_DPORT_USE
      (d1,
       ({ socket_t sock1 = port;
	  HURD_DPORT_USE (d2, __socket_connect2 (sock1, port)); })))
    {
      close (d1);
      close (d2);
      return __hurd_fail (err);
    }

  fds[0] = d1;
  fds[2] = d2;
  return 0;
}
