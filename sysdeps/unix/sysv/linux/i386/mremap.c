/* Copyright (C) 2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sysdeps/unix/sysv/linux/i386/sysdep.h>

void *
__mremap (void *old_address, size_t old_size, size_t new_size, int flags, ...)
{
  void *p = INLINE_SYSCALL (mremap, 4, old_address, old_size, new_size, flags);
  if ((long) p == -1) return MAP_FAILED;
#ifdef __CHKP__
  return __bnd_set_ptr_bounds (p, new_size);
#else
  return p;
#endif
}

weak_alias (__mremap, mremap)
