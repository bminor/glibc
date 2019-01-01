/* Query filename corresponding to an open FD.  Linux version.
   Copyright (C) 2001-2019 Free Software Foundation, Inc.
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

#include <sys/stat.h>
#include <string.h>
#include <_itoa.h>

#define FD_TO_FILENAME_SIZE ((sizeof ("/proc/self/fd/") - 1) \
			     + (sizeof ("4294967295") - 1) + 1)

static inline const char *
fd_to_filename (unsigned int fd, char *buf)
{
  *_fitoa_word (fd, __stpcpy (buf, "/proc/self/fd/"), 10, 0) = '\0';

  /* We must make sure the file exists.  */
  struct stat64 st;
  if (__lxstat64 (_STAT_VER, buf, &st) < 0)
    /* /proc is not mounted or something else happened.  */
    return NULL;
  return buf;
}
