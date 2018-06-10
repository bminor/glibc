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

/* If we do not assume that the kernel provides 64-bit-time syscalls,
 * then we need a dynamic way to remember whether it does or not.  */

#ifndef __ASSUME_KERNEL_Y2038_SUPPORT

/* Dynamically check whether actual kernel supports Y2038 or not.  */
extern bool __y2038_get_kernel_support (void);

/* Dynamically set whether actual kernel supports Y2038 or not.  */
extern void __y2038_set_kernel_support (bool support);

#endif
