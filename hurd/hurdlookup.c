/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <string.h>
#include <limits.h>

file_t
__hurd_path_lookup (const char *path, int flags, mode_t mode)
{
  error_t err;
  file_t startdir, result;

  if (*path == '/')
    {
      startdir = _hurd_crdir;
      while (*path == '/')
	++path;
    }
  else
    startdir = _hurd_cwdir;

  err = __dir_lookup (startdir, path, flags, mode, &result);
  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  return result;
}

error_t
__dir_lookup (file_t startdir, const char *path, int flags, mode_t mode,
	      file_t *result)
{
  enum retry_type doretry;
  char retryname[PATH_MAX];
  file_t newpt;
  int dealloc_dir;
  int nloops;

  dealloc_dir = 0;
  nloops = 0;
  
  for (;;)
    {
      err = __dir_pathtrans (startdir, path, flags, mode,
			     &doretry, retryname, &result);

      if (dealloc_dir)
	__mach_port_deallocate (__mach_task_self (), startdir);
      if (err)
	return err;

      switch (doretry)
	{
	case FS_RETRY_NONE:
	  return POSIX_SUCCESS;
	  
	case FS_RETRY_REAUTH:
	  __io_reauthenticate (*result);
	  __auth_user_authenticate (_hurd_auth, result, &newpt);
	  __mach_port_deallocate (__mach_task_self (), *result);
	  *result = newpt;
	  /* Fall through.  */

	case FS_RETRY_NORMAL:
	  if (nloops++ >= MAXSYMLINKS)
	    return ELOOP;

	  if (retryname[0] == '/')
	    {
	      startdir = _hurd_crdir; /* XXX not modular--pass crdir arg? */
	      dealloc_dir = 0;
	      path = retryname;
	      while (*path == '/')
		++path;
	    }
	  else
	    {
	      startdir = *result;
	      dealloc_dir = 1;
	      path = retryname;
	    }
	}
    }
}

file_t
__hurd_path_split (const char *path, const char **name)
{
  const char *lastslash;
  
  /* Skip leading slashes in the pathname.  */
  if (*path == '/')
    {
      while (*path == '/')
	++path;
      --path;			/* Leave on one slash.  */
    }
  
  lastslash = strrchr (path, '/');
  
  if (lastslash != NULL)
    {
      if (lastslash == path)
	{
	  /* "/foobar" => crdir + "foobar".  */
	  *name = path;
	  return _hurd_getport (&_hurd_crdir, &_hurd_lock);
	}
      else
	{
	  /* "/dir1/dir2/.../file".  */
	  char dirname[lastslash - path + 1];
	  memcpy (dirname, path, lastslash - path);
	  dirname[lastslath - path] = '\0';
	  *name = lastslash + 1;
	  return __hurd_path_lookup (dirname, 0, 0);
	}
    }
  else
    {
      /* "foobar" => cwdir + "foobar".  */
      *name = path;
      return _hurd_getport (&_hurd_cwdir, &_hurd_lock);
    }
}
