/* Linux times.  X32 version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

/* Inline Linux times system call.  */
#define INTERNAL_SYSCALL_TIMES(err, buf)				\
  ({									\
    unsigned long long int resultvar;					\
    LOAD_ARGS_1 (buf)							\
    LOAD_REGS_1								\
    asm volatile (							\
    "syscall\n\t"							\
    : "=a" (resultvar)							\
    : "0" (__NR_times) ASM_ARGS_1 : "memory", "cc", "r11", "cx");	\
    (long long int) resultvar; })

#include <sysdeps/unix/sysv/linux/times.c>
