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
 *	POSIX Standard: 9.2.2 User Database Access	<pwd.h>
 */

#ifndef	_PWD_H

#define	_PWD_H	1
#include <features.h>

#include <gnu/types.h>


/* The passwd structure.  */
struct passwd
  {
    char *pw_name;	/* Username.  */
    char *pw_passwd;	/* Password.  */
    __uid_t pw_uid;	/* User ID.  */
    __gid_t pw_gid;	/* Group ID.  */
    char *pw_gecos;	/* Real name.  */
    char *pw_dir;	/* Home directory.  */
    char *pw_shell;	/* Shell program.  */
  };


#if defined(__USE_SVID) || defined(__USE_GNU)
#define	__need_FILE
#include <stdio.h>
#endif

#ifdef	__USE_GNU
/* Return a new stream open on the password file.  */
extern FILE *EXFUN(__pwdopen, (NOARGS));

/* Read a password entry from STREAM, filling in P.
   Return the `struct passwd' of P if successful, NULL on failure.  */
extern struct passwd *EXFUN(__pwdread, (FILE *__stream, PTR __p));

/* Return a chunk of memory containing pre-initialized data for __pwdread.  */
extern PTR EXFUN(__pwdalloc, (NOARGS));
#endif


#if defined(__USE_SVID) || defined(__USE_MISC)
/* Rewind the password-file stream.  */
extern void EXFUN(setpwent, (NOARGS));

/* Close the password-file stream.  */
extern void EXFUN(endpwent, (NOARGS));

/* Read an entry from the password-file stream, opening it if necessary.  */
extern struct passwd *EXFUN(getpwent, (NOARGS));
#endif

#ifdef	__USE_SVID
/* Read an entry from STREAM.  */
extern struct passwd *EXFUN(fgetpwent, (FILE *__stream));

/* Write the given entry onto the given stream.  */
extern int EXFUN(putpwent, (CONST struct passwd *__p, FILE *__f));
#endif

/* Search for an entry with a matching user ID.
   This takes an `int' because that is what `uid_t's get widened to.  */
extern struct passwd *EXFUN(getpwuid, (__uid_t __uid));

/* Search for an entry with a matching username.  */
extern struct passwd *EXFUN(getpwnam, (CONST char *__name));


#endif	/* pwd.h  */
