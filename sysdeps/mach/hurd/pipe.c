/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <stddef.h>
#include <hurd.h>
#include <sys/socket.h>

/* Create a one-way communication channel (__pipe).
   If successul, two file descriptors are stored in PIPEDES;
   bytes written on PIPEDES[1] can be read from PIPEDES[0].
   Returns 0 if successful, -1 if not.  */
int
DEFUN(__pipe, (pipedes), int pipedes[2])
{
  socket_t skt, r, w;
  int fd0, fd1;
  error_t err;

  if (pipedes == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  ((volatile int *) pipedes)[0] = pipedes[1];

  skt = _hurd_socket_server (AF_LOCAL);
  if (err = __socket_create (skt, SOCK_STREAM, 0, &r))
    return __hurd_fail (err);
  __socket_shutdown (r, 1);
  if (err = __socket_create (skt, SOCK_STREAM, 0, &w))
    {
      __mach_port_deallocate (__mach_task_self (), r);
      return __hurd_fail (err);
    }
  __socket_shutdown (w, 0);

  __mutex_lock (&_hurd_dtable.lock);
  fd0 = _hurd_dalloc ();
  fd1 = _hurd_dalloc ();
  if (fd0 < 0 || fd0 < 0)
    {
      __mach_port_deallocate (__mach_task_self (), r);
      __mach_port_deallocate (__mach_task_self (), w);
    }
  else
    {
      _hurd_dtable.d[fd0].server = r;
      _hurd_dtable.d[fd1].server = w;
    }
  __mutex_unlock (&_hurd_dtable.lock);

  pipedes[0] = fd0;
  pipedes[1] = fd1;
  return 0;
}
