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

#ifndef	_SYS_FILE_H

#define	_SYS_FILE_H	1
#include <features.h>

#include <fcntl.h>


/* Alternate names for values for the WHENCE argument to `lseek'.  */
#define	L_SET	__L_SET	/* Seek from beginning of file.  */
#define	L_INCR	__L_INCR/* Seek from current position.  */
#define	L_XTND	__L_XTND/* Seek from end of file.  */


/* Operations for the `flock' call.  */
#define	LOCK_SH	__LOCK_SH    /* Shared lock.  */
#define	LOCK_EX	__LOCK_EX    /* Exclusive lock.  */
#define	LOCK_UN	__LOCK_UN    /* Unlock.  */

/* Can be OR'd in to one of the above.  */
#define	LOCK_NB	__LOCK_NB    /* Don't block when locking.  */


/* Apply or remove an advisory lock, according to OPERATION,
   on the file FD refers to.  */
extern int EXFUN(__flock, (int __fd, int __operation));

/* It just so happens that <fcntl.h> does #define flock __flock.  Bletch.  */


#endif	/* sys/file.h  */
