/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>


/* SIGALRM signal handler for `sleep'.  This does nothing but return,
   but SIG_IGN isn't supposed to break `pause'.  */
static void
DEFUN(sleep_handler, (sig), int sig)
{
  return;
}

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.  */
unsigned int
DEFUN(sleep, (seconds), unsigned int seconds)
{
  unsigned int remaining, slept;
  void EXFUN((*handler), (int sig));
  time_t before, after;

  handler = signal(SIGALRM, sleep_handler);
  if (handler == SIG_ERR)
    return seconds;

  before = time((time_t *) NULL);
  remaining = alarm(seconds);
  after = time((time_t *) NULL);

  (void) pause();

  (void) signal(SIGALRM, handler);

  slept = (after - before) / 1000;
  if (remaining > slept)
    alarm(remaining - slept);

  return slept;
}
