/* Copyright (C) 1999-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


static int
try_mkdir (const char *name, void *arg __attribute__ ((unused)))
{
  int result = __mkdir (name, S_IRUSR | S_IWUSR | S_IXUSR);
  if (result < 0 && errno == EEXIST)
    /* Nothing funny went wrong, it just already exists.  Keep looking.  */
    result = -2;
  return result;
}

/* Generate a unique temporary directory from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   The directory is created, mode 700, and its name is returned.
   (This function comes from OpenBSD.) */
char *
mkdtemp (char *template)
{
  if (__gen_tempname (template, 0, &try_mkdir, NULL))
    return NULL;
  else
    return template;
}
