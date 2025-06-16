/* Copyright (C) 1991-2025 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <termios.h>
#include <kernel_termios.h>
#include <sys/ioctl.h>

/* Return 1 if FD is a terminal, 0 if not, without changing errno  */
int
__isatty_nostatus (int fd)
{
  struct __kernel_termios k_termios;
  return INTERNAL_SYSCALL_CALL (ioctl, fd, TCGETS, &k_termios) == 0;
}
