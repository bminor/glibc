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

/*
 *	POSIX Standard: 4.5.2 Process Times	<sys/times.h>
 */

#ifndef	_SYS_TIMES_H

#define	_SYS_TIMES_H	1
#include <features.h>

#define	__need_clock_t
#include <time.h>


/* Structure describing CPU time used by a process and its children.  */
struct tms
  {
    clock_t tms_utime;	/* User CPU time.  */
    clock_t tms_stime;	/* System CPU time.  */

    clock_t tms_cutime;	/* User CPU time of dead children.  */
    clock_t tms_cstime;	/* System CPU time of dead children.  */
  };


/* Store the CPU time used by this process and all its
   dead children (and their dead children) in BUFFER.
   Return the elapsed real time, or (clock_t) -1 for errors.
   All times are in CLK_TCKths of a second.  */
extern clock_t EXFUN(__times, (struct tms *buffer));
extern clock_t EXFUN(times, (struct tms *buffer));

#ifdef	__OPTIMIZE__
#define	times(buffer)	__times(buffer)
#endif	/* Optimizing.  */

#endif	/* sys/times.h	*/
