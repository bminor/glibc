/* Hurdish implementation of getrandom
   Copyright (C) 2016-2022 Free Software Foundation, Inc.
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

#include <sys/random.h>
#include <fcntl.h>
#include <unistd.h>
#include <not-cancel.h>

extern char *__trivfs_server_name __attribute__((weak));

/* Write up to LENGTH bytes of randomness starting at BUFFER.
   Return the number of bytes written, or -1 on error.  */
ssize_t
__getrandom (void *buffer, size_t length, unsigned int flags)
{
  const char *random_source = "/dev/urandom";
  int open_flags = O_RDONLY | O_CLOEXEC;
  size_t amount_read;
  int fd;

  if (&__trivfs_server_name && __trivfs_server_name
      && __trivfs_server_name[0] == 'r'
      && __trivfs_server_name[1] == 'a'
      && __trivfs_server_name[2] == 'n'
      && __trivfs_server_name[3] == 'd'
      && __trivfs_server_name[4] == 'o'
      && __trivfs_server_name[5] == 'm'
      && __trivfs_server_name[6] == '\0')
    /* We are random, don't try to read ourselves!  */
    return length;

  if (flags & GRND_RANDOM)
    random_source = "/dev/random";

  if (flags & GRND_NONBLOCK)
    open_flags |= O_NONBLOCK;

  fd = __open_nocancel(random_source, open_flags);
  if (fd == -1)
    return -1;

  amount_read = __read_nocancel(fd, buffer, length);
  __close_nocancel_nostatus(fd);
  return amount_read;
}

libc_hidden_def (__getrandom)
weak_alias (__getrandom, getrandom)
