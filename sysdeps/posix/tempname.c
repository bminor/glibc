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

/* Return nonzero if DIR is an existent directory.  */
static int
DEFUN(diraccess, (dir), CONST char *dir)
{
  struct stat buf;
  return __stat (dir, &buf) == 0 && S_ISDIR (buf.st_mode);
}

/* Return nonzero if FILE exists.  */
static int
DEFUN(exists, (file), CONST char *file)
{
  /* We can stat the file even if we can't read its data.  */
  struct stat st;
  int save = errno;
  if (__stat (file, &st) == 0)
    return 1;
  else
    {
      /* We report that the file exists if stat failed for a reason other
	 than nonexistence.  In this case, it may or may not exist, and we
	 don't know; but reporting that it does exist will never cause any
	 trouble, while reporting that it doesn't exist when it does would
	 violate the interface of __stdio_gen_tempname.  */
      int exists = errno != ENOENT;
      errno = save;
      return exists;
    }
}


/* These are the characters used in temporary filenames.  */
static CONST char letters[] =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* Generate a temporary filename.
   If DIR_SEARCH is nonzero, DIR and PFX are used as
   described for tempnam.  If not, a temporary filename
   in P_tmpdir with no special prefix is generated.  If LENPTR
   is not NULL, *LENPTR is set the to length (including the
   terminating '\0') of the resultant filename, which is returned.
   This goes through a cyclic pattern of all possible filenames
   consisting of five decimal digits of the current pid and three
   of the characters in `letters'.  Data for tempnam and tmpnam
   is kept separate, but when tempnam is using P_tmpdir and no
   prefix (i.e, it is identical to tmpnam), the same data is used.
   Each potential filename is tested for an already-existing file of
   the same name, and no name of an existing file will be returned.
   When the cycle reaches its end (12345ZZZ), NULL is returned.  */
char *
DEFUN(__stdio_gen_tempname, (dir, pfx, dir_search, lenptr),
      CONST char *dir AND CONST char *pfx AND
      int dir_search AND size_t *lenptr)
{
  static CONST char tmpdir[] = P_tmpdir;
  static struct
    {
      char buf[3];
      char *s;
      size_t i;
    } infos[2], *info;
  static char buf[FILENAME_MAX];
  static pid_t oldpid = (pid_t) 0;
  pid_t pid = __getpid();
  register size_t len, plen, dlen;

  if (dir_search)
    {
      register CONST char *d = getenv("TMPDIR");
      if (d != NULL && !diraccess(d))
	d = NULL;
      if (d == NULL && dir != NULL && diraccess(dir))
	d = dir;
      if (d == NULL && diraccess(tmpdir))
	d = tmpdir;
      if (d == NULL && diraccess("/tmp"))
	d = "/tmp";
      if (d == NULL)
	{
	  errno = ENOENT;
	  return NULL;
	}
      dir = d;
    }
  else
    dir = tmpdir;

  dlen = strlen (dir);

  /* Remove trailing slashes from the directory name.  */
  while (dlen > 1 && dir[dlen - 1] == '/')
    --dlen;

  if (pfx != NULL && *pfx != '\0')
    {
      plen = strlen(pfx);
      if (plen > 5)
	plen = 5;
    }
  else
    plen = 0;

  if (dir != tmpdir && !strcmp(dir, tmpdir))
    dir = tmpdir;
  info = &infos[(plen == 0 && dir == tmpdir) ? 1 : 0];

  if (pid != oldpid)
    {
      oldpid = pid;
      infos[0].buf[0] = infos[0].buf[1] = infos[0].buf[2] = letters[0];
      infos[0].s = infos[0].buf;
      infos[0].i = 0;
      infos[1].buf[0] = infos[1].buf[1] = infos[1].buf[2] = letters[0];
      infos[1].s = infos[1].buf;
      infos[1].i = 0;
    }

  len = dlen + 1 + plen + 5 + 3;
  for (;;)
    {
      *info->s = letters[info->i];
      if (sizeof (buf) < len ||
	  sprintf (buf, "%.*s/%.*s%.5d%.3s",
		   (int) dlen, dir, (int) plen,
		   pfx, pid % 100000, info->buf) != (int) len)
	return NULL;

      /* Always return a unique string.  */
      ++info->i;

      if (!exists (buf))
	break;

      if (info->i > sizeof (letters) - 2)
	{
	  info->i = 0;
	  if (info->s == &info->buf[2])
	    {
	      errno = EEXIST;
	      return NULL;
	    }
	  ++info->s;
	}
    }

  if (lenptr != NULL)
    *lenptr = len + 1;
  return buf;
}
