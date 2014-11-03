/* Copyright (C) 2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SOCKET_CLOEXEC_H
#define _SOCKET_CLOEXEC_H

#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

/* Like socket, but with SOCK_CLOEXEC set if available.  If it's not,
   try to set the FD_CLOEXEC flag, and if that fails, close the socket
   and fail unless __tolerant.  */
static inline int
__socket_cloexec (int __domain, int __type, int __protocol, bool __tolerant)
{
  int __ret;
#ifdef SOCK_CLOEXEC
# ifdef __ASSUME_SOCK_CLOEXEC
  int  __have_sock_cloexec = 1;
# endif /* __ASSUME_SOCK_CLOEXEC */
  if (__have_sock_cloexec >= 0)
    {
      __ret = __socket (__domain, __type | SOCK_CLOEXEC, __protocol);
      if (__have_sock_cloexec == 0)
	__have_sock_cloexec = (__ret == -1 && errno == EINVAL ? -1 : 1);
      if (__have_sock_cloexec > 0)
	return __ret;
    }
#endif /* SOCK_CLOEXEC */
  __ret = __socket (__domain, __type, __protocol);

  if ((__ret >= 0 && __fcntl (__ret, F_SETFD, FD_CLOEXEC) < 0) && !__tolerant)
    {
      __close (__ret);
      __ret = -1;
    }

  return __ret;
}

/* Like socket, but with SOCK_CLOEXEC and SOCK_NONBLOCK set if
   SOCK_CLOEXEC is available.  SOCK_NONBLOCK is supported iff
   SOCK_CLOEXEC is.

   If SOCK_CLOEXEC is not supported, try to set the FD_CLOEXEC flag,
   and if that fails, close the socket and fail unless __tolerant.

   If SOCK_NONBLOCK is not supported, try to set the O_NONBLOCK flag,
   and if that fails, close the socket and fail REGARDLESS of
   __tolerant.  */
static inline int
__socket_cloexec_nonblock (int __domain, int __type, int __protocol,
			   bool __tolerant)
{
  int __ret;
#ifdef SOCK_NONBLOCK
# ifdef __ASSUME_SOCK_CLOEXEC
  int  __have_sock_cloexec = 1;
# endif /* __ASSUME_SOCK_CLOEXEC */
  if (__have_sock_cloexec >= 0)
    {
      __ret = __socket (__domain, __type | SOCK_CLOEXEC | SOCK_NONBLOCK,
			__protocol);
      if (__have_sock_cloexec == 0)
	__have_sock_cloexec = (__ret == -1 && errno == EINVAL ? -1 : 1);
      if (__have_sock_cloexec > 0)
	return __ret;
    }
#endif /* SOCK_NONBLOCK */
  __ret = __socket (__domain, __type, __protocol);
  if (__ret >= 0)
    {
      int __fl = __fcntl (__ret, F_GETFL);
      if (__fl == -1
	  || __fcntl (__ret, F_SETFL, __fl | O_NONBLOCK) < 0
	  || (__fcntl (__ret, F_SETFD, FD_CLOEXEC) < 0 && !__tolerant))
	{
	  __close (__ret);
	  __ret = -1;
	}
    }
  return __ret;
}

#pragma poison __socket
#pragma poison socket

#endif /* _SOCKET_CLOEXEC_H */
