/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


/* Defined in stdio/sys_siglist.c.  */
extern CONST char *CONST _sys_siglist[];


/* Return a string describing the meaning of the signal number SIGNUM.  */
char *
DEFUN(strsignal, (signum), int signum)
{
  if (signum < 0 || signum > NSIG)
    {
      static char unknown_signal[] = "Unknown signal 000000000000000000";
      static char fmt[] = "Unknown signal%d";
      size_t len = sprintf(unknown_signal, fmt, signum);
      if (len < sizeof(fmt) - 2)
	return NULL;
      unknown_signal[len - 1] = '\0';
      return unknown_signal;
    }

  return (char *) _sys_siglist[signum];
}
