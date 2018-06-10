/* y2038 general kernel support indication.
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

#include <stdbool.h>

#ifndef __ASSUME_KERNEL_Y2038_SUPPORT

/* By default glibc assumes the underlying kernel does not support Y2038.  */ 
bool
__default_y2038_get_kernel_support (void)
{
  return false;
}
weak_alias (__default_y2038_get_kernel_support, __y2038_get_kernel_support)

/* By default glibc just ignores Y2038 support indication setting.  */ 
void
__default_y2038_set_kernel_support (bool support __attribute__ ((unused)))
{
  /* Do nothing.  */
}
weak_alias (__default_y2038_set_kernel_support, __y2038_set_kernel_support)

#endif
