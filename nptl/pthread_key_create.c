/* Copyright (C) 2002-2014 Free Software Foundation, Inc.
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
#include "pthreadP.h"
#include <assert.h>
#include <atomic.h>

static int
claim_key (pthread_key_t *key, void (*destr) (void *), size_t cnt)
{
  uintptr_t seq = __pthread_keys[cnt].seq;
  if (KEY_UNUSED (seq) && KEY_USABLE (seq)
      /* We found an unused slot.  Try to allocate it.  */
      && ! atomic_compare_and_exchange_bool_acq (&__pthread_keys[cnt].seq,
                                                 seq + 1, seq))
    {
      /* Remember the destructor.  */
      __pthread_keys[cnt].destr = destr;

      /* Return the key to the caller.  */
      *key = cnt;
      return 0;
    }
  return -1;
}

int
__pthread_key_create (key, destr)
     pthread_key_t *key;
     void (*destr) (void *);
{
  /* Find a slot in __pthread_keys which is unused.  */
  for (size_t cnt = PTHREAD_SIGNAL_SAFE_KEYS_MAX; cnt < PTHREAD_KEYS_MAX; ++cnt)
    {
      if (claim_key (key, destr, cnt) == 0)
        return 0;
    }

  return EAGAIN;
}

int __google_pthread_signal_safe_key_create (
    pthread_key_t *key, void (*destr) (void *))
{
  /* Our implementation makes signal safe keys easy: they just have to
     reside in the first (inline) block. */
  assert (PTHREAD_SIGNAL_SAFE_KEYS_MAX <= PTHREAD_KEY_1STLEVEL_SIZE);
  /* Find a slot in __pthread_keys which is unused.  */
  for (size_t cnt = 0; cnt < PTHREAD_SIGNAL_SAFE_KEYS_MAX; ++cnt)
    {
      if (claim_key (key, destr, cnt) == 0)
        return 0;
    }

  return EAGAIN;
}
strong_alias (__pthread_key_create, pthread_key_create)
hidden_def (__pthread_key_create)
