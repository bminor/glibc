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
DEFUN(__symlink, (from, to), CONST char *from AND CONST char *to)
{
  error_t err;
  file_t todir;
  const char *toname;
  file_t node;

  todir = __hurd_path_split (to, &toname);
  if (todir == MACH_PORT_NULL)
    return -1;
  err = __dir_lookup (todir, toname,
		      FS_LOOKUP_CREATE|FS_LOOKUP_WRITE|FS_LOOKUP_EXCL,
		      0777 & umask,
		      &file);
  if (!err)
    {
      size_t to_write = strlen (from);
      while (to_write > 0)
	{
	  size_t wrote;
	  if (err = __io_write (file, from, strlen (from), -1, &wrote))
	    break;
	  to_write -= wrote;
	  from += wrote;
	}
      __mach_port_deallocate (__mach_task_self (), file);
    }

  if (!err)
    err = __dir_set_translator (todir, toname, FS_TRANS_EXCL, FS_GOAWAY_DONT,
				_HURD_SYMLINK,
				MACH_PORT_NULL);
  __mach_port_deallocate (__mach_task_self (), todir);

  if (err)
    return __hurd_fail (err);
  return 0;
}
