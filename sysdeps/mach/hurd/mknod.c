/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <hurd/paths.h>
#include <fcntl.h>

/* Create a device file named PATH, with permission and special bits MODE
   and device number DEV (which can be constructed from major and minor
   device numbers with the `makedev' macro above).  */
int
DEFUN(__mknod, (path, mode, dev),
      CONST char *path AND mode_t mode AND dev_t dev)
{
  file_t node;
  error_t err;
  const char *translator;
  unsigned int n;

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

  node = __path_lookup (path, O_WRITE|O_CREAT|O_EXCL, mode & 0777);
  if (node == MACH_PORT_NULL)
    return -1;

  err = __file_set_translator (node, FS_TRANS_EXCL, 0,
			       translator, strlen (translator) + 1,
			       MACH_PORT_NULL);

  if (!err)
    err = __io_write (node, (void *) &dev, sizeof (dev), -1, &n);
  if (!err && n != sizeof (dev))
    err = EIO;

  if (err)
    /* XXX The node still exists.... */
    return __hurd_fail (err);

  return 0;
}
