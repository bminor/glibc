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
#include <pthreadP.h>
#include <shlib-compat.h>

static int
setschedprio (struct pthread *pd, int prio)
{
  struct sched_param param;
  param.sched_priority = prio;

  /* See CREATE THREAD NOTES in nptl/pthread_create.c.  */
  lll_lock (pd->lock, LLL_PRIVATE);

  /* If the thread should have higher priority because of some
     PTHREAD_PRIO_PROTECT mutexes it holds, adjust the priority.  */
  if (pd->tpp != NULL && pd->tpp->priomax > prio)
    param.sched_priority = pd->tpp->priomax;

  /* Try to set the scheduler information.  */
  int r = INTERNAL_SYSCALL_CALL (sched_setparam, pd->tid, &param);
  if (r == 0)
    {
      /* We succeeded changing the kernel information.  Reflect this
	 change in the thread descriptor.  */
      param.sched_priority = prio;
      memcpy (&pd->schedparam, &param, sizeof (struct sched_param));
      pd->flags |= ATTR_FLAG_SCHED_SET;
    }

  lll_unlock (pd->lock, LLL_PRIVATE);

  return -r;
}

int
__pthread_setschedprio (pthread_t threadid, int prio)
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Block all signals, as required by pd->exit_lock.  */
  internal_sigset_t old_mask;
  internal_signal_block_all (&old_mask);
  __libc_lock_lock (pd->exit_lock);

  int result = pd->tid != 0 ? setschedprio (pd, prio) : EINVAL;

  __libc_lock_unlock (pd->exit_lock);
  internal_signal_restore_set (&old_mask);

  return result;
}
versioned_symbol (libc, __pthread_setschedprio, pthread_setschedprio,
		  GLIBC_2_34);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_3_4, GLIBC_2_34)
compat_symbol (libpthread, __pthread_setschedprio, pthread_setschedprio,
	       GLIBC_2_3_4);
#endif
