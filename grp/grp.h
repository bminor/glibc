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

/*
 *	POSIX Standard: 9.2.1 Group Database Access	<grp.h>
 */

#ifndef	_GRP_H

#define	_GRP_H	1
#include <features.h>

#include <gnu/types.h>


/* The group structure.	 */
struct group
  {
    char *gr_name;	/* Group name.	*/
    char *gr_passwd;	/* Password.	*/
    __gid_t gr_gid;	/* Group ID.	*/
    char **gr_mem;	/* Member list.	*/
  };


#if defined(__USE_SVID) || defined(__USE_GNU)
#define	__need_FILE
#include <stdio.h>
#endif

#ifdef	__USE_GNU
/* Return a new stream open on the group file.  */
extern FILE *EXFUN(__grpopen, (NOARGS));

/* Read a group entry from STREAM, filling in G.
   Return the `struct group' of G if successful, NULL on failure.  */
extern struct group *EXFUN(__grpread, (FILE *__stream, PTR __g));

/* Return a chunk of memory containing pre-initialized data for __grpread.  */
extern PTR EXFUN(__grpalloc, (NOARGS));
#endif


#if defined(__USE_SVID) || defined(__USE_MISC)
/* Rewind the group-file stream.  */
extern void EXFUN(setgrent, (NOARGS));

/* Close the group-file stream.  */
extern void EXFUN(endgrent, (NOARGS));

/* Read an entry from the group-file stream, opening it if necessary.  */
extern struct group *EXFUN(getgrent, (NOARGS));
#endif

#ifdef	__USE_SVID
/* Read a group entry from STREAM.  */
extern struct group *EXFUN(fgetgrent, (FILE *__stream));
#endif

/* Search for an entry with a matching group ID.
   This takes an `int' because that is what `gid_t's get widened to.  */
extern struct group *EXFUN(getgrgid, (__gid_t __gid));

/* Search for an entry with a matching group name.  */
extern struct group *EXFUN(getgrnam, (CONST char *__name));


#ifdef	__USE_BSD

#define	__need_size_t
#include <stddef.h>

/* Set the group set for the current user to GROUPS (N of them).  */
extern int EXFUN(setgroups, (size_t __n, CONST __gid_t *groups));

/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
extern int EXFUN(initgroups, (CONST char *user, __gid_t group));

#endif	/* Use BSD.  */


#endif	/* grp.h  */
