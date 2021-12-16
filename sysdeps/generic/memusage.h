/* Copyright (C) 2000-2021 Free Software Foundation, Inc.
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


#include <limits.h>
#include <stdint.h>

#ifndef GETSP
# warning "GETSP is not defined for this architecture."
# define GETSP 0
#endif

#ifndef GETTIME
# define GETTIME(low,high)						   \
  {									   \
    struct __timespec64 now;						   \
    uint64_t usecs;							   \
    __clock_gettime64 (CLOCK_REALTIME, &now);				   \
    usecs = (uint64_t)now.tv_nsec / 1000 + (uint64_t)now.tv_sec * 1000000; \
    low = usecs & 0xffffffff;						   \
    high = usecs >> 32;							   \
  }
#endif
