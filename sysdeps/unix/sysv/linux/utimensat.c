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
#include <sys/stat.h>
#include <sysdep.h>
#include <y2038-support.h>


/* Change the access time of FILE to TSP[0] and
   the modification time of FILE to TSP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall.  */
int
utimensat (int fd, const char *file, const struct timespec tsp[2],
	   int flags)
{
  if (file == NULL)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
  /* Avoid implicit array coercion in syscall macros.  */
  return INLINE_SYSCALL (utimensat, 4, fd, file, &tsp[0], flags);
}

/* 64-bit time version */

int
__utimensat_time64 (int fd, const char *file, const struct __timespec64 tsp[2],
		    int flags)
{
  struct timespec ts32[2], *ts32p = NULL;
/* Only try and use this syscall if defined by kernel */
#ifdef __NR_utimensat_time64
  struct __timespec64 ts64[2], *ts64p = NULL;
  int res;
#endif

  if (file == NULL)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

/* Only try and use this syscall if defined by kernel */
#ifdef __NR_utimensat_time64
  if (__y2038_linux_support > 0)
    {
      if (tsp)
        {
          ts64[0].tv_sec = tsp[0].tv_sec;
          ts64[0].tv_nsec = tsp[0].tv_nsec;
          ts64[0].tv_pad = 0;
          ts64[1].tv_sec = tsp[1].tv_sec;
          ts64[1].tv_nsec = tsp[1].tv_nsec;
          ts64[1].tv_pad = 0;
          ts64p = ts64;
        }

      res = INLINE_SYSCALL (utimensat_time64, 4, fd, file, ts64p, flags);
      if (res == 0 || errno != ENOSYS)
        return res;
      __y2038_linux_support = -1;
    }
#endif

  if (tsp)
    {
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
      ts32p = ts32;
    }

  return INLINE_SYSCALL (utimensat, 4, fd, file, ts32p, flags);
}
