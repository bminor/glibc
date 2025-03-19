/* Test the state save/restore procedures during signal handling.

   Copyright (C) 2025 Free Software Foundation, Inc.
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


#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mach/message.h>
#include <mach/gnumach.h>
#include <mach/mach_traps.h>
#include <mach-shortcuts.h>
#include <mach_init.h>
#include <hurd/io.h>
#include <hurd/io_reply.h>

#include <support/check.h>
#include <support/xthread.h>

#include "test-xstate.h"

static volatile bool loopflag = true;

void handler (int signum, siginfo_t *info, void *context)
{
  char buf3[XSTATE_BUFFER_SIZE];
  memset (buf3, 0x77, XSTATE_BUFFER_SIZE);
  SET_XSTATE (buf3);
  printf ("signal %d setting a different CPU state\n", signum);
  loopflag = false;
}

/* Helper thread to send a signal to the main thread  */
void* signal_sender (void *arg)
{
  sigset_t ss;
  assert (! sigemptyset (&ss));
  assert (! sigaddset (&ss, SIGUSR1));
  assert (! sigprocmask (SIG_BLOCK, &ss, NULL));

  TEST_COMPARE (kill (getpid (), SIGUSR1), 0);

  return NULL;
}

static int do_test (void)
{
#if ! XSTATE_HELPERS_SUPPORTED
  FAIL_UNSUPPORTED ("Test not supported on this arch.");
#endif

  struct sigaction act = { 0 };
  act.sa_sigaction = &handler;
  TEST_COMPARE (sigaction (SIGUSR1, &act, NULL), 0);

  pthread_t thsender = xpthread_create (NULL, signal_sender, NULL);

  char buf1[XSTATE_BUFFER_SIZE], buf2[XSTATE_BUFFER_SIZE];
  memset (buf1, 0x33, XSTATE_BUFFER_SIZE);

  SET_XSTATE (buf1);

  while (loopflag)
    ;

  GET_XSTATE (buf2);
  TEST_COMPARE_BLOB (buf1, sizeof (buf1), buf2, sizeof (buf2));

  xpthread_join (thsender);
  return EXIT_SUCCESS;
}

#include <support/test-driver.c>
