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

#ifndef _SYS_TIME_H

#if	!defined(__need_timeval)
#define _SYS_TIME_H	1
#include <features.h>
#endif

#include <gnu/time.h>


#ifdef	_SYS_TIME_H
#include <time.h>
#endif


#if	(!defined(__timeval_defined) &&	\
	 (defined(_SYS_TIME_H) || defined(__need_timeval)))
#define	__timeval_defined	1
#define	timeval	__timeval
#endif
#undef	__need_timeval


/* <time.h> already does #define timezone __timezone.  */

#ifdef	_SYS_TIME_H

#define	itimerval	__itimerval

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
extern int EXFUN(__gettimeofday, (struct __timeval *__tv,
				  struct __timezone *__tz));

/* Set the current time of day and timezone information.
   This call is restricted to the super-user.  */
extern int EXFUN(__settimeofday, (CONST struct __timeval *__tv,
				  CONST struct __timezone *__tz));

/* Adjust the current time of day by the amount in DELTA.
   If OLDDELTA is not NULL, it is filled in with the amount
   of time adjustment remaining to be done from the last `adjtime' call.
   This call is restricted to the super-user.  */
extern int EXFUN(__adjtime, (CONST struct __timeval *__delta,
			     struct __timeval *__olddelta));

#define	gettimeofday	__gettimeofday
#define	settimeofday	__settimeofday
#define	adjtime		__adjtime

/* Set *VALUE to the current setting of timer WHICH.
   Return 0 on success, -1 on errors.  */
extern int EXFUN(__getitimer, (enum __itimer_which __which,
			       struct __itimerval *__value));

/* Set the timer WHICH to *NEW.  If OLD is not NULL,
   set *OLD to the old value of timer WHICH.
   Returns 0 on success, -1 on errors.  */
extern int EXFUN(__setitimer, (enum __itimer_which __which,
			       struct __itimerval *__old,
			       struct __itimerval *__new));

/* Change the access time of FILE to TVP[0] and
   the modification time of FILE to TVP[1].  */
extern int EXFUN(__utimes, (CONST char *__file, struct __timeval __tvp[2]));

#define	getitimer	__getitimer
#define	setitimer	__setitimer
#define	utimes		__utimes


#endif	/* <sys/time.h> included.  */

#endif	/* sys/time.h */
