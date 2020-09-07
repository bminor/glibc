/* Copyright (C) 2015-2021 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <sys/socket.h>
#include <time.h>
#include <sysdep.h>
#include <socketcall.h>
#include <socket-constants-time64.h>

static int
setsockopt_syscall (int fd, int level, int optname, const void *optval,
		    socklen_t len)
{
#ifdef __ASSUME_SETSOCKOPT_SYSCALL
  return INLINE_SYSCALL_CALL (setsockopt, fd, level, optname, optval, len);
#else
  return SOCKETCALL (setsockopt, fd, level, optname, optval, len);
#endif
}

#ifndef __ASSUME_TIME64_SYSCALLS
static int
setsockopt32 (int fd, int level, int optname, const void *optval,
	      socklen_t len)
{
  int r = -1;

  if (level != SOL_SOCKET)
    return r;

  switch (optname)
    {
    case COMPAT_SO_RCVTIMEO_NEW:
    case COMPAT_SO_SNDTIMEO_NEW:
      {
        if (len < sizeof (struct __timeval64))
	  {
	    __set_errno (EINVAL);
	    break;
	  }

	struct __timeval64 *tv64 = (struct __timeval64 *) optval;
	if (! in_time_t_range (tv64->tv_sec))
	  {
	    __set_errno (EOVERFLOW);
	    break;
	  }

	if (optname == COMPAT_SO_RCVTIMEO_NEW)
	  optname = COMPAT_SO_RCVTIMEO_OLD;
	if (optname == COMPAT_SO_SNDTIMEO_NEW)
	  optname = COMPAT_SO_SNDTIMEO_OLD;

	struct __timeval32 tv32 = valid_timeval64_to_timeval32 (*tv64);

	r = setsockopt_syscall (fd, level, optname, &tv32, sizeof (tv32));
      }
    }

  return r;
}
#endif

int
setsockopt (int fd, int level, int optname, const void *optval, socklen_t len)
{
  int r = setsockopt_syscall (fd, level, optname, optval, len);

#ifndef __ASSUME_TIME64_SYSCALLS
  if (r == -1 && errno == ENOPROTOOPT)
    r = setsockopt32 (fd, level, optname, optval, len);
#endif

  return r;
}
weak_alias (setsockopt, __setsockopt)
