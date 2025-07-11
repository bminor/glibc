/* Linux internal definitions for ioctl.  powerpc version.
   Copyright (C) 2021-2025 Free Software Foundation, Inc.
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

#include <termios.h>
#include <termios_internals.h>
#include <assert.h>

/* PowerPC quirk: on PowerPC only, ioctl() emulates the TCGETS/TCSETS*
   ioctls with tcgetattr/tcsetattr using the glibc struct termios.
   As struct termios2 is the same as the kernel struct termios on PowerPC,
   simply consider the kernel ones as the termios2 interface, even
   though the kernel doesn't call it that. */

#define GLIBC_TCGETS _IOR ('t', 19, struct termios)
#define GLIBC_TCSETS _IOW ('t', 20, struct termios)
#define GLIBC_TCSETSW _IOW ('t', 21, struct termios)
#define GLIBC_TCSETSF _IOW ('t', 22, struct termios)

static inline bool
__ioctl_arch (int *r, int fd, unsigned long request, void *arg)
{
  static_assert (GLIBC_TCGETS != KERNEL_TCGETS2,
                 "emulation not possible due to matching ioctl constants");
  switch (request)
    {
    case GLIBC_TCGETS:
      *r = __tcgetattr (fd, (struct termios *) arg);
      break;

    case GLIBC_TCSETS:
      *r = __tcsetattr (fd, TCSANOW, (struct termios *) arg);
      break;

    case GLIBC_TCSETSW:
      *r = __tcsetattr (fd, TCSADRAIN, (struct termios *) arg);
      break;

    case GLIBC_TCSETSF:
      *r = __tcsetattr (fd, TCSAFLUSH, (struct termios *) arg);
      break;

    default:
      return false;
    }
  return true;
}
