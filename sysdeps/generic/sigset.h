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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SIGSET_H
#define	_SIGSET_H	1

typedef int __sig_atomic_t;

/* Return a mask that includes SIG only.
   The cast to `sigset_t' avoids overflow
   if `sigset_t' is wider than `int'.  */
#define	__sigmask(sig)	(((sigset_t) 1) << ((sig) - 1))

/* A `sigset_t' has a bit for each signal.  */
typedef unsigned long int __sigset_t;

#define	__sigemptyset(set)	((*(set) = 0L), 0)
#define	__sigfillset(set)	((*(set) = -1L), 0)
#define	__sigaddset(set, sig)	((*(set) |= __sigmask (sig)), 0)
#define	__sigdelset(set, sig)	((*(set) &= ~__sigmask (sig)), 0)
#define	__sigismember(set, sig)	((*(set) & __sigmask (sig)) ? 1 : 0)


#endif /* sigset.h  */
