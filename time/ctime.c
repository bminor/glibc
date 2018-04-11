/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#include <time.h>
#include <errno.h>

/* Return a string as returned by asctime which
   is the representation of *T in that form.  */
char *
__ctime64 (const __time64_t *t)
{
  /* Apply the same rule as ctime:
     make ctime64 (t) is equivalent to asctime (localtime64 (t)).  */
  return asctime (__localtime64 (t));
}

/* The 32-bit time wrapper.  */
char *
ctime (const time_t *t)
{
  __time64_t t64;
  if (t == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }
  t64 = *t;
  return __ctime64 (&t64);
}
