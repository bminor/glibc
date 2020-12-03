/* Send a signal to a specific pthread.  Stub version.
   Copyright (C) 2014-2021 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <pthreadP.h>
#include <shlib-compat.h>

int
__pthread_kill (pthread_t threadid, int signo)
{
  /* Disallow sending the signal we use for cancellation, timers,
     for the setxid implementation.  */
  if (__is_internal_signal (signo))
    return EINVAL;

  /* Force load of pd->tid into local variable or register.  Otherwise
     if a thread exits between ESRCH test and tgkill, we might return
     EINVAL, because pd->tid would be cleared by the kernel.  */
  struct pthread *pd = (struct pthread *) threadid;
  pid_t tid = atomic_forced_read (pd->tid);
  if (__glibc_unlikely (tid <= 0))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* We have a special syscall to do the work.  */
  pid_t pid = __getpid ();

  int val = INTERNAL_SYSCALL_CALL (tgkill, pid, tid, signo);
  return (INTERNAL_SYSCALL_ERROR_P (val)
	  ? INTERNAL_SYSCALL_ERRNO (val) : 0);
}
versioned_symbol (libc, __pthread_kill, pthread_kill, GLIBC_2_34);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_34)
compat_symbol (libc, __pthread_kill, pthread_kill, GLIBC_2_0);
#endif
