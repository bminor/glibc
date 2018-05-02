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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _LIBC
# include <time.h>
#else
# include "timegm.h"

/* Portable standalone applications should supply a "time_r.h" that
   declares a POSIX-compliant gmtime_r, for the benefit of older
   implementations that lack gmtime_r or have a nonstandard one.
   See the gnulib time_r module for one way to implement this.  */
# include <time_r.h>
# undef __gmtime_r
# define __gmtime_r gmtime_r
time64_t __mktime_internal (struct tm *,
			  struct tm * (*) (__time64_t const *, struct tm *),
			  __time64_t *);
#endif
# include <limits.h>
# include <errno.h>

__time64_t
__timegm64 (struct tm *tmp)
{
  static __time64_t gmtime_offset;
  tmp->tm_isdst = 0;
  return __mktime_internal (tmp, __gmtime64_r, &gmtime_offset);
}

time_t
timegm (struct tm *tmp)
{
  __time64_t result = __timegm64(tmp);
  /* Result may be correct for __mktime_internal() which handles 64-bit
     time, but still beyond 32-bit time_t */
  if (fits_in_time_t (result))
    return (time_t) result;
  __set_errno(EOVERFLOW);
  return (time_t) -1;
}
