/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

#include <sys/timex.h>
#include <include/time.h>

#ifndef MOD_OFFSET
# define modes mode
#endif


int
ntp_gettimex (struct ntptimeval *ntv)
{
  struct timex tntx;
  int result;

  tntx.modes = 0;
  result = __adjtimex (&tntx);
  ntv->time = tntx.time;
  ntv->maxerror = tntx.maxerror;
  ntv->esterror = tntx.esterror;
  ntv->tai = tntx.tai;
  ntv->__glibc_reserved1 = 0;
  ntv->__glibc_reserved2 = 0;
  ntv->__glibc_reserved3 = 0;
  ntv->__glibc_reserved4 = 0;
  return result;
}

/* The 64-bit-time version */

int
__ntp_gettimex64 (struct __ntptimeval64 *ntv)
{
  struct timex tntx;
  int result;

  tntx.modes = 0;
  result = __adjtimex (&tntx);
  ntv->time.tv_sec = tntx.time.tv_sec;
  ntv->time.tv_usec = tntx.time.tv_usec;
  ntv->maxerror = tntx.maxerror;
  ntv->maxerror = tntx.maxerror;
  ntv->esterror = tntx.esterror;
  ntv->tai = tntx.tai;
  ntv->__glibc_reserved1 = 0;
  ntv->__glibc_reserved2 = 0;
  ntv->__glibc_reserved3 = 0;
  ntv->__glibc_reserved4 = 0;
  return result;
}
