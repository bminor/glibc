/* y2038 Linux kernel support indication.
   Copyright (C) 2018 Free Software Foundation, Inc.

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

#ifndef __ASSUME_KERNEL_Y2038_SUPPORT

#include <stdbool.h>

/* Indicates Y2038 support.
 * Can be read directly from within libc linux-related files.
 * Can be written non-zero to indicate support or lack thereof.
 * Other libraries (e.g. librt) cannot access this variable and must
 * call __y2038_get_kernel_support() and __y2038_set_kernel_support().  */
extern bool __y2038_linux_support;

#endif

/* As a fallback, provide generic Y2038 support indication.  */
#include <misc/y2038-support.h>
