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

/*
 *	POSIX Standard: 6.5 File Control Operations	<fcntl.h>
 */

#ifndef	_FCNTL_H

#define	_FCNTL_H	1
#include <features.h>

/* This must be early so <fcntlbits.h> can define types winningly.  */
__BEGIN_DECLS

/* Get the definitions of O_*, F_*, FD_*: all the
   numbers and flag bits for `open', `fcntl', et al.  */
#include <fcntlbits.h>

#ifdef	__USE_MISC
#ifndef	R_OK			/* Verbatim from <unistd.h>.  Ugh.  */
/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4		/* Test for read permission.  */
#define	W_OK	2		/* Test for write permission.  */
#define	X_OK	1		/* Test for execute permission.  */
#define	F_OK	0		/* Test for existence.  */
#endif
#endif /* Use misc.  */


/* Do the file control operation described by CMD on FD.
   The remaining arguments are interpreted depending on CMD.  */
extern int __fcntl __P ((int __fd, int __cmd, ...));
extern int fcntl __P ((int __fd, int __cmd, ...));

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT is on OFLAG,
   the third argument is taken as a `mode_t', the mode of the created file.  */
extern int __open __P ((__const char *__file, int __oflag,...));
extern int open __P ((__const char *__file, int __oflag,...));

/* Create and open FILE, with mode MODE.
   This takes an `int' MODE argument because that is
   what `mode_t' will be widened to.  */
extern int creat __P ((__const char *__file, __mode_t __mode));

#ifdef	__OPTIMIZE__
#define	creat(file, m)	__open((file), O_WRONLY|O_CREAT|O_TRUNC, (m))
#endif /* Optimizing.  */

__END_DECLS

#endif /* fcntl.h  */
