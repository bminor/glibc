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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


/* Get the pathname of the current working directory,
   and put it in SIZE bytes of BUF.  Returns NULL if the
   directory couldn't be determined or SIZE was too small.
   If successful, returns BUF.  In GNU, if BUF is NULL,
   an array is allocated with `malloc'; the array is SIZE
   bytes long, unless SIZE <= 0, in which case it is as
   big as necessary.  */
char *
DEFUN(getcwd, (buf, size), char *buf AND size_t size)
{
  static CONST char dots[]
    = "../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../../\
../../../../../../../../../../../../../../../../../../../../../../../../../..";
  CONST char *dotp, *dotlist;
  size_t dotsize;
  dev_t rootdev, thisdev;
  ino_t rootino, thisino;
  char *path;
  register char *pathp;
  struct stat st;

  if (size == 0)
    {
      if (buf != NULL)
	{
	  errno = EINVAL;
	  return NULL;
	}

#ifndef PATH_MAX
#define	PATH_MAX 1024
#endif
      size = PATH_MAX + 1;
    }

  if (buf != NULL)
    path = buf;
  else
    {
      path = malloc (size);
      if (path == NULL)
	return NULL;
    }

  pathp = path + size;
  *--pathp = '\0';

  if (stat (".", &st) < 0)
    return NULL;
  thisdev = st.st_dev;
  thisino = st.st_ino;

  if (stat ("/", &st) < 0)
    return NULL;
  rootdev = st.st_dev;
  rootino = st.st_ino;

  dotsize = sizeof (dots) - 1;
  dotp = &dots[sizeof (dots)];
  dotlist = dots;
  while (!(thisdev == rootdev && thisino == rootino))
    {
      register DIR *dirstream;
      register struct dirent *d;
      dev_t dotdev;
      ino_t dotino;
      char mount_point;

      /* Look at the parent directory.  */
      if (dotp == dotlist)
	{
	  /* My, what a deep directory tree you have, Grandma.  */
	  char *new;
	  if (dotlist == dots)
	    {
	      new = malloc (dotsize * 2 + 1);
	      if (new == NULL)
		return NULL;
	      memcpy (new, dots, dotsize);
	    }
	  else
	    {
	      new = realloc ((PTR) dotlist, dotsize * 2 + 1);
	      if (new == NULL)
		goto lose;
	    }
	  memcpy (&new[dotsize], new, dotsize);
	  dotp = &new[dotsize];
	  dotsize *= 2;
	  new[dotsize] = '\0';
	  dotlist = new;
	}

      dotp -= 3;

      /* Figure out if this directory is a mount point.  */
      if (stat (dotp, &st) < 0)
	goto lose;
      dotdev = st.st_dev;
      dotino = st.st_ino;
      mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      dirstream = opendir (dotp);
      if (dirstream == NULL)
	goto lose;
      while ((d = readdir (dirstream)) != NULL)
	{
	  if (d->d_name[0] == '.' &&
	      (d->d_namlen == 1 || (d->d_namlen == 2 && d->d_name[1] == '.')))
	    continue;
	  if (mount_point || d->d_fileno == thisino)
	    {
	      char *name = __alloca (dotlist + dotsize - dotp +
				     1 + d->d_namlen + 1);
	      memcpy (name, dotp, dotlist + dotsize - dotp);
	      name[dotlist + dotsize - dotp] = '/';
	      memcpy (&name[dotlist + dotsize - dotp + 1],
		      d->d_name, d->d_namlen + 1);
	      if (stat (name, &st) < 0)
		{
		  int save = errno;
		  (void) closedir (dirstream);
		  errno = save;
		  goto lose;
		}
	      if (st.st_dev == thisdev && st.st_ino == thisino)
		break;
	    }
	}
      if (d == NULL)
	{
	  int save = errno;
	  (void) closedir (dirstream);
	  errno = save;
	  goto lose;
	}
      else
	{
	  if (pathp - path < d->d_namlen + 1)
	    {
	      if (buf != NULL)
		{
		  errno = ERANGE;
		  return NULL;
		}
	      else
		{
		  size *= 2;
		  buf = realloc (path, size);
		  if (buf == NULL)
		    {
		      (void) closedir (dirstream);
		      free (path);
		      errno = ENOMEM; /* closedir might have changed it.  */
		      return NULL;
		    }
		  pathp = &buf[pathp - path];
		  path = buf;
		}
	    }
	  pathp -= d->d_namlen;
	  (void) memcpy (pathp, d->d_name, d->d_namlen);
	  *--pathp = '/';
	  (void) closedir (dirstream);
	}

      thisdev = dotdev;
      thisino = dotino;
    }

  if (pathp == &path[size - 1])
    *--pathp = '/';

  if (dotlist != dots)
    free ((PTR) dotlist);

  return memmove (path, pathp, path + size - pathp);

 lose:
  if (dotlist != dots)
    free ((PTR) dotlist);
  return NULL;
}
