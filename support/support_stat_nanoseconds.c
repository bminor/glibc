/* Check if stat supports nanosecond resolution.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <stdbool.h>

bool
support_stat_nanoseconds (void)
{
  /* s390 stat64 compat symbol does not support nanoseconds resolution
     and it used on non-LFS [f,l]stat[at] implementations.  */
#if defined __linux__ && !defined __s390x__ && defined __s390__
  return false;
#else
  return true;
#endif
}
