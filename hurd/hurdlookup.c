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
      errno = __hurd_errno (err);
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
  size_t retrynamelen;
  file_t newpt;
  size_t pathlen;
  int dealloc_dir;
  int nloops;

  dealloc_dir = 0;
  nloops = 0;
  pathlen = strlen (path) + 1;
  
  for (;;)
    {
      err = __dir_pathtrans (startdir, path, pathlen, flags, mode,
			     &doretry, retryname, &retrynamelen, &result);

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
	    return POSIX_ELOOP;

	  if (*retryname == '/')
	    {
	      startdir = _hurd_crdir;
	      dealloc_dir = 0;
	      path = retryname;
	      pathlen = retrynamelen;
	      while (*path == '/')
		{
		  ++path;
		  --pathlen;
		}
	    }
	  else
	    {
	      startdir = *result;
	      dealloc_dir = 1;
	      path = retryname;
	      pathlen = retrynamelen;
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
	  *name = path;
	  __mach_port_mod_refs (__mach_task_self (), _hurd_crdir,
				MACH_PORT_RIGHT_SEND, 1);
	  return _hurd_crdir;
	}
      else
	{
	  char dirname[lastslash - path + 1];
	  memcpy (dirname, path, lastslash - path);
	  dirname[lastslath - path] = '\0';
	  *name = lastslash + 1;
	  return __hurd_path_lookup (dirname, 0, 0);
	}
    }
  else
    {
      *name = path;
      __mach_port_mod_refs (__mach_task_self (), _hurd_cwdir,
			    MACH_PORT_RIGHT_SEND, 1);
      return _hurd_cwdir;
    }
}
