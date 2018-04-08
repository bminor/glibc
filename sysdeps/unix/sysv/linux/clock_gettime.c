/* clock_gettime -- Get current time from a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <time.h>
#include "kernel-posix-cpu-timers.h"

#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>
#include <y2038-support.h>

# define DO_CLOCK_GETTIME_32 \
  retval = INLINE_VSYSCALL (clock_gettime, 2, clock_id, &ts32);		\
  if (retval == 0)							\
    {									\
      valid_timespec_to_timespec64 (&ts32, tp);				\
    }

#ifdef __NR_clock_gettime64

/* We are building with a 64-bit-time clock_gettime syscall */

# define DO_CLOCK_GETTIME_64 \
    if (__y2038_linux_support > 0)					\
      {									\
	retval = INLINE_SYSCALL (clock_gettime64, 2, clock_id, tp);	\
	if (retval == -1 && errno == ENOSYS)				\
	  {								\
	    __y2038_linux_support = -1;					\
	    DO_CLOCK_GETTIME_32;					\
	  }								\
      }									\
    else								\
      {									\
        DO_CLOCK_GETTIME_32;						\
      }

#else

/* We are building without a 64-bit-time clock_gettime syscall */

# define DO_CLOCK_GETTIME_64 DO_CLOCK_GETTIME_32

#endif

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETTIME \
  SYSDEP_GETTIME_CPUTIME;						      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    DO_CLOCK_GETTIME_64;						      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME	1

#define SYSDEP_GETTIME_CPU(clock_id, tp) \
  DO_CLOCK_GETTIME_64; \
  break

#define SYSDEP_GETTIME_CPUTIME \
  struct timespec ts32

#include <sysdeps/unix/clock_gettime.c>
