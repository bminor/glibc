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

#  include <time.h>
#  include <struct___timeval64.h>
/* Local definition of 64 bit time supporting timex struct */
#  if __TIMESIZE == 64
#   define __timex64 timex
#   define __clock_adjtime64 __clock_adjtime
#   define ___adjtimex64 ___adjtimex
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
extern int __clock_adjtime64 (const clockid_t clock_id, struct __timex64 *tx64);
libc_hidden_proto (__clock_adjtime64);
extern int ___adjtimex64 (struct __timex64 *tx64);
libc_hidden_proto (___adjtimex64)
#  endif

/* Convert a known valid struct timex into a struct __timex64.  */
static inline struct __timex64
valid_timex_to_timex64 (const struct timex tx)
{
  struct __timex64 tx64;

  tx64.modes = tx.modes;
  tx64.offset = tx.offset;
  tx64.freq = tx.freq;
  tx64.maxerror = tx.maxerror;
  tx64.esterror = tx.esterror;
  tx64.status = tx.status;
  tx64.constant = tx.constant;
  tx64.precision = tx.precision;
  tx64.tolerance = tx.tolerance;
  tx64.time = valid_timeval_to_timeval64 (tx.time);
  tx64.tick = tx.tick;
  tx64.ppsfreq = tx.ppsfreq;
  tx64.jitter = tx.jitter;
  tx64.shift = tx.shift;
  tx64.stabil = tx.stabil;
  tx64.jitcnt = tx.jitcnt;
  tx64.calcnt = tx.calcnt;
  tx64.errcnt = tx.errcnt;
  tx64.stbcnt = tx.stbcnt;
  tx64.tai = tx.tai;

  return tx64;
}

/* Convert a known valid struct __timex64 into a struct timex.  */
static inline struct timex
valid_timex64_to_timex (const struct __timex64 tx64)
{
  struct timex tx;

  tx.modes = tx64.modes;
  tx.offset = tx64.offset;
  tx.freq = tx64.freq;
  tx.maxerror = tx64.maxerror;
  tx.esterror = tx64.esterror;
  tx.status = tx64.status;
  tx.constant = tx64.constant;
  tx.precision = tx64.precision;
  tx.tolerance = tx64.tolerance;
  tx.time = valid_timeval64_to_timeval (tx64.time);
  tx.tick = tx64.tick;
  tx.ppsfreq = tx64.ppsfreq;
  tx.jitter = tx64.jitter;
  tx.shift = tx64.shift;
  tx.stabil = tx64.stabil;
  tx.jitcnt = tx64.jitcnt;
  tx.calcnt = tx64.calcnt;
  tx.errcnt = tx64.errcnt;
  tx.stbcnt = tx64.stbcnt;
  tx.tai = tx64.tai;

  return tx;
}
# endif /* _ISOMAC */
#endif /* sys/timex.h */
