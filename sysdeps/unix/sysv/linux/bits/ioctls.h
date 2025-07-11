/* Copyright (C) 1996-2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef _SYS_IOCTL_H
# error "Never use <bits/ioctls.h> directly; include <sys/ioctl.h> instead."
#endif

/* Use the definitions from the kernel header files.  */
#include <asm/ioctls.h>

/* The GNU C library has a different definition of struct termios,
   incompatible with what the ioctl interface expects.  The existence
   of the termios2 ioctls is considered an implementation detail.
   Undefine all related ioctl constants.  */
#undef TCGETS
#undef TCSETS
#undef TCSETSF
#undef TCSETSW
#undef TCGETS2
#undef TCSETS2
#undef TCSETSF2
#undef TCSETSW2


#include <linux/sockios.h>
