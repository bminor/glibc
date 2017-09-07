/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <include/time.h>

#define MAX_SEC	(INT_MAX / 1000000L - 2)
#define MIN_SEC	(INT_MIN / 1000000L + 2)

#ifndef MOD_OFFSET
#define modes mode
#endif

#ifndef TIMEVAL
#define TIMEVAL timeval
#endif

#ifndef TIMEX
#define TIMEX timex
#endif

#ifndef ADJTIME
#define ADJTIME __adjtime
#endif

#ifndef ADJTIMEX
#define NO_LOCAL_ADJTIME
#define ADJTIMEX(x) __adjtimex (x)
#endif

#ifndef LINKAGE
#define LINKAGE
#endif

int __adjtime64 (const struct __timeval64 *itv,
                 struct __timeval64 *otv)
{
  struct TIMEX tntx;

  if (itv)
    {
      struct TIMEVAL tmp;

      /* We will do some check here. */
      tmp.tv_sec = itv->tv_sec + itv->tv_usec / 1000000L;
      tmp.tv_usec = itv->tv_usec % 1000000L;
      if (tmp.tv_sec > MAX_SEC || tmp.tv_sec < MIN_SEC)
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
      tntx.offset = tmp.tv_usec + tmp.tv_sec * 1000000L;
      tntx.modes = ADJ_OFFSET_SINGLESHOT;
    }
  else
    tntx.modes = ADJ_OFFSET_SS_READ;

  if (__glibc_unlikely (ADJTIMEX (&tntx) < 0))
    return -1;

  if (otv)
    {
      if (tntx.offset < 0)
	{
	  otv->tv_usec = -(-tntx.offset % 1000000);
	  otv->tv_sec  = -(-tntx.offset / 1000000);
	}
      else
	{
	  otv->tv_usec = tntx.offset % 1000000;
	  otv->tv_sec  = tntx.offset / 1000000;
	}
    }
  return 0;
}

int
__adjtimex64(struct __timex64 *tx)
{
  struct timex tx32;

  if (tx == NULL)
    {
      __set_errno (EFAULT);
      return -1;
    }

  if ((tx->modes & ADJ_SETOFFSET) != 0 && tx->time.tv_sec > INT_MAX)
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  /* Implement with existing 32-bit time syscall */

  /* Just copy everything */
  tx32.modes = tx->modes;
  tx32.offset = tx->offset;
  tx32.freq = tx->freq;
  tx32.maxerror = tx->maxerror;
  tx32.esterror = tx->esterror;
  tx32.status = tx->status;
  tx32.constant = tx->constant;
  tx32.precision = tx->precision;
  tx32.tolerance = tx->tolerance;
  tx32.time.tv_sec = tx->time.tv_sec;
  tx32.time.tv_usec = tx->time.tv_usec;
  tx32.tick = tx->tick;
  tx32.ppsfreq = tx->ppsfreq;
  tx32.jitter = tx->jitter;
  tx32.shift = tx->shift;
  tx32.stabil = tx->stabil;
  tx32.jitcnt = tx->jitcnt;
  tx32.calcnt = tx->calcnt;
  tx32.errcnt = tx->errcnt;
  tx32.stbcnt = tx->stbcnt;

  tx32.tai = tx->tai;
  /* WARNING -- anonymous fields after TAI are not copied. */

  int result = ADJTIMEX(&tx32);

  if (result == 0)
    {
      /* Just copy back everything */
      tx->modes = tx32.modes;
      tx->offset = tx32.offset;
      tx->freq = tx32.freq;
      tx->maxerror = tx32.maxerror;
      tx->esterror = tx32.esterror;
      tx->status = tx32.status;
      tx->constant = tx32.constant;
      tx->precision = tx32.precision;
      tx->tolerance = tx32.tolerance;
      tx->time.tv_sec = tx32.time.tv_sec;
      tx->time.tv_usec = tx32.time.tv_usec;
      tx->tick = tx32.tick;
      tx->ppsfreq = tx32.ppsfreq;
      tx->jitter = tx32.jitter;
      tx->shift = tx32.shift;
      tx->stabil = tx32.stabil;
      tx->jitcnt = tx32.jitcnt;
      tx->calcnt = tx32.calcnt;
      tx->errcnt = tx32.errcnt;
      tx->stbcnt = tx32.stbcnt;
    }

  return result;
}
weak_alias (__adjtimex64, __ntp_adjtime64);
