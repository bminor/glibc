/* Convert UTC calendar time to simple time.  Like mktime but assumes UTC.

   Copyright (C) 1994-2018 Free Software Foundation, Inc.
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

#ifndef _LIBC
# include <config.h>
#endif

#include <time.h>

#include "mktime-internal.h"
#include <errno.h>

__time64_t
__timegm64 (struct tm *tmp)
{
  static long int gmtime_offset;
  tmp->tm_isdst = 0;
  return __mktime_internal (tmp, __gmtime64_r, &gmtime_offset);
}

#if __TIMESIZE != 64

time_t
timegm (struct tm *tmp)
{
  __time64_t t64 = __timegm64 (tmp);
  if (fits_in_time_t (t64))
    return t64;
  __set_errno (EOVERFLOW);
  return -1;
}

#endif
