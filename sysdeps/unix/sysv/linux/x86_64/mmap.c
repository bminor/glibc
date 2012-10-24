/* Copyright (C) 2012 Free Software Foundation, Inc.
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
#include <sysdeps/unix/sysv/linux/x86_64/sysdep.h>

void *
__mmap (void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  void *p = INLINE_SYSCALL (mmap, 6, addr, len, prot, flags, fd, offset);
  if ((long) p == -1) return MAP_FAILED;
#ifdef __CHKP__
  return __bnd_set_ptr_bounds (p, len);
#else
  return p;
#endif
}

weak_alias (__mmap, mmap64)
weak_alias (__mmap, __mmap64)
weak_alias (__mmap, mmap)

void *
__mremap (void *old_address, size_t old_size, size_t new_size, int flags, ...)
{
  void *p = INLINE_SYSCALL (mremap, 4, old_address, old_size, new_size, flags);
  if ((long) p  == -1) return MAP_FAILED;
#ifdef __CHKP__
  return __bnd_set_ptr_bounds (p, new_size);
#else
  return p;
#endif
}

weak_alias (__mremap, mremap)
