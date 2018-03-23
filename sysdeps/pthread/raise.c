/* ISO C raise function for libpthread.
   Copyright (C) 2002-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <internal-signals.h>

/* Raise the signal SIG.  POSIX requires raise to be async-signal-safe,
   but also requires it to be equivalent to pthread_kill (pthread_self (), sig),
   and that construct is *not* async-signal safe.  In particular, an
   async signal handler that calls fork (which is also async-signal-safe)
   could invalidate the handle returned by pthread_self, and/or cause
   pthread_kill to be called twice.  So we must block signals around
   the operation.  See bug 15368 for more detail.

   Also, raise sets errno on failure, whereas pthread_kill returns the
   error code.  (It is not possible for pthread_self to fail.)  */

int
__libc_raise (int sig)
{
  /* Disallow sending the signals we use for cancellation, timers,
     setxid, etc.  This check is also performed in pthread_kill, but
     if we do it now we can avoid blocking and then unblocking signals
     unnecessarily.  */
  if (__glibc_unlikely (__is_internal_signal (sig)))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* We can safely assume that __libc_signal_block_app and
     __libc_signal_restore_set will not fail, because
     sigprocmask can only fail under three circumstances:

     1. sigsetsize != sizeof (sigset_t) (EINVAL)
     2. a failure in copy from/to user space (EFAULT)
     3. an invalid 'how' operation (EINVAL)

     Getting any of these would indicate a bug in either the
     definition of sigset_t or the implementations of the
     wrappers.  */
  sigset_t omask;
  __libc_signal_block_app (&omask);

  int ret = __pthread_kill (__pthread_self (), sig);

  __libc_signal_restore_set (&omask);

  if (__glibc_unlikely (ret))
    {
      __set_errno (ret);
      return -1;
    }
  return 0;
}
strong_alias (__libc_raise, raise)
libc_hidden_def (raise)
weak_alias (__libc_raise, gsignal)
