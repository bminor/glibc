/* Change access and modification times of open file.  Linux version.
   Copyright (C) 2007-2020 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/stat.h>
#include <sysdep.h>
#include <time.h>
#include <kernel-features.h>

/* Helper function defined for easy reusage of the code which calls utimensat
   and utimensat_time64 syscall.  */
int
__utimensat64_helper (int fd, const char *file,
                      const struct __timespec64 tsp64[2], int flags)
{
#ifdef __ASSUME_TIME64_SYSCALLS
# ifndef __NR_utimensat_time64
#  define __NR_utimensat_time64 __NR_utimensat
# endif
  return INLINE_SYSCALL (utimensat_time64, 4, fd, file, &tsp64[0], flags);
#else
# ifdef __NR_utimensat_time64
  int ret = INLINE_SYSCALL (utimensat_time64, 4, fd, file, &tsp64[0], flags);
  if (ret == 0 || errno != ENOSYS)
    return ret;
# endif
  if (tsp64
      && (! in_time_t_range (tsp64[0].tv_sec)
          || ! in_time_t_range (tsp64[1].tv_sec)))
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  struct timespec tsp32[2];
  if (tsp64)
    {
      tsp32[0] = valid_timespec64_to_timespec (tsp64[0]);
      tsp32[1] = valid_timespec64_to_timespec (tsp64[1]);
    }

  return INLINE_SYSCALL (utimensat, 4, fd, file, tsp64 ? &tsp32[0] : NULL,
                         flags);
#endif

}
libc_hidden_def (__utimensat64_helper)

/* Change the access time of FILE to TSP[0] and
   the modification time of FILE to TSP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall.  */
int
__utimensat64 (int fd, const char *file, const struct __timespec64 tsp64[2],
               int flags)
{
  if (file == NULL)
    return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

  return __utimensat64_helper (fd, file, &tsp64[0], flags);
}

#if __TIMESIZE != 64
int
__utimensat (int fd, const char *file, const struct timespec tsp[2],
             int flags)
{
  struct __timespec64 tsp64[2];
  if (tsp)
    {
      tsp64[0] = valid_timespec_to_timespec64 (tsp[0]);
      tsp64[1] = valid_timespec_to_timespec64 (tsp[1]);
    }

  return __utimensat64 (fd, file, tsp ? &tsp64[0] : NULL, flags);
}
#endif
weak_alias (__utimensat, utimensat)
