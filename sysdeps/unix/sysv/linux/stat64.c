/* Get file status.  Linux version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#define __stat __redirect___stat
#define stat   __redirect_stat
#include <sys/stat.h>
#undef __stat
#undef stat
#include <fcntl.h>
#include <kernel_stat.h>

int
__stat64 (const char *file, struct stat64 *buf)
{
  return __fstatat64 (AT_FDCWD, file, buf, 0);
}
hidden_def (__stat64)
weak_alias (__stat64, stat64)

#if XSTAT_IS_XSTAT64
strong_alias (__stat64, __stat)
weak_alias (__stat64, stat)
#endif
