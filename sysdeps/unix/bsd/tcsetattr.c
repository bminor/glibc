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

#undef	B0
#undef	B50
#undef	B75
#undef	B110
#undef	B134
#undef	B150
#undef	B200
#undef	B300
#undef	B600
#undef	B1200
#undef	B1800
#undef	B2400
#undef	B4800
#undef	B9600
#undef	B19200
#undef	B38400
#undef	ECHO
#undef	TOSTOP
#undef	NOFLSH
#include <sys/ioctl.h>


static int EXFUN(translate_speed, (speed_t speed, char *speed_ptr));

/* Set the state of FD to *TERMIOS_P.  */
int
DEFUN(tcsetattr, (fd, optional_actions, termios_p),
      int fd AND int optional_actions AND CONST struct termios *termios_p)
{
  struct sgttyb buf;
  struct tchars tchars;
  struct ltchars ltchars;
  int local;
#ifdef	TIOCGETX
  int extra;
#endif

  if (__ioctl(fd, TIOCGETP, &buf) < 0 ||
      __ioctl(fd, TIOCGETC, &tchars) < 0 ||
      __ioctl(fd, TIOCGLTC, &ltchars) < 0 ||
#ifdef	TIOCGETX
      __ioctl(fd, TIOCGETX, &extra) < 0 ||
#endif
      __ioctl(fd, TIOCLGET, &local) < 0)
    return -1;

  if (termios_p == NULL)
    {
      errno = EINVAL;
      return -1;
    }
  switch (optional_actions)
    {
    case TCSANOW:
      break;
    case TCSADRAIN:
      if (tcdrain(fd) < 0)
	return -1;
      break;
    case TCSAFLUSH:
      if (tcflush(fd, TCIFLUSH) < 0)
	return(-1);
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  if (!translate_speed(termios_p->__ospeed, &buf.sg_ospeed) ||
      !translate_speed(termios_p->__ispeed == 0 ?
		       termios_p->__ospeed : termios_p->__ispeed,
		       &buf.sg_ispeed))
    {
      errno = EINVAL;
      return -1;
    }
  buf.sg_ispeed = termios_p->__ispeed;
  buf.sg_ospeed = termios_p->__ospeed;

  buf.sg_flags &= ~(CBREAK|RAW);
  if (!(termios_p->c_lflag & ICANON))
    buf.sg_flags |= (termios_p->c_cflag & OPOST) ? CBREAK : RAW;
#ifdef	LPASS8
  if (termios_p->c_oflag & CS8)
    local |= LPASS8;
  else
    local &= ~LPASS8;
#endif
  if (termios_p->c_lflag & _NOFLSH)
    local |= LNOFLSH;
  else
    local &= ~LNOFLSH;
  if (termios_p->c_oflag & OPOST)
    local &= ~LLITOUT;
  else
    local |= LLITOUT;
#ifdef	TIOCGETX
  if (termios_p->c_lflag & _ISIG)
    extra &= ~NOISIG;
  else
    extra |= NOISIG;
  if (termios_p->c_cflag & CSTOPB)
    extra |= STOPB;
  else
    extra &= ~STOPB;
#endif
  if (termios_p->c_iflag & ICRNL)
    buf.sg_flags |= CRMOD;
  else
    buf.sg_flags &= ~CRMOD;
  if (termios_p->c_iflag & IXOFF)
    buf.sg_flags |= TANDEM;
  else
    buf.sg_flags &= ~TANDEM;

  buf.sg_flags &= ~(ODDP|EVENP);
  if (!(termios_p->c_cflag & PARENB))
    buf.sg_flags |= ODDP | EVENP;
  else if (termios_p->c_cflag & PARODD)
    buf.sg_flags |= ODDP;
  else
    buf.sg_flags |= EVENP;

  if (termios_p->c_lflag & _ECHO)
    buf.sg_flags |= ECHO;
  else
    buf.sg_flags &= ~ECHO;
  if (termios_p->c_lflag & ECHOE)
    local |= LCRTERA;
  else
    local &= ~LCRTERA;
  if (termios_p->c_lflag & ECHOK)
    local |= LCRTKIL;
  else
    local &= ~LCRTKIL;
  if (termios_p->c_lflag & _TOSTOP)
    local |= LTOSTOP;
  else
    local &= ~TOSTOP;

  buf.sg_erase = termios_p->c_cc[VERASE];
  buf.sg_kill = termios_p->c_cc[VKILL];
  tchars.t_eofc = termios_p->c_cc[VEOF];
  tchars.t_intrc = termios_p->c_cc[VINTR];
  tchars.t_quitc = termios_p->c_cc[VQUIT];
  ltchars.t_suspc = termios_p->c_cc[VSUSP];
  tchars.t_startc = termios_p->c_cc[VSTART];
  tchars.t_stopc = termios_p->c_cc[VSTOP];

  if (__ioctl(fd, TIOCSETP, &buf) < 0 ||
      __ioctl(fd, TIOCSETC, &tchars) < 0 ||
      __ioctl(fd, TIOCSLTC, &ltchars) < 0 ||
#ifdef	TIOCGETX
      __ioctl(fd, TIOCSETX, &extra) < 0 ||
#endif
      __ioctl(fd, TIOCLSET, &local) < 0)
    return -1;
  return 0;
}


/* Translate SPEED into a `struct sgttyb' speed value.  */
static int
DEFUN(translate_speed, (speed, speed_ptr),
      speed_t speed AND char *speed_ptr)
{
  switch (speed)
    {
    case _B0:
      *speed_ptr = B0;
      return 1;
    case _B50:
      *speed_ptr = B50;
      return 1;
    case _B75:
      *speed_ptr = B75;
      return 1;
    case _B110:
      *speed_ptr = B110;
      return 1;
    case _B134:
      *speed_ptr = B134;
      return 1;
    case _B150:
      *speed_ptr = B150;
      return 1;
    case _B200:
      *speed_ptr = B200;
      return 1;
    case _B300:
      *speed_ptr = B300;
      return 1;
    case _B600:
      *speed_ptr = B600;
      return 1;
    case _B1200:
      *speed_ptr = B1200;
      return 1;
    case _B1800:
      *speed_ptr = B1800;
      return 1;
    case _B2400:
      *speed_ptr = B2400;
      return 1;
    case _B4800:
      *speed_ptr = B4800;
      return 1;
    case _B9600:
      *speed_ptr = B9600;
      return 1;
    case _B19200:
      *speed_ptr = B19200;
      return 1;
    case _B38400:
      *speed_ptr = B38400;
      return 1;
    default:
      return 0;
    }
}
