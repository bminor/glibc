/* Pseudo Random Number Generator based on ChaCha20.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <arc4random.h>
#include <errno.h>
#include <not-cancel.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/random.h>
#include <tls-internal.h>

/* arc4random keeps two counters: 'have' is the current valid bytes not yet
   consumed in 'buf' while 'count' is the maximum number of bytes until a
   reseed.

   Both the initial seed and reseed try to obtain entropy from the kernel
   and abort the process if none could be obtained.

   The state 'buf' improves the usage of the cipher calls, allowing to call
   optimized implementations (if the architecture provides it) and minimize
   function call overhead.  */

#include <chacha20.c>

/* Called from the fork function to reset the state.  */
void
__arc4random_fork_subprocess (void)
{
  struct arc4random_state_t *state = __glibc_tls_internal ()->rand_state;
  if (state != NULL)
    {
      explicit_bzero (state, sizeof (*state));
      /* Force key init.  */
      state->count = -1;
    }
}

/* Return the current thread random state or try to create one if there is
   none available.  In the case malloc can not allocate a state, arc4random
   will try to get entropy with arc4random_getentropy.  */
static struct arc4random_state_t *
arc4random_get_state (void)
{
  struct arc4random_state_t *state = __glibc_tls_internal ()->rand_state;
  if (state == NULL)
    {
      state = malloc (sizeof (struct arc4random_state_t));
      if (state != NULL)
	{
	  /* Force key initialization on first call.  */
	  state->count = -1;
	  __glibc_tls_internal ()->rand_state = state;
	}
    }
  return state;
}

static void
arc4random_getrandom_failure (void)
{
  __libc_fatal ("Fatal glibc error: cannot get entropy for arc4random\n");
}

static void
arc4random_rekey (struct arc4random_state_t *state, uint8_t *rnd, size_t rndlen)
{
  chacha20_crypt (state->ctx, state->buf, state->buf, sizeof state->buf);

  /* Mix optional user provided data.  */
  if (rnd != NULL)
    {
      size_t m = MIN (rndlen, CHACHA20_KEY_SIZE + CHACHA20_IV_SIZE);
      for (size_t i = 0; i < m; i++)
	state->buf[i] ^= rnd[i];
    }

  /* Immediately reinit for backtracking resistance.  */
  chacha20_init (state->ctx, state->buf, state->buf + CHACHA20_KEY_SIZE);
  explicit_bzero (state->buf, CHACHA20_KEY_SIZE + CHACHA20_IV_SIZE);
  state->have = sizeof (state->buf) - (CHACHA20_KEY_SIZE + CHACHA20_IV_SIZE);
}

static void
arc4random_getentropy (void *rnd, size_t len)
{
  if (__getrandom_nocancel (rnd, len, GRND_NONBLOCK) == len)
    return;

  int fd = TEMP_FAILURE_RETRY (__open64_nocancel ("/dev/urandom",
						  O_RDONLY | O_CLOEXEC));
  if (fd != -1)
    {
      uint8_t *p = rnd;
      uint8_t *end = p + len;
      do
	{
	  ssize_t ret = TEMP_FAILURE_RETRY (__read_nocancel (fd, p, end - p));
	  if (ret <= 0)
	    arc4random_getrandom_failure ();
	  p += ret;
	}
      while (p < end);

      if (__close_nocancel (fd) == 0)
	return;
    }
  arc4random_getrandom_failure ();
}

/* Check if the thread context STATE should be reseed with kernel entropy
   depending of requested LEN bytes.  If there is less than requested,
   the state is either initialized or reseeded, otherwise the internal
   counter subtract the requested length.  */
static void
arc4random_check_stir (struct arc4random_state_t *state, size_t len)
{
  if (state->count <= len || state->count == -1)
    {
      uint8_t rnd[CHACHA20_KEY_SIZE + CHACHA20_IV_SIZE];
      arc4random_getentropy (rnd, sizeof rnd);

      if (state->count == -1)
	chacha20_init (state->ctx, rnd, rnd + CHACHA20_KEY_SIZE);
      else
	arc4random_rekey (state, rnd, sizeof rnd);

      explicit_bzero (rnd, sizeof rnd);

      /* Invalidate the buf.  */
      state->have = 0;
      memset (state->buf, 0, sizeof state->buf);
      state->count = CHACHA20_RESEED_SIZE;
    }
  else
    state->count -= len;
}

void
__arc4random_buf (void *buffer, size_t len)
{
  struct arc4random_state_t *state = arc4random_get_state ();
  if (__glibc_unlikely (state == NULL))
    {
      arc4random_getentropy (buffer, len);
      return;
    }

  arc4random_check_stir (state, len);
  while (len > 0)
    {
      if (state->have > 0)
	{
	  size_t m = MIN (len, state->have);
	  uint8_t *ks = state->buf + sizeof (state->buf) - state->have;
	  memcpy (buffer, ks, m);
	  explicit_bzero (ks, m);
	  buffer += m;
	  len -= m;
	  state->have -= m;
	}
      if (state->have == 0)
	arc4random_rekey (state, NULL, 0);
    }
}
libc_hidden_def (__arc4random_buf)
weak_alias (__arc4random_buf, arc4random_buf)

uint32_t
__arc4random (void)
{
  uint32_t r;

  struct arc4random_state_t *state = arc4random_get_state ();
  if (__glibc_unlikely (state == NULL))
    {
      arc4random_getentropy (&r, sizeof (uint32_t));
      return r;
    }

  arc4random_check_stir (state, sizeof (uint32_t));
  if (state->have < sizeof (uint32_t))
    arc4random_rekey (state, NULL, 0);
  uint8_t *ks = state->buf + sizeof (state->buf) - state->have;
  memcpy (&r, ks, sizeof (uint32_t));
  memset (ks, 0, sizeof (uint32_t));
  state->have -= sizeof (uint32_t);

  return r;
}
libc_hidden_def (__arc4random)
weak_alias (__arc4random, arc4random)
