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

/* These values should be changed as appropriate for your system.  */

#ifndef	_GNU_FCNTL_H

#define	_GNU_FCNTL_H	1


#include <gnu/types.h>


/* Values for the second argument to fcntl.  */
#define	__F_DUPFD	0	/* Duplicate file descriptor.  */
#define	__F_GETFD	1	/* Get file descriptor flags.  */
#define	__F_SETFD	2	/* Set file descriptor flags.  */
#define	__F_GETFL	3	/* Get file status flags.  */
#define	__F_SETFL	4	/* Set file status flags.  */
#define	__F_GETOWN	8	/* Get owner (receiver of SIGIO).  */
#define	__F_SETOWN	9	/* Set owner (receiver of SIGIO).  */
#define	__F_GETLK	5	/* Get record locking info.  */
#define	__F_SETLK	6	/* Set record locking info.  */
#define	__F_SETLKW	7	/* Set record locking info, wait.  */

/* File descriptor flags used with F_GETFD and F_SETFD.  */
#define	__FD_CLOEXEC	1	/* Close on exec.  */


/* The structure describing an advisory lock.  This is the type of the third
   argument to `fcntl' for the F_GETLK, F_SETLK, and F_SETLKW requests.  */
struct __flock
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off_t l_start;	/* Offset where the lock begins.  */
    __off_t l_len;	/* Size of the locked area; zero means until EOF.  */
    short int l_sysid;
    short int l_pid;	/* Process holding the lock.  */
  };

#define	__F_RDLCK	1	/* Read lock.  */
#define	__F_WRLCK	2	/* Write lock.  */
#define	__F_UNLCK	3	/* Remove lock.  */


#endif	/* gnu/fcntl.h */
