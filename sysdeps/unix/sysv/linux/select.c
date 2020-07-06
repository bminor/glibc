/* Linux select implementation.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <sysdep-cancel.h>
#include <time64-support.h>

/* Check the first NFDS descriptors each in READFDS (if not NULL) for read
   readiness, in WRITEFDS (if not NULL) for write readiness, and in EXCEPTFDS
   (if not NULL) for exceptional conditions.  If TIMEOUT is not NULL, time out
   after waiting the interval specified therein.  Returns the number of ready
   descriptors, or -1 for errors.  */

int
__select64 (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	    struct __timeval64 *timeout)
{
  struct __timespec64 ts64, *pts64 = NULL;
  if (timeout != NULL)
    {
      ts64 = timeval64_to_timespec64 (*timeout);
      pts64 = &ts64;
    }

#ifndef __NR_pselect6_time64
# define __NR_pselect6_time64 __NR_pselect6
#endif
  int r;
  if (supports_time64 ())
    {
      r = SYSCALL_CANCEL (pselect6_time64, nfds, readfds, writefds, exceptfds,
			  pts64, NULL);
      /* Linux by default will update the timeout after a pselect6 syscall
         (though the pselect() glibc call suppresses this behavior).
         Since select() on Linux has the same behavior as the pselect6
         syscall, we update the timeout here.  */
      if (r == 0 || errno != ENOSYS)
	{
	  if (timeout != NULL)
	    TIMEVAL_TO_TIMESPEC (timeout, &ts64);
	  return r;
	}

      mark_time64_unsupported ();
    }

#ifndef __ASSUME_TIME64_SYSCALLS
  struct timespec ts32, *pts32 = NULL;
  if (timeout != NULL)
    {
      if (! in_time_t_range (timeout->tv_sec))
	{
	  __set_errno (EINVAL);
	  return -1;
	}
      ts32 = valid_timespec64_to_timespec (ts64);
      pts32 = &ts32;
    }
# ifndef __ASSUME_PSELECT
#  ifdef __NR__newselect
#   undef __NR_select
#   define __NR_select __NR__newselect
#  endif
  r = SYSCALL_CANCEL (select, nfds, readfds, writefds, exceptfds, pts32);
# else
  r = SYSCALL_CANCEL (pselect6, nfds, readfds, writefds, exceptfds, pts32,
		      NULL);
# endif
  if (r >= 0 && timeout != NULL)
    *timeout = valid_timespec_to_timeval64 (ts32);
#endif

  return r;
}

#if __TIMESIZE != 64
libc_hidden_def (__select64)

int
__select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	  struct timeval *timeout)
{
  struct __timeval64 tv64, *ptv64 = NULL;
  if (timeout != NULL)
    {
      tv64 = valid_timeval_to_timeval64 (*timeout);
      ptv64 = &tv64;
    }
  int r = __select64 (nfds, readfds, writefds, exceptfds, ptv64);
  if (r >= 0 && timeout != NULL)
    /* The remanining timeout will be always less the input TIMEOUT.  */
    *timeout = valid_timeval64_to_timeval (tv64);
  return r;
}
#endif
libc_hidden_def (__select)

weak_alias (__select, select)
weak_alias (__select, __libc_select)
