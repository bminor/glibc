/* Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#include <libc-lock.h>
#include <kernel-posix-cpu-timers.h>
#include <pthreadP.h>

static int
getschedparam (struct pthread *pd, int *policy, struct sched_param *param)
{
  int r = 0;

  /* See CREATE THREAD NOTES in nptl/pthread_create.c.  */
  lll_lock (pd->lock, LLL_PRIVATE);

  /* The library is responsible for maintaining the values at all
     times.  If the user uses an interface other than
     pthread_setschedparam to modify the scheduler setting it is not
     the library's problem.  In case the descriptor's values have
     not yet been retrieved do it now.  */
  if ((pd->flags & ATTR_FLAG_SCHED_SET) == 0)
    {
      r = INTERNAL_SYSCALL_CALL (sched_getparam, pd->tid, &pd->schedparam);
      if (r == 0)
	pd->flags |= ATTR_FLAG_SCHED_SET;
    }
  if ((pd->flags & ATTR_FLAG_POLICY_SET) == 0)
    {
      r = pd->schedpolicy = INTERNAL_SYSCALL_CALL (sched_getscheduler, pd->tid);
      if (r >= 0)
	pd->flags |= ATTR_FLAG_POLICY_SET;
    }

  if (r >= 0)
    {
      *policy = pd->schedpolicy;
      memcpy (param, &pd->schedparam, sizeof (struct sched_param));
    }

  lll_unlock (pd->lock, LLL_PRIVATE);

  return -r;
}

int
__pthread_getschedparam (pthread_t threadid, int *policy,
			 struct sched_param *param)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Block all signals, as required by pd->exit_lock.  */
  internal_sigset_t old_mask;
  internal_signal_block_all (&old_mask);
  __libc_lock_lock (pd->exit_lock);

  int result = pd->tid != 0 ? getschedparam (pd, policy, param) : EINVAL;

  __libc_lock_unlock (pd->exit_lock);
  internal_signal_restore_set (&old_mask);

  return result;
}
libc_hidden_def (__pthread_getschedparam)
strong_alias (__pthread_getschedparam, pthread_getschedparam)
