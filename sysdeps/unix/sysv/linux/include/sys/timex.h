/* Internal declarations for sys/timex.h.
   Copyright (C) 2014-2020 Free Software Foundation, Inc.
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

#ifndef	_INCLUDE_SYS_TIMEX_H
#define	_INCLUDE_SYS_TIMEX_H	1

#include_next <sys/timex.h>

# ifndef _ISOMAC

libc_hidden_proto (__adjtimex)

#  include <struct___timeval64.h>
/* Local definition of 64 bit time supporting timex struct */
#  if __TIMESIZE == 64
#   define __timex64 timex
#  else

struct __timex64
{
  unsigned int modes;          /* mode selector */
  int :32;                     /* pad */
  long long int offset;            /* time offset (usec) */
  long long int freq;              /* frequency offset (scaled ppm) */
  long long int maxerror;          /* maximum error (usec) */
  long long int esterror;          /* estimated error (usec) */
  int status;                  /* clock command/status */
  int :32;                     /* pad */
  long long int constant;          /* pll time constant */
  long long int precision;         /* clock precision (usec) (read only) */
  long long int tolerance;         /* clock frequency tolerance (ppm) (ro) */
  struct __timeval64 time;     /* (read only, except for ADJ_SETOFFSET) */
  long long int tick;              /* (modified) usecs between clock ticks */
  long long int ppsfreq;           /* pps frequency (scaled ppm) (ro) */
  long long int jitter;            /* pps jitter (us) (ro) */
  int shift;                   /* interval duration (s) (shift) (ro) */
  int :32;                     /* pad */
  long long int stabil;            /* pps stability (scaled ppm) (ro) */
  long long int jitcnt;            /* jitter limit exceeded (ro) */
  long long int calcnt;            /* calibration intervals (ro) */
  long long int errcnt;            /* calibration errors (ro) */
  long long int stbcnt;            /* stability limit exceeded (ro) */

  int tai;                     /* TAI offset (ro) */

  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
  int  :32;
};
#  endif
# endif /* _ISOMAC */
#endif /* sys/timex.h */
