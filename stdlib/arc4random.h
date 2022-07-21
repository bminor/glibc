/* Arc4random definition used on TLS.
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

#ifndef _CHACHA20_H
#define _CHACHA20_H

#include <stddef.h>
#include <stdint.h>

/* Internal ChaCha20 state.  */
#define CHACHA20_STATE_LEN	16
#define CHACHA20_BLOCK_SIZE	64

/* Maximum number bytes until reseed (16 MB).  */
#define CHACHA20_RESEED_SIZE	(16 * 1024 * 1024)

/* Internal arc4random buffer, used on each feedback step so offer some
   backtracking protection and to allow better used of vectorized
   chacha20 implementations.  */
#define CHACHA20_BUFSIZE        (8 * CHACHA20_BLOCK_SIZE)

_Static_assert (CHACHA20_BUFSIZE >= CHACHA20_BLOCK_SIZE + CHACHA20_BLOCK_SIZE,
		"CHACHA20_BUFSIZE < CHACHA20_BLOCK_SIZE + CHACHA20_BLOCK_SIZE");

struct arc4random_state_t
{
  uint32_t ctx[CHACHA20_STATE_LEN];
  size_t have;
  size_t count;
  uint8_t buf[CHACHA20_BUFSIZE];
};

#endif
