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

/*
 *	POSIX Standard: 5.1.2 Directory Operations	<dirent.h>
 */

#ifndef	_DIRENT_H

#define	_DIRENT_H	1
#include <features.h>

#ifdef	__cplusplus
extern "C" {
#endif

#include <gnu/types.h>

#define	__need_size_t
#include <stddef.h>


/* Directory entry structure.  */
struct dirent
  {
    __ino_t d_fileno;	/* File serial number.  */
    size_t d_namlen;	/* Length of the file name.  */

    /* Only this member is in the POSIX standard.  */
    char d_name[1];	/* File name (actually longer).  */
  };

#if defined(__USE_BSD) || defined(__USE_MISC)
#define	d_ino		d_fileno	/* Backward compatibility.  */
#endif

/* Directory stream type.  */
typedef struct
  {
    int __fd;			/* File descriptor.  */

    char *__data;		/* Directory block.  */
    size_t __allocation;	/* Space allocated for the block.  */
    size_t __offset;		/* Current offset into the block.  */
    size_t __size;		/* Total valid data in the block.  */

    struct dirent __entry;	/* Returned by `readdir'.  */
  } DIR;


/* Open a directory stream on NAME.
   Return a DIR stream on the directory, or NULL if it could not be opened.  */
extern DIR *EXFUN(opendir, (CONST char *__name));

/* Close the directory stream DIRP.
   Return 0 if successful, -1 if not.  */
extern int EXFUN(closedir, (DIR *__dirp));

/* Read a directory entry from DIRP.
   Return a pointer to a `struct dirent' describing the entry,
   or NULL for EOF or error.  The storage returned may be overwritten
   by a later readdir call on the same DIR stream.  */
extern struct dirent *EXFUN(readdir, (DIR *__dirp));

/* Rewind DIRP to the beginning of the directory.  */
extern void EXFUN(rewinddir, (DIR *__dirp));

#if defined(__USE_BSD) || defined(__USE_MISC)

#ifndef	MAXNAMLEN
/* Get the definitions of the POSIX.1 limits.  */
#include <posix1_lim.h>

/* `MAXNAMLEN' is the BSD name for what POSIX calls `NAME_MAX'.  */
#ifdef	NAME_MAX
#define	MAXNAMLEN	NAME_MAX
#else
#define	MAXNAMLEN	255
#endif
#endif

#include <gnu/types.h>

/* Seek to position POS on DIRP.  */
extern void EXFUN(seekdir, (DIR *__dirp, __off_t __pos));

/* Return the current position of DIRP.  */
extern __off_t EXFUN(telldir, (DIR *__dirp));

/* Scan the directory DIR, calling SELECT on each directory entry.
   Entries for which SELECT returns nonzero are individually malloc'd,
   sorted using qsort with CMP, and collected in a malloc'd array in
   *NAMELIST.  Returns the number of entries selected, or -1 on error.  */
extern int EXFUN(scandir, (CONST char *__dir,
			   struct dirent ***__namelist,
			   int EXFUN((*__select), (struct dirent *));
			   int EXFUN((*__cmp), (CONST PTR, CONST PTR))));

/* Function to compare two `struct dirent's alphabetically.  */
extern int EXFUN(alphasort, (CONST PTR, CONST PTR));


/* Read directory entries from FD into BUF, reading at most NBYTES.
   Reading starts at offset *BASEP, and *BASEP is updated with the new
   position after reading.  Returns the number of bytes read; zero when at
   end of directory; or -1 for errors.  */
extern __ssize_t EXFUN(__getdirentries, (int __fd, char *__buf,
					 size_t __nbytes, __off_t *__basep));
extern __ssize_t EXFUN(getdirentries, (int __fd, char *__buf,
				       size_t __nbytes, __off_t *__basep));


#endif	/* Use BSD or misc.  */

#ifdef	__cplusplus
}
#endif

#endif	/* dirent.h  */
