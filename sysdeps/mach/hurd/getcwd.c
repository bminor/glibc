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

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <hurd.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


/* Get the pathname of the current working directory, and put it in SIZE
   bytes of BUF.  Returns NULL if the directory couldn't be determined or
   SIZE was too small.  If successful, returns BUF.  In GNU, if BUF is
   NULL, an array is allocated with `malloc'; the array is SIZE bytes long,
   unless SIZE <= 0, in which case it is as big as necessary.  */

char *
getcwd (char *buf, size_t size)
{
  error_t err;
  dev_t rootdev, thisdev;
  ino_t rootino, thisino;
  char *path;
  register char *pathp;
  struct stat st;
  file_t parent;
  char *dirbuf = NULL;
  unsigned int dirbufsize = 0;
  file_t crdir;
  int dealloc_crdir;

  inline void cleanup (void)
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);
      __mach_port_deallocate (__mach_task_self (), parent);

      if (dirbuf != NULL)
	__vm_deallocate (__mach_task_self (),
			 (vm_address_t) dirbuf, dirbufsize);
    }

      
  if (size == 0)
    {
      if (buf != NULL)
	{
	  errno = EINVAL;
	  return NULL;
	}

      size = FILENAME_MAX + 1;	/* Good starting guess.  */
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

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir);
  if (err = __io_stat (crdir, &st))
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);
      return __hurd_fail (err), NULL;
    }
  rootdev = st.st_dev;
  rootino = st.st_ino;

  if (err = __USEPORT (CWDIR, __mach_port_mod_refs (__mach_task_self (),
						    (parent = port),
						    MACH_PORT_TYPE_SEND,
						    1)))
    {
      _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);
      return __hurd_fail (err), NULL;
    }
  thisdev = st.st_dev;
  thisino = st.st_ino;

  while (!(thisdev == rootdev && thisino == rootino))
    {
      struct dirent *d;
      dev_t dotdev;
      ino_t dotino;
      int mount_point;
      file_t newp;
      char *dirdata;
      unsigned int dirdatasize;
      off_t dirpos;

      /* Look at the parent directory.  */
      if (err = __hurd_path_lookup (crdir, parent, "..", O_EXEC, 0, &newp))
	goto lose;
      __mach_port_deallocate (__mach_task_self (), parent);
      parent = newp;

      /* Figure out if this directory is a mount point.  */
      if (err = __io_stat (parent, &st))
	goto lose;
      dotdev = st.st_dev;
      dotino = st.st_ino;
      mount_point = dotdev != thisdev;

      /* Search for the last directory.  */
      dirpos = 0;
      dirdata = dirbuf;
      dirdatasize = dirbufsize;
      while (!(err = __dir_readdir (parent, &dirdata, &dirdatasize,
				    dirpos, &dirpos, st.st_blksize)) &&
	     dirdatasize != 0)	     
	{
	  unsigned int offset;

	  if (dirdata != dirbuf)
	    {
	      /* The data was passed out of line, so our old buffer is no
		 longer useful.  Deallocate the old buffer and reset our
		 information for the new buffer.  */
	      __vm_deallocate (__mach_task_self (),
			       (vm_address_t) dirbuf, dirbufsize);
	      dirbuf = dirdata;
	      dirbufsize = dirdatasize;
	    }

	  offset = 0;
	  while (offset < dirdatasize)
	    {
	      d = (struct dirent *) &dirdata[offset];
	      offset += d->d_reclen;

	      /* Ignore `.' and `..'.  */	
	      if (d->d_name[0] == '.' &&
		  (d->d_namlen == 1 ||
		   (d->d_namlen == 2 && d->d_name[1] == '.')))
		continue;

	      if (mount_point || d->d_ino == thisino)
		{
		  file_t try;
		  if (err = __hurd_path_lookup (crdir, parent, d->d_name,
						O_NOLINK, 0, &try))
		    goto lose;
		  err = __io_stat (try, &st);
		  __mach_port_deallocate (__mach_task_self (), try);
		  if (err)
		    goto lose;
		  if (st.st_dev == thisdev && st.st_ino == thisino)
		    break;
		}
	    }
	}

      if (err)
	goto lose;
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
		      free (path);
		      return NULL;
		    }
		  pathp = &buf[pathp - path];
		  path = buf;
		}
	    }
	  pathp -= d->d_namlen;
	  (void) memcpy (pathp, d->d_name, d->d_namlen);
	  *--pathp = '/';
	}

      thisdev = dotdev;
      thisino = dotino;
    }

  if (pathp == &path[size - 1])
    *--pathp = '/';

  memmove (path, pathp, path + size - pathp);
  cleanup ();
  return path;

 lose:
  cleanup ();
  return NULL;
}
