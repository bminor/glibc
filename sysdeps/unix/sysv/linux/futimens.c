/* Change access and modification times of open file.  Linux version.
   Copyright (C) 2007-2018 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sysdep.h>
#include <y2038-support.h>

/* Change the access time of the file associated with FD to TSP[0] and
   the modification time of FILE to TSP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall which
   can be used to implement futimens.  */
int
futimens (int fd, const struct timespec tsp[2])
{
  if (fd < 0)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EBADF);
  /* Avoid implicit array coercion in syscall macros.  */
  return INLINE_SYSCALL (utimensat, 4, fd, NULL, &tsp[0], 0);
}

/* 64-bit time version */

int
__futimens64 (int fd, const struct __timespec64 tsp[2])
{
  struct timespec ts32[2];
/* Only try and use this syscall if defined by kernel */
#ifdef __NR_utimesat_time64
  struct __timespec64 ts64[2];
  int res;
#endif

  if (fd < 0)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EBADF);

/* Only try and use this syscall if defined by kernel */
#ifdef __NR_utimesat_time64
  if (__y2038_linux_support > 0)
    {
      ts64[0].tv_sec = tsp[0].tv_sec;
      ts64[0].tv_nsec = tsp[0].tv_nsec;
      ts64[0].tv_pad = 0;
      ts64[1].tv_sec = tsp[1].tv_sec;
      ts64[1].tv_nsec = tsp[1].tv_nsec;
      ts64[1].tv_pad = 0;
      res = INLINE_SYSCALL (utimensat_time64, 4, fd, NULL, &ts64[0], 0);
      if (res == 0 || errno != ENOSYS)
        return res;
      __y2038_linux_support = -1;
    }
#endif

  if (! timespec64_to_timespec(&tsp[0], &ts32[0]))
    {
      __set_errno(EOVERFLOW);
      return -1;
    }

  if (! timespec64_to_timespec(&tsp[1], &ts32[1]))
    {
      __set_errno(EOVERFLOW);
      return -1;
    }

  return INLINE_SYSCALL (utimensat, 4, fd, NULL, &ts32[0], 0);
}
