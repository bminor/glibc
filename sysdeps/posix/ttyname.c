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
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


char *__ttyname = NULL;

/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
char *
DEFUN(ttyname, (fd), int fd)
{
  static char dev[] = "/dev";
  char name[NAME_MAX];
  char *CONST np = &name[sizeof(dev)];
  struct stat st;
  dev_t mydev;
  ino_t myino;
  DIR *dirstream;
  struct dirent *d;

  if (fstat(fd, &st) < 0)
    return NULL;
  if (!S_ISCHR(st.st_mode))
    {
      errno = EINVAL;
      return NULL;
    }
  mydev = st.st_dev;
  myino = st.st_ino;

  dirstream = opendir(dev);
  if (dirstream == NULL)
    return NULL;

  (void) memcpy(name, dev, sizeof(dev) - 1);
  np[-1] = '/';
  while ((d = readdir(dirstream)) != NULL)
    if (d->d_fileno == myino)
      {
	(void) memcpy(np, d->d_name, d->d_namlen + 1);
	if (stat(name, &st) == 0 && st.st_dev == mydev)
	  {
	    static char result[sizeof (dev) + NAME_MAX];
	    (void) memcpy(result, name, (np - name) + d->d_namlen + 1);
	    (void) closedir(dirstream);
	    __ttyname = result;
	    return result;
	  }
      }
  (void) closedir(dirstream);
  return NULL;
}
