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

#ifndef	_SYS_IOCTL_H

#define	_SYS_IOCTL_H	1
#include <features.h>

/* Get the list of `ioctl' requests and related constants.  */
#include <ioctls.h>


#if	defined(TIOCGETC) || defined(TIOCSETC)
/* Type of ARG for TIOCGETC and TIOCSETC requests.  */
struct tchars
  {
    char t_intrc;	/* Interrupt character.  */
    char t_quitc;	/* Quit character.  */
    char t_startc;	/* Start-output character.  */
    char t_stopc;	/* Stop-output character.  */
    char t_eofc;	/* End-of-file character.  */
    char t_brkc;	/* Input delimiter character.  */
  };
#endif

#if	defined(TIOCGLTC) || defined(TIOCSLTC)
/* Type of ARG for TIOCGLTC and TIOCSLTC requests.  */
struct ltchars
  {
    char t_suspc;	/* Suspend character.  */
    char t_dsuspc;	/* Delayed suspend character.  */
    char t_rprntc;	/* Reprint-line character.  */
    char t_flushc;	/* Flush-output character.  */
    char t_werasc;	/* Word-erase character.  */
    char t_lnextc;	/* Literal-next character.  */
  };
#endif

#if	defined(TIOCGETP) || defined(TIOCSETP)
/* Type of ARG for TIOCGETP and TIOCSETP requests.  */
struct sgttyb
  {
    char sg_ispeed;	/* Input speed.  */
    char sg_ospeed;	/* Output speed.  */
    char sg_erase;	/* Erase character.  */
    char sg_kill;	/* Kill character.  */
    short int sg_flags;	/* Mode flags.  */
  };
#endif

#if	defined(TIOCGWINSZ) || defined(TIOCSWINSZ)
/* Type of ARG for TIOCGWINSZ and TIOCSWINSZ requests.  */
struct winsize
  {
    unsigned short int ws_row;	/* Rows, in characters.  */
    unsigned short int ws_col;	/* Columns, in characters.  */

    unsigned short int ws_xpixel;	/* Horizontal pixels.  */
    unsigned short int ws_ypixel;	/* Vertical pixels.  */
  };
#endif


/* Perform the I/O control operation specified by REQUEST on FD.
   The actual type and use of ARG and the return value depend on REQUEST.  */
extern int EXFUN(__ioctl, (int __fd, int __request, PTR __arg));
extern int EXFUN(ioctl, (int __fd, int __request, PTR __arg));

#ifdef	__OPTIMIZE__
#define	ioctl(fd, request, arg)	__ioctl((fd), (request), (arg))
#endif


#endif	/* sys/ioctl.h */
