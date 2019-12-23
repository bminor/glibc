/* Compatibility definitions for 'struct timeval' with 32-bit time_t.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _TV32_COMPAT_H
#define _TV32_COMPAT_H 1

#include <bits/types/time_t.h>

/* Structures containing 'struct timeval' with 32-bit time_t.  */
struct __itimerval32
{
  struct __timeval32 it_interval;
  struct __timeval32 it_value;
};

#endif /* tv32-compat.h */
