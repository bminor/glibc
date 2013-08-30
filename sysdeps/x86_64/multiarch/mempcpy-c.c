/* C-version of mempcpy for using when Intel MPX is enabled
   in order to process with an array of pointers correctly.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <stddef.h>

void *
mempcpy (void *dst, const void *src, size_t n)
{
  return memcpy(dst, src, n) + n;
}

void *
chkp_mempcpy_nochk (void *dst, const void *src, size_t n)
{
  return chkp_memcpy_nochk(dst, src, n) + n;
}

weak_alias (mempcpy, __GI_mempcpy)
weak_alias (mempcpy, __GI___mempcpy)
weak_alias (mempcpy, __mempcpy)
