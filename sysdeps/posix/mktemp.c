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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.  */
char *
DEFUN(mktemp, (template), char *template)
{
  static CONST char letters[]
    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static pid_t oldpid = (pid_t) 0;
  pid_t pid = __getpid();
  static size_t count;
  size_t len;
  char c;

  len = strlen(template);
  if (len < 6 || strcmp(&template[len - 6], "XXXXXX"))
    {
      errno = EINVAL;
      return template;
    }

  if (pid != oldpid)
    count = 0;

  c = letters[count++];
  count %= sizeof(letters) - 1;

  if (sprintf(&template[len - 6], "%c%.5u",
	      c, (unsigned int) pid % 100000) != 6)
    return NULL;

  return template;
}
