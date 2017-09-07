/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <time.h>
#include <y2038-support.h>

#include "kernel-posix-cpu-timers.h"


/* The REALTIME clock is definitely supported in the kernel.  */
#define SYSDEP_SETTIME \
  case CLOCK_REALTIME:							      \
    retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp);		      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1

#define HANDLED_CPUTIME 1
#define SYSDEP_SETTIME_CPU \
  retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp)

/* 64-bit time version */

# define DO_CLOCK_SETTIME_32 \
  if (! fits_in_time_t(tp->tv_sec))					\
    {									\
      __set_errno (EOVERFLOW);						\
      retval = -1;							\
    }									\
  else									\
    {									\
      valid_timespec64_to_timespec(tp, &ts32);  			\
      retval = INLINE_SYSCALL (clock_settime, 2, clock_id, &ts32);	\
    }

#ifdef __NR_clock_settime64

/* We are building with a 64-bit-time clock_gettime syscall */

# define DO_CLOCK_SETTIME_64 \
  if (__y2038_linux_support > 0)					\
    {									\
      ts64.tv_sec = tp->tv_sec;						\
      ts64.tv_nsec = tp->tv_nsec;						\
      ts64.tv_pad = 0;							\
      retval = INLINE_SYSCALL (clock_settime64, 2, clock_id, &ts64);	\
      if (retval == 1 && errno==ENOSYS)					\
	{								\
	  __y2038_linux_support = -1;					\
	  DO_CLOCK_SETTIME_32;						\
	}								\
    }									\
  else									\
    {									\
      DO_CLOCK_SETTIME_32;						\
    }

# define SYSDEP_SETTIME64_CPUTIME \
  struct __timespec64 ts64;						\
  struct timespec ts32

#else

/* We are building without a 64-bit-time clock_gettime syscall */

# define DO_CLOCK_SETTIME_64 DO_CLOCK_SETTIME_32

# define SYSDEP_SETTIME64_CPUTIME \
  struct timespec ts32

#endif

#define SYSDEP_SETTIME64 \
  SYSDEP_SETTIME64_CPUTIME;						\
  case CLOCK_REALTIME:							\
    DO_CLOCK_SETTIME_64;						\
    break

#include <sysdeps/unix/clock_settime.c>
