/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <hurd/term.h>

char *__ttyname = NULL;

/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
char *
ttyname (int fd)
{
  static const char dev[] = "/dev";
  DIR *dirstream;
  struct dirent *d;
  error_t err;
  mach_port_t fd_cttyid;
  static char nodename[1024] = "";	/* XXX */
  
  /* Open FILENAME relative to DIR and see if its ctty ID port
     is the same as FD_CTTYID.  If so, deallocate that port and return 1.  */
  int try (file_t dir, const char *filename)
    {
      mach_port_t file, cttyid;
      if (__USEPORT (CRDIR, __hurd_path_lookup (port, dir, filename, 0, 0,
						&file)))
	return 0;		/* Can't open it.  */
      err = __term_getctty (file, &cttyid);
      __mach_port_deallocate (__mach_task_self (), file);
      if (err)
	return 0;		/* Not a terminal.  */
      /* We only need to know if CTTYID is the same port as FD_CTTYID,
	 so deallocating the reference can never hurt.  */
      __mach_port_deallocate (__mach_task_self (), cttyid);
      return cttyid == fd_cttyid;
    }

  /* Get the ctty ID port of the object we want to find.  */
  if (err = _HURD_DPORT_USE (fd,
			     (__term_get_nodename (port, nodename),
			      __term_getctty (port, &fd_cttyid))))
    return __hurd_fail (err), NULL;

  if (nodename[0] != '\0' && __USEPORT (CWDIR, try (port, nodename)))
    return nodename;

  dirstream = opendir (dev);
  if (dirstream == NULL)
    return NULL;

  while ((d = readdir (dirstream)) != NULL)
    if (try (dirstream->__port, d->d_name))
      {
	int save;
	if (__ttyname)
	  free (__ttyname);
	__ttyname = malloc (sizeof (dev) + 1 + d->d_namlen);
	if (__ttyname != NULL)
	  {
	    memcpy (__ttyname, dev, sizeof (dev) - 1);
	    __ttyname[sizeof (dev)] = '/';
	    memcpy (&__ttyname[sizeof (dev) + 1], d->d_name, d->d_namlen + 1);
	  }
	save = errno;
	(void) closedir (dirstream);
	errno = save;
	return __ttyname;
      }

  {
    int save = errno;
    (void) closedir (dirstream);
    errno = save;
    return NULL;
  }
}
