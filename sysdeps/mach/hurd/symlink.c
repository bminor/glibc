/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <hurd/paths.h>
#include <fcntl.h>

/* Make a link to FROM called TO.  */
int
DEFUN(__symlink, (from, to), CONST char *from AND CONST char *to)
{
  error_t err;
  file_t node;
  size_t to_write;

  node = __path_lookup (to, O_WRITE|O_CREAT|O_EXCL,
			0777 & _hurd_umask);
  if (node == MACH_PORT_NULL)
    return -1;

  to_write = strlen (from);

  while (to_write > 0)
    {
      mach_msg_type_number_t wrote;
      if (err = __io_write (node, from, to_write, -1, &wrote))
	break;
      to_write -= wrote;
      from += wrote;
    }

  if (!err)
    err = __file_set_translator (node, FS_TRANS_EXCL, 0,
				 _HURD_SYMLINK, sizeof (_HURD_SYMLINK),
				 MACH_PORT_NULL);

  /* XXX can leave half-finished file */

  __mach_port_deallocate (__mach_task_self (), node);

  if (err)
    return __hurd_fail (err);
  return 0;
}
