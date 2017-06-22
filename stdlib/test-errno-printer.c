/* Helper program for testing the errno pretty-printer.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#define _GNU_SOURCE 1
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#define PASS 0
#define FAIL 1

const error_t array_of_error_t[3] = { 0, ERANGE, -2 };

__thread int ensure_gdb_can_read_thread_variables = 0;

int
main (void)
{
  int result = PASS;
  errno = array_of_error_t[0];
  unsigned long x = strtoul("9999999999999999999999999999999999999", 0, 10);
  if (x != ULONG_MAX)
    result = FAIL;
  if (errno != ERANGE)
    result = FAIL;
  errno = -2; /* Break: test errno 2 */
  return result;
}
