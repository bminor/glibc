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

/* PowerPC quirk: on PowerPC only, ioctl() emulates the TCGETS/TCSETS*
   ioctls with tcgetattr/tcsetattr using the glibc struct termios.
   As struct termios2 is the same as the kernel struct termios on PowerPC,
   simply consider the kernel ones as the termios2 interface, even
   though the kernel doesn't call it that. */

#define TCGETS2	 _IOR ('t', 19, struct termios2)
#define TCSETS2	 _IOW ('t', 20, struct termios2)
#define TCSETSW2 _IOW ('t', 21, struct termios2)
#define TCSETSF2 _IOW ('t', 22, struct termios2)

#include <linux/sockios.h>
