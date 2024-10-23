/* __thread_pointer definition.  loongarch version.
   Copyright (C) 2021-2024 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef _SYS_THREAD_POINTER_H
#define _SYS_THREAD_POINTER_H

#include <sys/cdefs.h>

static inline void *
__thread_pointer (void)
{
#if __glibc_has_builtin (__builtin_thread_pointer)
  return __builtin_thread_pointer ();
#else
  void *__thread_register;
  __asm__ ("move %0, $tp" : "=r" (__thread_register));
  return __thread_register;
#endif
}

#endif /* _SYS_THREAD_POINTER_H */
