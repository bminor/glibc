/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <grp.h>

/* This is the function that all the others are based on.
   The format of the group file is known only here.  */

/* Structure containing info kept by each __grpread caller.  */
typedef struct
  {
#define	NAME_SIZE	9
#define	PASSWD_SIZE	21
#define	MEMLIST_SIZE	1001
    char name[NAME_SIZE], passwd[PASSWD_SIZE], memlist[MEMLIST_SIZE];
    size_t max_members;
    char **members;
    struct group g;
  } grpread_info;


/* Return a chunk of memory containing a pre-initialized `grpread_info'.  */
PTR
DEFUN_VOID(__grpalloc)
{
  grpread_info *info = (PTR) malloc(sizeof(grpread_info));
  if (info == NULL)
    return NULL;

  info->max_members = 5;
  info->members = (char **) malloc(5 * sizeof(char *));
  if (info->members == NULL)
    {
      free((PTR) info);
      return NULL;
    }

  return info;
}

/* Read a group entry from STREAM, filling in G.  */
struct group *
DEFUN(__grpread, (stream, g), FILE *stream AND PTR CONST g)
{
  register grpread_info *CONST info = (grpread_info *) g;
  register size_t i;
  register char *s;

  /* Idiocy checks.  */
  if (stream == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  if (fscanf(stream, "%8[^:]:%20[^:]:%hd:%1000[^\n]\n",
	     info->name, info->passwd, &info->g.gr_gid, info->memlist) != 4)
    return NULL;

  /* Turn the comma-separated list into an array.  */
  i = 0;
  for (s = strtok(info->memlist, ","); s != NULL;
       s = strtok((char *) NULL, ","))
    {
      if (i == info->max_members - 2)
	{
	  info->max_members += 5;
	  info->members = (char **)
	    realloc((PTR) info->members, info->max_members * sizeof(char *));
	  if (info->members == NULL)
	    return NULL;
	}

      info->members[i++] = s;
    }
  info->members[i] = NULL;

  info->g.gr_name = info->name;
  info->g.gr_passwd = info->passwd;
  info->g.gr_mem = info->members;
  return &info->g;
}
