/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <hurd.h>

/* Make a link to FROM called TO.  */
int
DEFUN(__link, (from, to), CONST char *from AND CONST char *to)
{
  error_t err;
  file_t oldfile, linknode, todir;
  CONST char *toname;

  if (err = __hurd_path_lookup (from, 0, 0, &oldfile))
    goto lose;
  err = __file_getlinknode (oldfile, &linknode);
  if (err == POSIX_EOPNOTSUPP)
    linknode = oldfile;
  else
    __mach_port_deallocate (__mach_task_self (), oldfile);
  if (err)
    goto lose;
  err = __hurd_path_split (to, &todir, &toname);
  if (!err)
    {
      err = __dir_link (linknode, todir, toname);
      __mach_port_deallocate (__mach_task_self (), todir);
    }
  __mach_port_deallocate (__mach_task_self (), linknode);

 lose:
  return __hurd_fail (err);
}
