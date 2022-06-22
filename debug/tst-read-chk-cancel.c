/* Test that __read_chk is a cancellation point (BZ #29274)
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

#include <stdint.h>
#include <support/xunistd.h>
#include <support/xthread.h>

static int pipe_fds[2];
static pthread_barrier_t barrier;

static void *
read_thread (void *n)
{
  xpthread_barrier_wait (&barrier);
  char c;
  /* This call should be forwarded to __read_chk because the buffer size
     is known, but the read length is non-constant.  */
  if (read (pipe_fds[0], &c, (uintptr_t) n) != 1)
    return (void *) -1L;
  return 0;
}

static int
do_test (void)
{
  xpthread_barrier_init (&barrier, 0, 2);
  xpipe (pipe_fds);
  pthread_t thr = xpthread_create (0, read_thread, (void *) 1L);
  xpthread_barrier_wait (&barrier);
  xpthread_cancel (thr);
  xpthread_join (thr);
  return 0;
}

#include <support/test-driver.c>
