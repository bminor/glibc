/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

char *
getpass (prompt)
     const char *prompt;
{
  FILE *in;
  struct termios t;
  int echo_off;
  static char *buf = NULL;
  static size_t bufsize = 0;

  in = fopen ("/dev/tty", "w+");
  if (in == NULL)
    in = stdin;

  if (tcgetattr (fileno (in), &t) == 0)
    {
      if (t.c_lflag & ECHO)
	{
	  t.c_lflag &= ~ECHO;
	  echo_off = tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t) == 0;
	  t.c_lflag |= ECHO;
	}
      else
	echo_off = 0;
    }
  else
    echo_off = 0;

  fputs (prompt, in == stdin ? stderr : in);
  fflush (in == stdin ? stderr : in);

  if (__getline (&buf, &bufsize, in) < 0 && buf != NULL)
    buf[0] = '\0';

  if (echo_off)
    (void) tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t);

  if (in != stdin)
    fclose (in);

  return buf;
}
