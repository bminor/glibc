#if 1
#include <sysdeps/stub/sysd-stdio.c>
#else
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
#include <stdio.h>
#include <hurd.h>


/* XXX FILE * locking */


/* Read up to N chars into BUF from COOKIE.
   Return how many chars were read, 0 for EOF or -1 for error.  */
int
DEFUN(__stdio_read, (cookie, buf, n),
      PTR cookie AND register char *buf AND register size_t n)
{
  size_t nread;
  error_t err;
  char *bufp = buf;

  if (err = __io_read ((io_t) cookie, /* XXX ctty */,
		       &bufp, &nread, -1, n))
    return __hurd_fail (err);

  if (bufp != buf)
    {
      memcpy (buf, bufp, nread);
      __vm_deallocate (bufp, nread);
    }

  return nread;
}

/* Write up to N chars from BUF to COOKIE.
   Return how many chars were written or -1 for error.  */
int
DEFUN(__stdio_write, (cookie, buf, n),
      PTR cookie AND register CONST char *buf AND register size_t n)
{
  size_t wrote;
  error_t err;

  if (err = __io_write ((io_t) cookie, /* XXX ctty */,
			buf, n, -1, &wrote))
    return __hurd_fail (err);

  return wrote;
}

/* Move COOKIE's file position *POS bytes, according to WHENCE.
   The current file position is stored in *POS.
   Returns zero if successful, nonzero if not.  */
int
DEFUN(__stdio_seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  error_t error = __file_seek ((file_t) cookie, *pos, whence, pos);
  if (error)
    return __hurd_fail (error);
  return 0;
}

/* Close the file associated with COOKIE.
   Return 0 for success or -1 for failure.  */
int
DEFUN(__stdio_close, (cookie), PTR cookie)
{
  if (__mach_port_deallocate (__mach_task_self (), (mach_port_t) cookie))
    {
      errno = EINVAL;		/* ? */
      return -1;
    }
  return 0;
}


/* Open FILENAME with the mode in M.
   Return the magic cookie associated with the opened file
   or NULL which specifies that an integral descriptor may be
   found in *FDPTR.  This descriptor is negative for errors.  */
PTR
DEFUN(__stdio_open, (filename, m, fdptr),
      CONST char *filename AND __io_mode m AND int *fdptr)
{
  int error;
  int flags;
  file_t file;

  flags = 0;
  if (m.__read)
    flags |= FS_LOOKUP_READ;
  if (m.__write)
    flags |= FS_LOOKUP_WRITE;
  if (m.__append)
    flags |= FS_LOOKUP_APPEND;
  if (m.__create)
    flags |= FS_LOOKUP_CREATE;
  if (m.__truncate)
    flags |= FS_LOOKUP_TRUNCATE;
  if (m.__exclusive)
    flags |= FS_LOOKUP_EXCL;

  file = __path_lookup (filename, flags, 0666 & ~_hurd_umask);
  if (file == MACH_PORT_NULL)
    {
      *fdptr = -1;
      return NULL;
    }
  return (PTR) file;
}


/* Write a message to the error output.
   Try hard to make it really get out.  */
void
DEFUN(__stdio_errmsg, (msg, len), CONST char *msg AND size_t len)
{
  io_t server;
  size_t wrote;

  server = __getdport (2);
  __io_write (server, 0, 0, 0,	/* XXX ctty? */
	      buf, n, -1, &wrote);
  __mach_port_deallocate (__mach_task_self (), server);
}


/* Generate a (hopefully) unique temporary filename
   in DIR (if applicable), using prefix PFX.
   If DIR_SEARCH is nonzero, perform directory searching
   malarky as per the SVID for tempnam.
   Return the generated filename or NULL if one could not
   be generated, putting the length of the string in *LENPTR.  */
char *
DEFUN(__stdio_gen_tempname, (dir, pfx, dir_search, lenptr),
      CONST char *dir AND CONST char *pfx AND
      int dir_search AND size_t *lenptr)
{
  char *name;
  error_t err;
  static file_t tmpserv = MACH_PORT_NULL;

  if (tmpserv == MACH_PORT_NULL)
    {
      tmpserv = __path_lookup (_SERVERS_TMPFILE, 0, 0);
      if (tmpserv == MACH_PORT_NULL)
	return NULL;
    }

  if (err = __tmpfile_generate_name (tmpserv, dir, pfx, getenv ("TMPDIR"),
				     &name, lenptr))
    {
      errno = __hurd_errno (err);
      return NULL;
    }
  return name;
}

/* XXX _hurd_fork_hook to install stream ports.  */

static error_t
fork_stdio (task_t newtask)
{
  FILE *f;

  for (f = __stdio_head; f != NULL; f = f->next)
    if (f->__io_funcs.__close == __stdio_close &&
	(io_t) f->__cookie != MACH_PORT_NULL)
      {
	error_t err = __mach_port_insert_right (newtask,
						(io_t) f->__cookie,
						(io_t) f->__cookie,
						MACH_PORT_COPY_SEND);
	if (err)
	  return err;
      }
}
#endif /* 1 */
