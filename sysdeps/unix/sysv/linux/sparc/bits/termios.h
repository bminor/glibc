/* termios type and macro definitions.  Linux/SPARC version.
   Copyright (C) 1993-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _TERMIOS_H
# error "Never include <bits/termios.h> directly; use <termios.h> instead."
#endif

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#include <bits/termios-struct.h>
#include <bits/termios-c_cc.h>
#include <bits/termios-c_iflag.h>
#include <bits/termios-c_oflag.h>

/* c_cflag bit meaning */
#define  B0	0x00000000	/* hang up */
#define  B50	0x00000001
#define  B75	0x00000002
#define  B110	0x00000003
#define  B134	0x00000004
#define  B150	0x00000005
#define  B200	0x00000006
#define  B300	0x00000007
#define  B600	0x00000008
#define  B1200	0x00000009
#define  B1800	0x0000000a
#define  B2400	0x0000000b
#define  B4800	0x0000000c
#define  B9600	0x0000000d
#define  B19200	0x0000000e
#define  B38400	0x0000000f
#ifdef __USE_MISC
# define EXTA    B19200
# define EXTB    B38400
#endif
#include <bits/termios-baud.h>

#define  CSIZE  0x00000030
#define   CS5	0x00000000
#define   CS6	0x00000010
#define   CS7	0x00000020
#define   CS8	0x00000030
#define CSTOPB	0x00000040
#define CREAD	0x00000080
#define PARENB	0x00000100
#define PARODD	0x00000200
#define HUPCL	0x00000400
#define CLOCAL	0x00000800

/* c_lflag bits */
#define ISIG	0x00000001
#define ICANON	0x00000002
#if defined __USE_MISC || (defined __USE_XOPEN && !defined __USE_XOPEN2K)
# define XCASE	0x00000004
#endif
#define ECHO	0x00000008
#define ECHOE	0x00000010
#define ECHOK	0x00000020
#define ECHONL	0x00000040
#define NOFLSH	0x00000080
#define TOSTOP	0x00000100
#ifdef __USE_MISC
# define ECHOCTL	0x00000200
# define ECHOPRT	0x00000400
# define ECHOKE		0x00000800
# define DEFECHO	0x00001000	/* SUNOS thing, what is it? */
# define FLUSHO		0x00002000
# define PENDIN		0x00004000
#endif
#define IEXTEN	0x00008000
#ifdef __USE_MISC
# define EXTPROC 0x00010000
#endif

#ifdef __USE_MISC
/* ioctl (fd, TIOCSERGETLSR, &result) where result may be as below */
# define TIOCSER_TEMT    0x01	/* Transmitter physically empty */
#endif

/* tcflow() and TCXONC use these */
#define	TCOOFF		0
#define	TCOON		1
#define	TCIOFF		2
#define	TCION		3

/* tcflush() and TCFLSH use these */
#define	TCIFLUSH	0
#define	TCOFLUSH	1
#define	TCIOFLUSH	2

/* tcsetattr uses these */
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2
