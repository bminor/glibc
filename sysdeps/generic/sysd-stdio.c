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

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Read N bytes into BUF from COOKIE.  */
int
DEFUN(__stdio_read, (cookie, buf, n),
      PTR cookie AND register char *buf AND register size_t n)
{
#if	defined (EINTR) && defined (EINTR_REPEAT)
  CONST int fd = *(int *) cookie;
  int save = errno;
  int nread;

 try:;
  errno = 0;
  nread = __read(fd, buf, (int) n);
  if (nread < 0)
    {
      if (errno == EINTR)
	goto try;
      return -1;
    }
  errno = save;
  return nread;

#else	/* No EINTR.  */
  return __read(*(int *) cookie, buf, (int) n);
#endif
}


/* Write N bytes from BUF to COOKIE.  */
int
DEFUN(__stdio_write, (cookie, buf, n),
      PTR cookie AND register CONST char *buf AND register size_t n)
{
  CONST int fd = *(int *) cookie;
  register size_t written = 0;

  while (n > 0)
    {
      int count = __write (fd, buf, (int) n);
      if (count > 0)
	{
	  buf += count;
	  written += count;
	  n -= count;
	}
      else if (count < 0
#if	defined (EINTR) && defined (EINTR_REPEAT)
	       && errno != EINTR
#endif
	       )
	/* Write error.  */
	return -1;
    }

  return (int) written;
}


/* Move COOKIE's file position *POS bytes, according to WHENCE.
   The new file position is stored in *POS.
   Returns zero if successful, nonzero if not.  */
int
DEFUN(__stdio_seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  off_t new;
  new = __lseek(*(int *) cookie, (off_t) *pos, whence);
  if (new < 0)
    return 1;
  *pos = (fpos_t) new;
  return 0;
}


/* Close COOKIE.  */
int
DEFUN(__stdio_close, (cookie), PTR cookie)
{
  return __close(*(int *) cookie);
}


/* Open the given file with the mode given in the __io_mode argument.  */
PTR
DEFUN(__stdio_open, (filename, m, fdptr),
      CONST char *filename AND __io_mode m AND int *fdptr)
{
  int mode;

  if (m.__read && m.__write)
    mode = O_RDWR;
  else
    mode = m.__read ? O_RDONLY : O_WRONLY;

  if (m.__append)
    mode |= O_APPEND;
  if (m.__exclusive)
    mode |= O_EXCL;
  if (m.__truncate)
    mode |= O_TRUNC;

  if (m.__create)
    *fdptr = __open(filename, mode | O_CREAT,
		    S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  else
    *fdptr = __open(filename, mode);

  return NULL;

}
