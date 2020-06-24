/* Wait on a semaphore with a timeout.  Generic version.
   Copyright (C) 2005-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include <pt-internal.h>

struct cancel_ctx
{
  struct __pthread *wakeup;
  sem_t *sem;
  int cancel_wake;
};

static void
cancel_hook (void *arg)
{
  struct cancel_ctx *ctx = arg;
  struct __pthread *wakeup = ctx->wakeup;
  sem_t *sem = ctx->sem;
  int unblock;

  __pthread_spin_wait (&sem->__lock);
  /* The thread only needs to be awaken if it's blocking or about to block.
     If it was already unblocked, it's not queued any more.  */
  unblock = wakeup->prevp != NULL;
  if (unblock)
    {
      __pthread_dequeue (wakeup);
      ctx->cancel_wake = 1;
    }
  __pthread_spin_unlock (&sem->__lock);

  if (unblock)
    __pthread_wakeup (wakeup);
}

int
__sem_timedwait_internal (sem_t *restrict sem,
			  clockid_t clock_id,
			  const struct timespec *restrict timeout)
{
  error_t err;
  int cancelled, oldtype, drain;
  int ret = 0;

  struct __pthread *self = _pthread_self ();
  struct cancel_ctx ctx;
  ctx.wakeup = self;
  ctx.sem = sem;
  ctx.cancel_wake = 0;

  /* Test for a pending cancellation request, switch to deferred mode for
     safer resource handling, and prepare the hook to call in case we're
     cancelled while blocking.  Once CANCEL_LOCK is released, the cancellation
     hook can be called by another thread at any time.  Whatever happens,
     this function must exit with MUTEX locked.

     This function contains inline implementations of pthread_testcancel and
     pthread_setcanceltype to reduce locking overhead.  */
  __pthread_mutex_lock (&self->cancel_lock);
  cancelled = (self->cancel_state == PTHREAD_CANCEL_ENABLE)
      && self->cancel_pending;

  if (cancelled)
    {
      __pthread_mutex_unlock (&self->cancel_lock);
      __pthread_exit (PTHREAD_CANCELED);
    }

  self->cancel_hook = cancel_hook;
  self->cancel_hook_arg = &ctx;
  oldtype = self->cancel_type;

  if (oldtype != PTHREAD_CANCEL_DEFERRED)
    self->cancel_type = PTHREAD_CANCEL_DEFERRED;

  /* Add ourselves to the list of waiters.  This is done while setting
     the cancellation hook to simplify the cancellation procedure, i.e.
     if the thread is queued, it can be cancelled, otherwise it is
     already unblocked, progressing on the return path.  */
  __pthread_spin_wait (&sem->__lock);
  if (sem->__value > 0)
    /* Successful down.  */
    {
      sem->__value--;
      __pthread_spin_unlock (&sem->__lock);
      goto out_locked;
    }

  if (timeout != NULL && ! valid_nanoseconds (timeout->tv_nsec))
    {
      errno = EINVAL;
      ret = -1;
      __pthread_spin_unlock (&sem->__lock);
      goto out_locked;
    }

  /* Add ourselves to the queue.  */
  __pthread_enqueue (&sem->__queue, self);
  __pthread_spin_unlock (&sem->__lock);

  __pthread_mutex_unlock (&self->cancel_lock);

  /* Block the thread.  */
  if (timeout != NULL)
    err = __pthread_timedblock_intr (self, timeout, clock_id);
  else
    err = __pthread_block_intr (self);

  __pthread_spin_wait (&sem->__lock);
  if (self->prevp == NULL)
    /* Another thread removed us from the queue, which means a wakeup message
       has been sent.  It was either consumed while we were blocking, or
       queued after we timed out and before we acquired the semaphore lock, in
       which case the message queue must be drained.  */
    drain = err ? 1 : 0;
  else
    {
      /* We're still in the queue.  Noone attempted to wake us up, i.e. we
         timed out.  */
      __pthread_dequeue (self);
      drain = 0;
    }
  __pthread_spin_unlock (&sem->__lock);

  if (drain)
    __pthread_block (self);

  if (err)
    {
      assert (err == ETIMEDOUT || err == EINTR);
      errno = err;
      ret = -1;
    }

  /* We're almost done.  Remove the unblock hook, restore the previous
     cancellation type, and check for a pending cancellation request.  */
  __pthread_mutex_lock (&self->cancel_lock);
out_locked:
  self->cancel_hook = NULL;
  self->cancel_hook_arg = NULL;
  self->cancel_type = oldtype;
  cancelled = (self->cancel_state == PTHREAD_CANCEL_ENABLE)
      && self->cancel_pending;
  __pthread_mutex_unlock (&self->cancel_lock);

  if (cancelled)
    {
      if (ret == 0 && ctx.cancel_wake == 0)
	/* We were cancelled while waking up with a token, put it back.  */
	__sem_post (sem);

      __pthread_exit (PTHREAD_CANCELED);
    }

  return ret;
}

int
__sem_clockwait (sem_t *sem, clockid_t clockid,
		 const struct timespec *restrict timeout)
{
  return __sem_timedwait_internal (sem, clockid, timeout);
}
weak_alias (__sem_clockwait, sem_clockwait);

int
__sem_timedwait (sem_t *restrict sem, const struct timespec *restrict timeout)
{
  return __sem_timedwait_internal (sem, CLOCK_REALTIME, timeout);
}

weak_alias (__sem_timedwait, sem_timedwait);
