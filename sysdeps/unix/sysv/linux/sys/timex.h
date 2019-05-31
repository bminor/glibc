/* Copyright (C) 1995-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef	_SYS_TIMEX_H
#define	_SYS_TIMEX_H	1

#include <features.h>

/* Get struct timex and related constants.  */
#include <bits/timex.h>

/* Parameter structure used by ntp_gettime(x).  */
struct ntptimeval
{
  struct timeval time;	/* current time (ro) */
  long int maxerror;	/* maximum error (us) (ro) */
  long int esterror;	/* estimated error (us) (ro) */
  long int tai;		/* TAI offset (ro) */

  long int __glibc_reserved1;
  long int __glibc_reserved2;
  long int __glibc_reserved3;
  long int __glibc_reserved4;
};


__BEGIN_DECLS

extern int __adjtimex (struct timex *__ntx) __THROW;
extern int adjtimex (struct timex *__ntx) __THROW;

#ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (ntp_gettime, (struct ntptimeval *__ntv),
			   ntp_gettimex);
#else
extern int ntp_gettimex (struct ntptimeval *__ntv) __THROW;
# define ntp_gettime ntp_gettimex
#endif
extern int ntp_adjtime (struct timex *__tntx) __THROW;

__END_DECLS

#endif /* sys/timex.h */
