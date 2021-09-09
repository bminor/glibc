/* Copyright (C) 2009-2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include <libc-lock.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthreadP.h>
#include <tls.h>
#include <sysdep.h>
#include <shlib-compat.h>

int
__pthread_sigqueue (pthread_t threadid, int signo, const union sigval value)
{
  /* Disallow sending the signal we use for cancellation, timers,
     for the setxid implementation.  */
  if (signo == SIGCANCEL || signo == SIGTIMER || signo == SIGSETXID)
    return EINVAL;

  struct pthread *pd = (struct pthread *) threadid;

  /* Block all signals, as required by pd->exit_lock.  */
  internal_sigset_t old_mask;
  internal_signal_block_all (&old_mask);
  __libc_lock_lock (pd->exit_lock);

  int res;
  if (pd->tid != 0)
    {
      pid_t pid = getpid ();

      siginfo_t info = { 0 };
      info.si_signo = signo;
      info.si_code = SI_QUEUE;
      info.si_pid = pid;
      info.si_uid = __getuid ();
      info.si_value = value;

      res = -INTERNAL_SYSCALL_CALL (rt_tgsigqueueinfo, pid, pd->tid, signo,
				    &info);
    }
  else
    res = EINVAL;

  __libc_lock_unlock (pd->exit_lock);
  internal_signal_restore_set (&old_mask);

  return res;
}
versioned_symbol (libc, __pthread_sigqueue, pthread_sigqueue, GLIBC_2_34);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_11, GLIBC_2_34)
compat_symbol (libpthread, __pthread_sigqueue, pthread_sigqueue, GLIBC_2_11);
#endif
