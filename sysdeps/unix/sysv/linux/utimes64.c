/* Copyright (C) 1995-2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <utime.h>
#include <sys/time.h>
#include <sysdep.h>
#include <fcntl.h>

/* 64-bit-time variant */

int
__utimes64 (const char *file, const struct __timeval64 tvp[2])
{
  struct timespec ts32[2], *tsp32 = NULL;

  if (tvp != NULL)
    {
      if (tvp[0].tv_sec > INT_MAX || tvp[1].tv_sec > INT_MAX)
        {
          __set_errno(EOVERFLOW);
          return -1;
        }
      ts32[0].tv_sec = tvp[0].tv_sec;
      ts32[0].tv_nsec = tvp[0].tv_usec * 1000;
      ts32[1].tv_sec = tvp[1].tv_sec;
      ts32[1].tv_nsec = tvp[1].tv_usec * 1000;
      tsp32 = ts32;
    }

  /* use utimensat rather than utimes which not all arches can use */
  return INLINE_SYSCALL (utimensat, 4, AT_FDCWD, file, tsp32, 0);
}
