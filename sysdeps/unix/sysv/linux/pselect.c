/* Copyright (C) 2006-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2006.

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

#include <sys/select.h>
#include <sysdep-cancel.h>
#include <time64-support.h>

int
__pselect64 (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	     const struct __timespec64 *timeout, const sigset_t *sigmask)
{
  /* The Linux kernel can in some situations update the timeout value.
     We do not want that so use a local variable.  */
  struct __timespec64 tval;
  if (timeout != NULL)
    {
      tval = *timeout;
      timeout = &tval;
    }

  /* Note: the system call expects 7 values but on most architectures
     we can only pass in 6 directly.  If there is an architecture with
     support for more parameters a new version of this file needs to
     be created.  */

#ifndef __NR_pselect6_time64
# define __NR_pselect6_time64 __NR_pselect6
#endif
  int r;
  if (supports_time64 ())
    {
      /* NB: This is required by ARGIFY used in x32 internal_syscallN.  */
      __syscall_ulong_t data[2] =
	{
	  (uintptr_t) sigmask, __NSIG_BYTES
	};
      r = SYSCALL_CANCEL (pselect6_time64, nfds, readfds, writefds, exceptfds,
			  timeout, data);
      if (r == 0 || errno != ENOSYS)
	return r;

      mark_time64_unsupported ();
    }

#ifndef __ASSUME_TIME64_SYSCALLS
  r = __pselect32 (nfds, readfds, writefds, exceptfds, timeout, sigmask);
#endif
  return r;
}

#if __TIMESIZE != 64
libc_hidden_def (__pselect64)

int
__pselect (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	   const struct timespec *timeout, const sigset_t *sigmask)
{
  struct __timespec64 ts64, *pts64 = NULL;
  if (timeout != NULL)
    {
      ts64 = valid_timespec_to_timespec64 (*timeout);
      pts64 = &ts64;
    }
  return __pselect64 (nfds, readfds, writefds, exceptfds, pts64, sigmask);
}
#endif

#ifndef __pselect
weak_alias (__pselect, pselect)
#endif
