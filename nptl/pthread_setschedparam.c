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
setschedparam (struct pthread *pd, int policy,
	       const struct sched_param *param)
{
  /* See CREATE THREAD NOTES in nptl/pthread_create.c.  */
  lll_lock (pd->lock, LLL_PRIVATE);

  struct sched_param p;
  const struct sched_param *orig_param = param;

  /* If the thread should have higher priority because of some
     PTHREAD_PRIO_PROTECT mutexes it holds, adjust the priority.  */
  if (pd->tpp != NULL && pd->tpp->priomax > param->sched_priority)
    {
      p = *param;
      p.sched_priority = pd->tpp->priomax;
      param = &p;
    }

  /* Try to set the scheduler information.  */
  int r = INTERNAL_SYSCALL_CALL (sched_setscheduler, pd->tid, policy, param);
  if (r == 0)
    {
      /* We succeeded changing the kernel information.  Reflect this
	 change in the thread descriptor.  */
      pd->schedpolicy = policy;
      memcpy (&pd->schedparam, orig_param, sizeof (struct sched_param));
      pd->flags |= ATTR_FLAG_SCHED_SET | ATTR_FLAG_POLICY_SET;
    }

  lll_unlock (pd->lock, LLL_PRIVATE);

  return -r;
}

int
__pthread_setschedparam (pthread_t threadid, int policy,
			 const struct sched_param *param)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Block all signals, as required by pd->exit_lock.  */
  internal_sigset_t old_mask;
  internal_signal_block_all (&old_mask);
  __libc_lock_lock (pd->exit_lock);

  int result = pd->tid != 0 ? setschedparam (pd, policy, param) : EINVAL;

  __libc_lock_unlock (pd->exit_lock);
  internal_signal_restore_set (&old_mask);

  return result;
}
strong_alias (__pthread_setschedparam, pthread_setschedparam)
