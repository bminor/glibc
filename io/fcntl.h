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
 *	POSIX Standard: 6.5 File Control Operations	<fcntl.h>
 */

#ifndef	_FCNTL_H

#define	_FCNTL_H	1
#include <features.h>

#include <fcntlbits.h>
#include <filebits.h>


/* Values for the second argument to fcntl.  */
#define	F_DUPFD		__F_DUPFD	/* Duplicate file descriptor.  */
#define	F_GETFD		__F_GETFD	/* Get file descriptor flags.  */
#define	F_SETFD		__F_SETFD	/* Set file descriptor flags.  */
#define	F_GETFL		__F_GETFL	/* Get file status flags.  */
#define	F_SETFL		__F_SETFL	/* Set file status flags.  */
#define	F_GETLK		__F_GETLK	/* Get record locking info.  */
#define	F_SETLK		__F_SETLK	/* Set record locking info.  */
#define	F_SETLKW	__F_SETLKW	/* Set record locking, block.  */

#ifdef	__USE_BSD
#define	F_GETOWN	__F_GETOWN	/* Get owner (receiver of SIGIO).  */
#define	F_SETOWN	__F_SETOWN	/* Set owner (receiver of SIGIO).  */
#endif


/* File descriptor flags used with F_GETFD and F_SETFD.  */
#define	FD_CLOEXEC	__FD_CLOEXEC	/* Close on exec.  */


/* File access modes for open and fcntl.  */
#define	O_RDONLY	__O_RDONLY	/* Open read-only.  */
#define	O_WRONLY	__O_WRONLY	/* Open write-only.  */
#define	O_RDWR		__O_RDWR	/* Open read/write.  */


/* Bits OR'd into the second argument to open.  */
#define	O_CREAT		__O_CREAT	/* Create file if it doesn't exist.  */
#define	O_EXCL		__O_EXCL	/* Fail if file already exists.  */
#define	O_TRUNC		__O_TRUNC	/* Truncate file to zero length.  */
#define	O_NOCTTY	__O_NOCTTY	/* Don't assign a controlling tty.  */


/* File status flags for `open' and `fcntl'.  */
#define	O_APPEND	__O_APPEND	/* Writes append to the file.  */
#define	O_NONBLOCK	__O_NONBLOCK	/* No delay when opening the file.  */

#ifdef	__USE_BSD
#define	O_NDELAY	__O_NDELAY
#endif

#ifdef	__USE_MISC
/* Unix-style flags for `fcntl' F_GETFL and F_SETFL.  */
#define	FREAD		0x1		/* Read access.  */
#define	FWRITE		0x2		/* Write access.  */
#define	FNDELAY		O_NONBLOCK
#define	FAPPEND		O_APPEND
#define	FASYNC		0x40		/* Send SIGIO when data is ready.  */
#define	FCREAT		O_CREAT
#define	FTRUNC		O_TRUNC
#define	FEXCL		O_EXCL
#define	FSYNC		0x2000		/* Synchronous writes.  */

#ifndef	R_OK	/* Verbatim from <unistd.h>.  Ugh.  */
/* Values for the second argument to access.
   These may be OR'd together.  */
#define	R_OK	4	/* Test for read permission.  */
#define	W_OK	2	/* Test for write permission.  */
#define	X_OK	1	/* Test for execute permission.  */
#define	F_OK	0	/* Test for existence.  */
#endif
#endif	/* Use misc.  */


/* Mask for file access modes.  */
#define	O_ACCMODE	__O_ACCMODE


/* Do the file control operation described by CMD on FD.
   The remaining arguments are interpreted depending on CMD.  */
extern int EXFUN(__fcntl, (int __fd, int __cmd, ...));

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT is on OFLAG,
   the third argument is taken as a `mode_t', the mode of the created file.  */
extern int EXFUN(__open, (CONST char *__file, int __oflag, ...));

#define	fcntl	__fcntl
#define	open	__open

/* Create and open FILE, with mode MODE.
   This takes an `int' MODE argument because that is
   what `mode_t' will be widened to.  */
extern int EXFUN(creat, (CONST char *__file, __mode_t __mode));

#ifdef	__OPTIMIZE__
#define	creat(file, m)	__open((file), O_WRONLY|O_CREAT|O_TRUNC, (m))
#endif	/* Optimizing.  */

#define	flock	__flock
#define	F_RDLCK	__F_RDLCK
#define	F_WRLCK	__F_WRLCK
#define	F_UNLCK	__F_UNLCK


#endif	/* fcntl.h  */
