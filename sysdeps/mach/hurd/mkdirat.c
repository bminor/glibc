/* Create a directory named relative to another open directory.  Hurd version.
   Copyright (C) 1991-2025 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <string.h>

int
__mkdirat (int fd, const char *path, mode_t mode)
{
  error_t err;
  const char *name;
  file_t parent;
  if (!strcmp (path, "/"))
    return __hurd_fail (EEXIST);
  parent = __directory_name_split_at (fd, path, (char **) &name);
  if (parent == MACH_PORT_NULL)
    return -1;
  err = __dir_mkdir (parent, name, mode & ~_hurd_umask);
  __mach_port_deallocate (__mach_task_self (), parent);
  if (err)
    return __hurd_fail (err);
  return 0;
}

weak_alias (__mkdirat, mkdirat)
