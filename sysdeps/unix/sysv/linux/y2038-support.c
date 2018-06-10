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

/* By default the underlying Linux kernel is assumed not to support Y2038.
 * Any Linux architecture may (test and) claim Y2038 kernel support by
 * setting __y2038_linux_support.
 */
bool __y2038_linux_support = false;

/* For Linux, Y2038 kernel support is determined by __y2038_linux_support.  */

bool
__linux_y2038_get_kernel_support (void)
{
  return __y2038_linux_support;
}
strong_alias (__linux_y2038_get_kernel_support, __y2038_get_kernel_support)

void
__linux_y2038_set_kernel_support (bool support)
{
  __y2038_linux_support = support;
}
strong_alias (__linux_y2038_set_kernel_support, __y2038_set_kernel_support)

#endif
