/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <hurd.h>

/* Create a device file named PATH, with permission and special bits MODE
   and device number DEV (which can be constructed from major and minor
   device numbers with the `makedev' macro above).  */
int
DEFUN(__mknod, (path, mode, dev),
      CONST char *path AND mode_t mode AND dev_t dev)
{
  const char *name;
  file_t node;
  error_t err;
  const char *translator;
  size_t n;

  if (S_ISCHR (mode))
    translator = _HURD_CHRDEV;
  else if (S_ISBLK (mode))
    translator = _HURD_BLKDEV;
  else if (S_ISFIFO (mode))
    translator = _HURD_FIFO;
  else
    {
      errno = EINVAL;
      return -1;
    }

  node = __path_lookup (path,
			FS_LOOKUP_CREATE|FS_LOOKUP_EXCL|FS_LOOKUP_WRITE,
			mode & 0777,
			&node);
  if (node == MACH_PORT_NULL)
    return -1;

  err = __file_set_translator (node,
			       FS_GOAWAY_DONT,
			       translator, strlen (translator) + 1,
			       MACH_PORT_NULL);

  if (!err)
    err = __io_write (node, &dev, sizeof (dev), &n);
  if (!err && n != sizeof (dev))
    err = EIO;

  if (err)
    /* XXX The node still exists.... */
    return __hurd_fail (err);

  return 0;
}
