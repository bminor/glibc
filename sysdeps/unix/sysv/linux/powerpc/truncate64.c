/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>


#ifdef __NR_truncate64
/* The variable is shared between all wrappers around *truncate64 calls.  */
int __have_no_truncate64;

extern int __syscall_truncate64 (const char *path, off64_t length);


/* Truncate the file FD refers to to LENGTH bytes.  */
int
truncate64 (path, length)
     const char *path;
     off64_t length;
{
  if (! __have_no_truncate64)
    {
      int saved_errno = errno;
      int result = __syscall_truncate64 (path, length);

      if (result != -1 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __have_no_truncate64 = 1;
    }

  if ((off_t) length != length)
    {
      __set_errno (EINVAL);
      return -1;
    }
  return truncate (path, (off_t) length);
}

#else
/* Use the generic implementation.  */
# include <sysdeps/generic/truncate64.c>
#endif
