/* Support code for timespec checks.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#include <support/timespec.h>
#include <stdio.h>
#include <stdint.h>

void
test_timespec_before_impl (const char *file, int line,
			   const struct timespec left,
			   const struct timespec right)
{
  if (left.tv_sec > right.tv_sec
      || (left.tv_sec == right.tv_sec
	  && left.tv_nsec > right.tv_nsec)) {
    support_record_failure ();
    const struct timespec diff = timespec_sub (left, right);
    printf ("%s:%d: %jd.%09jds not before %jd.%09jds "
	    "(difference %jd.%09jds)\n",
	    file, line,
	    (intmax_t) left.tv_sec, (intmax_t) left.tv_nsec,
	    (intmax_t) right.tv_sec, (intmax_t) right.tv_nsec,
	    (intmax_t) diff.tv_sec, (intmax_t) diff.tv_nsec);
  }
}

void
test_timespec_equal_or_after_impl (const char *file, int line,
				   const struct timespec left,
				   const struct timespec right)
{
  if (left.tv_sec < right.tv_sec
      || (left.tv_sec == right.tv_sec
	  && left.tv_nsec < right.tv_nsec)) {
    support_record_failure ();
    const struct timespec diff = timespec_sub (right, left);
    printf ("%s:%d: %jd.%09jds not after %jd.%09jds "
	    "(difference %jd.%09jds)\n",
	    file, line,
	    (intmax_t) left.tv_sec, (intmax_t) left.tv_nsec,
	    (intmax_t) right.tv_sec, (intmax_t) right.tv_nsec,
	    (intmax_t) diff.tv_sec, (intmax_t) diff.tv_nsec);
  }
}
