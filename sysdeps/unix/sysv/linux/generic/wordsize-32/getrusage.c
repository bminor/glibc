/* getrusage -- get the rusage struct.  Linux/tv32 version.
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

#include <sys/time.h>
#include <sys/resource.h>
#include <sysdep.h>
#include <tv32-compat.h>

int
__getrusage (int who, struct rusage *usage)
{
#if __TIMESIZE == 64
  struct __rusage32 usage32;
  if (INLINE_SYSCALL_CALL (getrusage, who, &usage32) == -1)
    return -1;

  rusage32_to_rusage64 (&usage32, usage);
  return 0;
#else
  return INLINE_SYSCALL_CALL (getrusage, who, usage);
#endif
}

strong_alias (__getrusage, getrusage)
