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
#include <unistd.h>
#include <sys/time.h>

/* Sleep USECONDS microseconds, or until a previously set timer goes off.  */
unsigned int
DEFUN(usleep, (useconds), unsigned int useconds)
{
  struct itimerval otimer;
  struct timeval before, after, delta;

  if (getitimer(ITIMER_REAL, &otimer) < 0)
    return -1;

  if ((unsigned int) ((otimer.it_value.tv_sec * 1000) +
		      otimer.it_value.tv_usec) < useconds)
    {
      /* Set up the timer.  */
      struct itimerval timer;
      timer.it_value.tv_sec = useconds / 1000;
      timer.it_value.tv_usec = useconds % 1000;
      timer.it_interval.tv_sec = 0;
      timer.it_interval.tv_usec = 0;

      /* Find the time beforehand so we can tell how much time elapsed.  */
      if (gettimeofday(&before, (struct timezone *) NULL) < 0)
	return -1;

      if (setitimer(ITIMER_REAL, &timer, &otimer) < 0)
	return -1;
    }

  /* Wait for the timer to expire.  */
  (void) pause();

  /* Find out what time it is now, and see how much time elapsed.  */
  if (gettimeofday(&after, (struct timezone *) NULL) < 0)
    {
      delta.tv_sec = 0;
      delta.tv_usec = 0;
    }
  else
    {
      delta.tv_sec = after.tv_sec - before.tv_sec;
      delta.tv_usec = after.tv_usec - before.tv_usec;
    }

  /* Adjust the old timer to account for the elapsed time.  */
  otimer.it_value.tv_sec -= delta.tv_sec;
  if (otimer.it_value.tv_sec < 0)
    otimer.it_value.tv_sec = 0;
  otimer.it_value.tv_usec -= delta.tv_usec; 
  if (otimer.it_value.tv_usec < 0)
    otimer.it_value.tv_usec = 0;

  /* Restore the old timer (which might have be zero).  */
  (void) setitimer(ITIMER_REAL, &otimer, (struct itimerval *) NULL);

  return useconds - ((delta.tv_sec * 1000) + delta.tv_usec);
}
