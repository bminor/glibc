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
#include <errno.h>
#include <stddef.h>
#include <termios.h>

static int EXFUN(bad_speed, (speed_t speed));

/* Set the state of FD to *TERMIOS_P.  */
int
DEFUN(tcsetattr, (fd, optional_actions, termios_p),
      int fd AND int optional_actions AND CONST struct termios *termios_p)
{
  if (fd < 0)
    {
      errno = EBADF;
      return -1;
    }
  if (termios_p == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  switch (optional_actions)
    {
    case TCSANOW:
    case TCSADRAIN:
    case TCSAFLUSH:
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  if (bad_speed(termios_p->__ospeed) ||
      bad_speed(termios_p->__ispeed == 0 ?
		termios_p->__ospeed : termios_p->__ispeed))
    {
      errno = EINVAL;
      return -1;
    }

  errno = ENOSYS;
  return -1;
}


/* Stricknine checking.  */
static int
DEFUN(bad_speed, (speed), speed_t speed)
{
  switch (speed)
    {
    B0:
    B50:
    B75:
    B110:
    B134:
    B150:
    B200:
    B300:
    B600:
    B1200:
    B1800:
    B2400:
    B4800:
    B9600:
    B19200:
    B38400:
      return 0;
    default:
      return 1;
    }
}


#ifdef	 __GNU_STAB__

#include <gnu-stabs.h>

stub_warning(tcsetattr);

#endif	/* GNU stabs.  */
