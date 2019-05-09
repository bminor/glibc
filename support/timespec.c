/* Support code for timespec checks.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <support/timespec.h>
#include <stdio.h>

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
    printf ("%s:%d: %ld.%09lds not before %ld.%09lds "
	    "(difference %ld.%09lds)n",
	    file, line,
	    left.tv_sec, left.tv_nsec,
	    right.tv_sec, right.tv_nsec,
	    diff.tv_sec, diff.tv_nsec);
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
    printf ("%s:%d: %ld.%09lds not after %ld.%09lds "
	    "(difference %ld.%09lds)n",
	    file, line,
	    left.tv_sec, left.tv_nsec,
	    right.tv_sec, right.tv_nsec,
	    diff.tv_sec, diff.tv_nsec);
  }
}
