/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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
#include <fcntl.h>

error_t
__hurd_path_lookup (file_t crdir, file_t cwdir,
		    const char *path, int flags, mode_t mode,
		    file_t *result)
{
  error_t err;
  file_t startdir;

  enum retry_type doretry;
  char retryname[1024];		/* XXX string_t LOSES! */
  file_t newpt;
  int dealloc_dir;
  int nloops;

  if (*path == '/')
    {
      startdir = crdir;
      while (*path == '/')
	++path;
    }
  else
    startdir = cwdir;

  dealloc_dir = 0;
  nloops = 0;
  
  for (;;)
    {
      err = __dir_pathtrans (startdir, (char *) path, flags, mode,
			     &doretry, retryname, result);

      if (dealloc_dir)
	__mach_port_deallocate (__mach_task_self (), startdir);
      if (err)
	return err;

      switch (doretry)
	{
	case FS_RETRY_NONE:
	  /* We got a successful translation.  Now apply any
	     open-time action flags we were passed.  */
	  if (flags & O_EXLOCK)
	    ;			/* XXX */
	  if (!err && (flags & O_SHLOCK))
	    ;			/* XXX */
	  if (!err && (flags & O_TRUNC))
	    err = __file_truncate (*result, 0);

	  if (err)
	    __mach_port_deallocate (__mach_task_self (), *result);
	  return err;
	  
	case FS_RETRY_REAUTH:
	  __io_reauthenticate (*result, _hurd_pid);
	  __USEPORT (AUTH, __auth_user_authenticate (port, *result, _hurd_pid,
						     &newpt));
	  __mach_port_deallocate (__mach_task_self (), *result);
	  *result = newpt;
	  /* Fall through.  */

	case FS_RETRY_NORMAL:
#ifdef SYMLINK_MAX
	  if (nloops++ >= SYMLINK_MAX)
	    return ELOOP;
#endif

	  if (retryname[0] == '/')
	    {
	      startdir = crdir;
	      dealloc_dir = 0;
	      path = retryname;
	      do
		++path;
	      while (*path == '/');
	    }
	  else
	    {
	      startdir = *result;
	      dealloc_dir = 1;
	      path = retryname;
	    }
	  break;

	/* case FS_RETRY_MAGICAL: XXX */
	}
    }
}

error_t
__hurd_path_split (file_t crdir, file_t cwdir,
		   const char *path,
		   file_t *dir, char **name)
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
	  *name = (char *) path + 1;
	  __mach_port_mod_refs (__mach_task_self (), MACH_PORT_RIGHT_SEND,
				crdir, +1);
	  *dir = crdir;
	  return 0;
	}
      else
	{
	  /* "/dir1/dir2/.../file".  */
	  char dirname[lastslash - path + 1];
	  memcpy (dirname, path, lastslash - path);
	  dirname[lastslash - path] = '\0';
	  *name = (char *) lastslash + 1;
	  return __hurd_path_lookup (crdir, cwdir, dirname, 0, 0, dir);
	}
    }
  else
    {
      /* "foobar" => cwdir + "foobar".  */
      *name = (char *) path;
      __mach_port_mod_refs (__mach_task_self (), MACH_PORT_RIGHT_SEND,
			    cwdir, 1);
      *dir = cwdir;
      return 0;
    }
}

file_t
__path_lookup (const char *path, int flags, mode_t mode)
{
  error_t err;
  file_t result, crdir, cwdir;
  int dealloc_crdir, dealloc_cwdir;

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir);
  cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir);

  err = __hurd_path_lookup (crdir, cwdir, path, flags, mode, &result);

  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);
  _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir, cwdir);

  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  else
    return result;
}

file_t
__path_split (const char *path, char **name)
{
  error_t err;
  file_t dir, crdir, cwdir;
  int dealloc_crdir, dealloc_cwdir;

  crdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir);
  cwdir = _hurd_port_get (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir);

  err = __hurd_path_split (crdir, cwdir, path, &dir, name);

  _hurd_port_free (&_hurd_ports[INIT_PORT_CRDIR], &dealloc_crdir, crdir);
  _hurd_port_free (&_hurd_ports[INIT_PORT_CWDIR], &dealloc_cwdir, cwdir);

  if (err)
    {
      errno = err;
      return MACH_PORT_NULL;
    }
  else
    return dir;
}
