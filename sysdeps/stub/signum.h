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

#ifdef	_SIGNAL_H

/* Fake signal functions.  */
extern void EXFUN(_sig_ign, (int sig));
extern void EXFUN(_sig_dfl, (int sig));

#define	SIG_ERR	/* Error return.  */	\
	((void EXFUN((*), (int sig))) 0)
#define	SIG_DFL	/* Default action.  */	\
	_sig_dfl
#define	SIG_IGN	/* Ignore signal.  */	\
	_sig_ign


/* ANSI signals.  */
#define	SIGABRT	1	/* Abnormal termination.  */
#define	SIGFPE	2	/* Erroneous arithmetic operation.  */
#define	SIGILL	3	/* Illegal instruction.  */
#define	SIGINT	3	/* Interactive attention signal.  */
#define	SIGSEGV	4	/* Invalid access to storage.  */
#define	SIGTERM	5	/* Termination request.  */

/* POSIX signals.  */
#define	SIGHUP	6	/* Hangup.  */
#define	SIGQUIT	7	/* Quit.  */
#define	SIGPIPE	8	/* Broken pipe.  */
#define	SIGKILL	9	/* Kill (cannot be blocked, caught, or ignored).  */
#define	SIGALRM	10	/* Alarm clock.  */
#define	SIGSTOP	11	/* Stop (cannot be blocked, caught, or ignored).  */
#define	SIGTSTP	12	/* Keyboard stop.  */
#define	SIGCONT	13	/* Continue.  */
#define	SIGCHLD	14	/* Child terminated or stopped.  */
#define	SIGTTIN	15	/* Background read from control terminal.  */
#define	SIGTTOU	16	/* Background write to control terminal.  */

#endif	/* <signal.h> included.  */

#define	_NSIG	17
