/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
 *	ANSI Standard: 4.7 SIGNAL HANDLING <signal.h>
 */

#ifndef	_SIGNAL_H

#if	!defined(__need_sig_atomic_t) && !defined(__need_sigset_t)
#define	_SIGNAL_H	1
#include <features.h>
#endif

#include <gnu/types.h>
#include <gnu/signal.h>

#if	!defined(__sig_atomic_t_defined) &&	\
  (defined(_SIGNAL_H) || defined(__need_sig_atomic_t))
/* An integral type that can be modified atomically, without the
   possibility of a signal arriving in the middle of the operation.  */
typedef __sig_atomic_t sig_atomic_t;
#endif	/* `sig_atomic_t' undefined and <signal.h> or need `sig_atomic_t'.  */
#undef	__need_sig_atomic_t

#ifdef	_SIGNAL_H

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
extern __sighandler_t EXFUN(signal, (int __sig, __sighandler_t __handler));

/* Send signal SIG to process number PID.  If PID is zero,
   send SIG to all processes in the current process's process group.
   If PID is < -1, send SIG to all processes in process group - PID.  */
extern int EXFUN(__kill, (__pid_t __pid, int __sig));
#ifdef	__USE_POSIX
extern int EXFUN(kill, (int __pid, int __sig));
#ifdef	__OPTIMIZE__
#define	kill(pid, sig)	__kill((pid), (sig))
#endif	/* Optimizing.  */
#endif	/* Use POSIX.  */

#ifdef	__USE_BSD
/* Send SIG to all processes in process group PGRP.
   If PGRP is zero, send SIG to all processes in
   the current process's process group.  */
extern int EXFUN(killpg, (int __pgrp, int __sig));
#endif	/* Use BSD.  */

/* Raise signal SIG, i.e., send SIG to yourself.  */
extern int EXFUN(raise, (int __sig));

#ifdef	__USE_SVID
/* SVID names for the same things.  */
extern __sighandler_t EXFUN(ssignal, (int __sig, __sighandler_t __handler));
extern int EXFUN(gsignal, (int __sig));

#ifdef	__OPTIMIZE__
#define	gsignal(sig)		raise(sig)
#define	ssignal(sig, handler)	signal((sig), (handler))
#endif	/* Optimizing.  */

#endif	/* Use SVID.  */


/* Block signals in MASK, returning the old mask.  */
extern int EXFUN(__sigblock, (int __mask));

/* Set the mask of blocked signals to MASK, returning the old mask.  */
extern int EXFUN(__sigsetmask, (int __mask));

/* Set the mask of blocked signals to MASK,
   wait for a signal to arrive, and then restore the mask.  */
extern int EXFUN(__sigpause, (int __mask));

#ifdef	__USE_BSD
#define	sigmask(sig)	__sigmask(sig)

extern int EXFUN(sigblock, (int __mask));
extern int EXFUN(sigsetmask, (int __mask));
extern int EXFUN(sigpause, (int __mask));

#define	sigblock	__sigblock
#define	sigsetmask	__sigsetmask
#define	sigpause	__sigpause
#endif	/* Use BSD.  */


#ifdef	__USE_MISC
#define	NSIG	_NSIG
#endif

#ifdef	__USE_GNU
#define	sighandler_t	__sighandler_t
#endif

#endif	/* <signal.h> included.  */


#ifdef	__USE_POSIX

#if	!defined(__sigset_t_defined) &&	\
   (defined(_SIGNAL_H) || defined(__need_sigset_t))
#define	sigset_t	__sigset_t
#define	__sigset_t_defined	1
#endif	/* `sigset_t' not defined and <signal.h> or need `sigset_t'.  */
#undef	__need_sigset_t

#ifdef	_SIGNAL_H

/* Clear all signals from SET.  */
extern int EXFUN(sigemptyset, (sigset_t *__set));

/* Set all signals in SET.  */
extern int EXFUN(sigfillset, (sigset_t *__set));

/* Add SIGNO to SET.  */
extern int EXFUN(sigaddset, (sigset_t *__set, int __signo));

/* Remove SIGNO from SET.  */
extern int EXFUN(sigdelset, (sigset_t *__set, int __signo));

/* Return 1 if SIGNO is in SET, 0 if not.  */
extern int EXFUN(sigismember, (CONST sigset_t *__set, int signo));

/* Get and/or change the set of blocked signals.  */
extern int EXFUN(__sigprocmask, (int __how, CONST sigset_t *__set,
				 sigset_t *__oset));
extern int EXFUN(sigprocmask, (int __how, sigset_t *__set, sigset_t *__oset));

/* Values for the HOW argument to `sigprocmask'.  */
#define	SIG_BLOCK	1	/* Block signals.  */
#define	SIG_UNBLOCK	2	/* Unblock signals.  */
#define	SIG_SETMASK	3	/* Set the set of blocked signals.  */


/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
extern int EXFUN(sigsuspend, (CONST sigset_t *__set));

/* Bits the in `sa_flags' field of a `struct sigaction'.  */
#ifdef	__USE_BSD
#define	SA_ONSTACK	__SA_ONSTACK	/* Take signal on signal stack.  */
#define	SA_RESTART	__SA_RESTART	/* No syscall restart on sig ret.  */
#endif
#define	SA_NOCLDSTOP	__SA_NOCLDSTOP	/* No SIGCHLD when children stop.  */

/* Get and/or set the action for signal SIG.  */
extern int EXFUN(__sigaction, (int __sig, CONST struct __sigaction *__act,
			       struct __sigaction *__oact));
extern int EXFUN(sigaction, (int __sig, CONST struct __sigaction *__act,
			     struct __sigaction *__oact));

#define	sigaction	__sigaction

/* Put in SET all signals that are blocked and waiting to be delivered.  */
extern int EXFUN(sigpending, (sigset_t *__set));

#define	sigemptyset	__sigemptyset
#define	sigfillset	__sigfillset
#define	sigaddset	__sigaddset
#define	sigdelset	__sigdelset
#define	sigismember	__sigismember
#define	sigprocmask	__sigprocmask
#define	sigaction	__sigaction

#endif	/* <signal.h> included.  */

#endif	/* Use POSIX.  */

#if	defined(_SIGNAL_H) && defined(__USE_BSD)

/* Structure passed to `sigvec'.  */
struct __sigvec
  {
    __sighandler_t sv_handler;	/* Signal handler.  */
    int sv_mask;		/* Mask of signals to be blocked.  */

    int sv_flags;		/* Flags (see below).  */
#define	sv_onstack	sv_flags/* 4.2 BSD compatibility.  */
  };

/* Bits in `sv_flags'.  */
#define	SV_ONSTACK	(1 << 0)/* Take the signal on the signal stack.  */
#define	SV_INTERRUPT	(1 << 1)/* Do not restart system calls.  */
#define	SV_RESETHAND	(1 << 2)/* Reset handler to SIG_DFL on receipt.  */


/* If VEC is non-NULL, set the handler for SIG to the `sv_handler' member
   of VEC.  The signals in `sv_mask' will be blocked while the handler runs.
   If the SV_RESETHAND bit is set in `sv_flags', the handler for SIG will be
   reset to SIG_DFL before `sv_handler' is entered.  If OVEC is non-NULL,
   it is filled in with the old information for SIG.  */
extern int EXFUN(__sigvec, (int __sig, CONST struct __sigvec *__vec,
			    struct __sigvec *__ovec));
extern int EXFUN(sigvec, (int __sig, CONST struct __sigvec *__vec,
			  struct __sigvec *__ovec));
#define	sigvec	__sigvec


/* If INTERRUPT is nonzero, make signal SIG interrupt system calls
   (causing them to fail with EINTR); if INTERRUPT is zero, make system
   calls be restarted after signal SIG.  */
extern int EXFUN(siginterrupt, (int __sig, int __interrupt));


/* Structure describing a signal stack.  */
struct sigstack
  {
    PTR ss_sp;			/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.  */
  };

/* Run signals handlers on the stack specified by SS (if not NULL).
   If OSS is not NULL, it is filled in with the old signal stack status.  */
extern int EXFUN(sigstack, (CONST struct sigstack *__ss,
			    struct sigstack *__oss));


/* Get machine-dependent `struct sigcontext' and signal subcodes.  */
#include <sigcontext.h>

/* Restore the state saved in SCP.  */
extern int EXFUN(__sigreturn, (CONST struct sigcontext *__scp));
extern int EXFUN(sigreturn, (CONST struct sigcontext *__scp));
#define	sigreturn	__sigreturn


#endif	/* signal.h included and use BSD.  */

#endif	/* signal.h  */
