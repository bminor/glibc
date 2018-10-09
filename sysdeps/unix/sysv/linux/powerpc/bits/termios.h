/* Copyright (C) 1997-2019 Free Software Foundation, Inc.
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

typedef unsigned char	cc_t;
typedef unsigned int	speed_t;
typedef unsigned int	tcflag_t;

#include <bits/termios-struct.h>
#include <bits/termios-c_cc.h>
#include <bits/termios-c_iflag.h>
#include <bits/termios-c_oflag.h>

/* c_cflag bit meaning */
#ifdef __USE_MISC
# define CBAUD	0000377
#endif
#define  B0	0000000		/* hang up */
#define  B50	0000001
#define  B75	0000002
#define  B110	0000003
#define  B134	0000004
#define  B150	0000005
#define  B200	0000006
#define  B300	0000007
#define  B600	0000010
#define  B1200	0000011
#define  B1800	0000012
#define  B2400	0000013
#define  B4800	0000014
#define  B9600	0000015
#define  B19200	0000016
#define  B38400	0000017
#ifdef __USE_MISC
# define EXTA B19200
# define EXTB B38400
# define CBAUDEX 0000020
#endif
#define  B57600   00020
#define  B115200  00021
#define  B230400  00022
#define  B460800  00023
#define  B500000  00024
#define  B576000  00025
#define  B921600  00026
#define  B1000000 00027
#define  B1152000 00030
#define  B1500000 00031
#define  B2000000 00032
#define  B2500000 00033
#define  B3000000 00034
#define  B3500000 00035
#define  B4000000 00036
#define __MAX_BAUD B4000000

#define CSIZE	00001400
#define   CS5	00000000
#define   CS6	00000400
#define   CS7	00001000
#define   CS8	00001400

#define CSTOPB	00002000
#define CREAD	00004000
#define PARENB	00010000
#define PARODD	00020000
#define HUPCL	00040000

#define CLOCAL	00100000
#ifdef __USE_MISC
# define CMSPAR   010000000000		/* mark or space (stick) parity */
# define CRTSCTS  020000000000		/* flow control */
#endif

/* c_lflag bits */
#define ISIG	0x00000080
#define ICANON	0x00000100
#if defined __USE_MISC || (defined __USE_XOPEN && !defined __USE_XOPEN2K)
# define XCASE	0x00004000
#endif
#define ECHO	0x00000008
#define ECHOE	0x00000002
#define ECHOK	0x00000004
#define ECHONL	0x00000010
#define NOFLSH	0x80000000
#define TOSTOP	0x00400000
#ifdef __USE_MISC
# define ECHOCTL	0x00000040
# define ECHOPRT	0x00000020
# define ECHOKE	0x00000001
# define FLUSHO	0x00800000
# define PENDIN	0x20000000
#endif
#define IEXTEN	0x00000400
#ifdef __USE_MISC
# define EXTPROC 0x10000000
#endif

/* Values for the ACTION argument to `tcflow'.  */
#define	TCOOFF		0
#define	TCOON		1
#define	TCIOFF		2
#define	TCION		3

/* Values for the QUEUE_SELECTOR argument to `tcflush'.  */
#define	TCIFLUSH	0
#define	TCOFLUSH	1
#define	TCIOFLUSH	2

/* Values for the OPTIONAL_ACTIONS argument to `tcsetattr'.  */
#define	TCSANOW		0
#define	TCSADRAIN	1
#define	TCSAFLUSH	2

#ifdef __USE_MISC

struct sgttyb {
	char	sg_ispeed;
	char	sg_ospeed;
	char	sg_erase;
	char	sg_kill;
	short	sg_flags;
};

struct tchars {
	char	t_intrc;
	char	t_quitc;
	char	t_startc;
	char	t_stopc;
	char	t_eofc;
	char	t_brkc;
};

struct ltchars {
	char	t_suspc;
	char	t_dsuspc;
	char	t_rprntc;
	char	t_flushc;
	char	t_werasc;
	char	t_lnextc;
};

/* Used for packet mode */
#define TIOCPKT_DATA		 0
#define TIOCPKT_FLUSHREAD	 1
#define TIOCPKT_FLUSHWRITE	 2
#define TIOCPKT_STOP		 4
#define TIOCPKT_START		 8
#define TIOCPKT_NOSTOP		16
#define TIOCPKT_DOSTOP		32

/* c_cc characters */
#define _VINTR	0
#define _VQUIT	1
#define _VERASE	2
#define _VKILL	3
#define _VEOF	4
#define _VMIN	5
#define _VEOL	6
#define _VTIME	7
#define _VEOL2	8
#define _VSWTC	9

#ifdef __USE_MISC
/* ioctl (fd, TIOCSERGETLSR, &result) where result may be as below */
# define TIOCSER_TEMT    0x01	/* Transmitter physically empty */
#endif

#endif /* __USE_MISC  */
