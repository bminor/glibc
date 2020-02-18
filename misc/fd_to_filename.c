/* Construct a pathname under /proc/self/fd (or /dev/fd for Hurd).
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <fd_to_filename.h>

#include <assert.h>
#include <string.h>

char *
__fd_to_filename (int descriptor, struct fd_to_filename *storage)
{
  assert (descriptor >= 0);

  char *p = mempcpy (storage->buffer, FD_TO_FILENAME_PREFIX,
                     strlen (FD_TO_FILENAME_PREFIX));

  for (int d = descriptor; p++, (d /= 10) != 0; )
    continue;
  *p = '\0';
  for (int d = descriptor; *--p = '0' + d % 10, (d /= 10) != 0; )
    continue;
  return storage->buffer;
}
