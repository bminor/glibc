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

/*
 *	POSIX Standard: 7.1-2 General Terminal Interface	<termios.h>
 */

#ifndef	_TERMIOS_H

#define	_TERMIOS_H	1
#include <features.h>

/* Type of terminal control flag masks.  */
typedef unsigned short int tcflag_t;

/* Type of control characters.  */
typedef unsigned char cc_t;

/* Type of baud rate specifiers.  */
typedef unsigned char speed_t;

/* Terminal control structure.  */
struct termios
  {
    /* Input modes.  */
    tcflag_t c_iflag;
#define	BRKINT	(1 << 0)	/* Signal interrupt on break.  */
#define	ICRNL	(1 << 1)	/* Map CR to NL on input.  */
#define	IGNBRK	(1 << 2)	/* Ignore break condition.  */
#define	IGNCR	(1 << 3)	/* Ignore CR.  */
#define	IGNPAR	(1 << 4)	/* Ignore characters with parity errors.  */
#define	INLCR	(1 << 5)	/* Map NL to CR on input.  */
#define	INPCK	(1 << 6)	/* Enable input parity check.  */
#define	ISTRIP	(1 << 7)	/* Strip character.  */
#define	IXOFF	(1 << 8)	/* Enable start/stop input control.  */
#define	IXON	(1 << 9)	/* Enable start/stop output control.  */
#define	PARMRK	(1 << 10)	/* Mark parity errors.  */

    /* Output modes.  */
    tcflag_t c_oflag;
#define	OPOST	(1 << 0)	/* Perform output processing.  */

    /* Control modes.  */
    tcflag_t c_cflag;
#define	CLOCAL	(1 << 0)		/* Ignore modem status lines.  */
#define	CREAD	(1 << 1)		/* Enable receiver.  */
#define	CSIZE	(CS5|CS6|CS7|CS8)	/* Number of bits per byte.  */
#define	CS5	(1 << 2)		/* 5 bits per byte.  */
#define	CS6	(1 << 3)		/* 6 bits per byte.  */
#define	CS7	(1 << 4)		/* 7 bits per byte.  */
#define	CS8	(1 << 5)		/* 8 bits per byte.  */
#define	CSTOPB	(1 << 6)		/* Two stop bits instead of one.  */
#define	HUPCL	(1 << 7)		/* Hang up on last close.  */
#define	PARENB	(1 << 8)		/* Parity enable.  */
#define	PARODD	(1 << 9)		/* Odd parity instead of even.  */

    /* Local modes.  */
    tcflag_t c_lflag;
#define	_ECHO	(1 << 0)	/* Enable echo.  */
#define	_ECHOE	(1 << 1)	/* Echo ERASE as a destructive backspace.  */
#define	_ECHOK	(1 << 2)	/* Echo KILL.  */
#define	_ECHONL	(1 << 3)	/* Echo '\n'.  */
#define	_ICANON	(1 << 4)	/* Do erase and kill processing.  */
#define	_IEXTEN	(1 << 5)	/* Enable extended functions.  */
#define	_ISIG	(1 << 6)	/* Enable signals.  */
#define	_NOFLSH	(1 << 7)	/* Disable flush after INTR, QUIT, or SUSP.  */
#define	_TOSTOP	(1 << 8)	/* Send SIGTTOU for background output.  */
#define	ECHO	_ECHO
#define	ECHOE	_ECHOE
#define	ECHOK	_ECHOK
#define	ECHONL	_ECHONL
#define	ICANON	_ICANON
#define	IEXTEN	_IEXTEN
#define	ISIG	_ISIG
#define	NOFLSH	_NOFLSH
#define	TOSTOP	_TOSTOP

    /* Control characters.  */
#define	NCCS	11
    cc_t c_cc[NCCS];
#define	VEOF	0	/* End-of-file character.  */
#define	VEOL	1	/* End-of-line character.  */
#define	VERASE	2	/* Erase character.  */
#define	VINTR	3	/* Interrupt character.  */
#define	VKILL	4	/* Kill character.  */
#define	VMIN	5	/* Minimum number of bytes read at once.  */
#define	VQUIT	6	/* Quit character.  */
#define	VSUSP	7	/* Suspend character.  */
#define	VTIME	8	/* Time-out value (tenths of a second).  */
#define	VSTART	9	/* Start (X-ON) character.  */
#define	VSTOP	10	/* Stop (X-OFF) character.  */

    /* Input and output baud rates.  */
    speed_t __ispeed, __ospeed;
#define	_B0	0	/* Hang up.  */
#define	_B50	50	/* 50 baud.  */
#define	_B75	75	/* 75 baud.  */
#define	_B110	110	/* 110 baud.  */
#define	_B134	134	/* 134.5 baud.  */
#define	_B150	150	/* 150 baud.  */
#define	_B200	200	/* 200 baud.  */
#define	_B300	300	/* 300 baud.  */
#define	_B600	600	/* 600 baud.  */
#define	_B1200	1200	/* 1200 baud.  */
#define	_B1800	1800	/* 1800 baud.  */
#define	_B2400	2400	/* 2400 baud.  */
#define	_B4800	4800	/* 4800 baud.  */
#define	_B9600	9600	/* 9600 baud.  */
#define	_B19200	19200	/* 19200 baud.  */
#define	_B38400	38400	/* 38400 baud.  */
#define	B0	0
#define	B50	50
#define	B75	75
#define	B110	110
#define	B134	134
#define	B150	150
#define	B200	200
#define	B300	300
#define	B600	600
#define	B1200	1200
#define	B1800	1800
#define	B2400	2400
#define	B4800	4800
#define	B9600	9600
#define	B19200	19200
#define	B38400	38400
  };


/* Return the output baud rate stored in *TERMIOS_P.  */
extern speed_t EXFUN(cfgetospeed, (CONST struct termios *__termios_p));

/* Return the input baud rate stored in *TERMIOS_P.  */
extern speed_t EXFUN(cfgetispeed, (CONST struct termios *__termios_p));

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
extern int EXFUN(cfsetospeed, (struct termios *__termios_p, speed_t __speed));

/* Set the input baud rate stored in *TERMIOS_P to SPEED.  */
extern int EXFUN(cfsetispeed, (struct termios *__termios_p, speed_t __speed));


/* Put the state of FD into *TERMIOS_P.  */
extern int EXFUN(__tcgetattr, (int __fd, struct termios *__termios_p));
extern int EXFUN(tcgetattr, (int __fd, struct termios *__termios_p));

#ifdef	__OPTIMIZE__
#define	tcgetattr(fd, termios_p)	__tcgetattr((fd), (termios_p))
#endif	/* Optimizing.  */

/* Values for the OPTIONAL_ACTIONS argument to `tcsetattr'.  */
#define	TCSANOW		0	/* Change immediately.  */
#define	TCSADRAIN	1	/* Change when pending output is written.  */
#define	TCSAFLUSH	2	/* Flush pending input before changing.  */

/* Set the state of FD to *TERMIOS_P.  */
extern int EXFUN(tcsetattr, (int __fd, int __optional_actions,
			     CONST struct termios *__termios_p));


/* Send zero bits on FD.  */
extern int EXFUN(tcsendbreak, (int __fd, int __duration));

/* Wait for pending output to be written on FD.  */
extern int EXFUN(tcdrain, (int __fd));

/* Values for the QUEUE_SELECTOR argument to `tcflush'.  */
#define	TCIFLUSH	0	/* Discard data received but not yet read.  */
#define	TCOFLUSH	1	/* Discard data written but not yet sent.  */
#define	TCIOFLUSH	2	/* Discard all pending data.  */

/* Flush pending data on FD.  */
extern int EXFUN(tcflush, (int __fd, int __queue_selector));

/* Values for the ACTION argument to `tcflow'.  */
#define	TCOOFF	0	/* Suspend output.  */
#define	TCOON	1	/* Restart suspended output.  */
#define	TCIOFF	2	/* Send a STOP character.  */
#define	TCION	3	/* Send a START character.  */

/* Suspend or restart transmission on FD.  */
extern int EXFUN(tcflow, (int __fd, int __action));


#endif	/* termios.h  */
