/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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
#include <sys/types.h>
#include <hurd.h>
#include <fcntl.h>

/* XXX ctty undone */


/* Read up to N chars into BUF from COOKIE.
   Return how many chars were read, 0 for EOF or -1 for error.  */
ssize_t
DEFUN(__stdio_read, (cookie, buf, n),
      PTR cookie AND register char *buf AND register size_t n)
{
  unsigned int nread;
  error_t err;
  char *bufp = buf;

  if (err = __io_read ((io_t) cookie, &bufp, &nread, -1, n))
    return __hurd_fail (err);

  if (bufp != buf)
    {
      memcpy (buf, bufp, nread);
      __vm_deallocate (__mach_task_self (),
		       (vm_address_t) bufp, (vm_size_t) nread);
    }

  return nread;
}

/* Write up to N chars from BUF to COOKIE.
   Return how many chars were written or -1 for error.  */
ssize_t
DEFUN(__stdio_write, (cookie, buf, n),
      PTR cookie AND register CONST char *buf AND register size_t n)
{
  unsigned int wrote;
  error_t err;

  if (err = __io_write ((io_t) cookie, buf, n, -1, &wrote))
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
  error_t error = __io_seek ((file_t) cookie, *pos, whence, pos);
  if (error)
    return __hurd_fail (error);
  return 0;
}

/* Close the file associated with COOKIE.
   Return 0 for success or -1 for failure.  */
int
DEFUN(__stdio_close, (cookie), PTR cookie)
{
  error_t error = __mach_port_deallocate (__mach_task_self (),
					  (mach_port_t) cookie);
  if (error)
    return __hurd_fail (error);
  return 0;
}


/* Open FILENAME with the mode in M.  */
int
DEFUN(__stdio_open, (filename, m, cookieptr),
      CONST char *filename AND __io_mode m AND PTR *cookieptr)
{
  int flags;
  file_t port;

  flags = 0;
  if (m.__read)
    flags |= O_READ;
  if (m.__write)
    flags |= O_WRITE;
  if (m.__append)
    flags |= O_APPEND;
  if (m.__create)
    flags |= O_CREAT;
  if (m.__truncate)
    flags |= O_TRUNC;
  if (m.__exclusive)
    flags |= O_EXCL;

  port = __path_lookup (filename, flags, 0666 & ~_hurd_umask);
  if (port == MACH_PORT_NULL)
    return -1;
  *cookieptr = (void *) port;
  return 0;
}


/* Write a message to the error output.
   Try hard to make it really get out.  */
void
DEFUN(__stdio_errmsg, (msg, len), CONST char *msg AND size_t len)
{
  io_t server;
  unsigned int wrote;

  server = __getdport (2);
  __io_write (server, msg, len, -1, &wrote);
  __mach_port_deallocate (__mach_task_self (), server);
}

/* Return the POSIX.1 file descriptor associated with COOKIE,
   or -1 for errors.  If COOKIE does not relate to any POSIX.1 file
   descriptor, this should return -1 with errno set to EOPNOTSUPP.  */
int
DEFUN(__stdio_fileno, (cookie), PTR cookie)
{
  errno = ENOSYS;
  return -1;
}

#ifdef notready
#define __stdio_read	__stdio_fd_read
#define __stdio_write	__stdio_fd_write
#define __stdio_seek	__stdio_fd_seek
#define __stdio_close	__stdio_fd_close
#define __stdio_open	__stdio_fd_open
#include <sysdeps/posix/sysd-stdio.c>

static const __io_functions fd_io_funcs =
  {
    __read: __stdio_fd_read,
    __write: __stdio_fd_write,
    __seek: __stdio_fd_seek,
    __close: __stdio_fd_close,
  };

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
#endif
