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
 *	ANSI Standard: 4.11 STRING HANDLING	<string.h>
 */

#ifndef	_STRING_H

#define	_STRING_H	1
#include <features.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Get size_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_NULL
#include <stddef.h>


/* Copy N bytes of SRC to DEST.  */
extern PTR EXFUN(memcpy, (PTR __dest, CONST PTR __src, size_t __n));
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern PTR EXFUN(memmove, (PTR __dest, CONST PTR __src, size_t __n));

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied,
   or NULL if C was not found in the first N bytes of SRC.  */
extern PTR EXFUN(__memccpy, (PTR __dest, CONST PTR __src,
			     int __c, size_t __n));
#if defined (__USE_SVID) || defined (__USE_BSD)
extern PTR EXFUN(memccpy, (PTR __dest, CONST PTR __src,
			   int __c, size_t __n));
#ifdef	__OPTIMIZE__
#define	memccpy(dest, src, c, n) __memccpy((dest), (src), (c), (n))
#endif	/* Optimizing.  */
#endif	/* SVID.  */


/* Set N bytes of S to C.  */
extern PTR EXFUN(memset, (PTR __s, int __c, size_t __n));

/* Compare N bytes of S1 and S2.  */
extern int EXFUN(memcmp, (CONST PTR __s1, CONST PTR __s2, size_t __n));

/* Search N bytes of S for C.  */
extern PTR EXFUN(memchr, (CONST PTR __s, int __c, size_t __n));


/* Copy SRC to DEST.  */
extern char *EXFUN(strcpy, (char *__dest, CONST char *__src));
/* Copy no more than N characters of SRC to DEST.  */
extern char *EXFUN(strncpy, (char *__dest, CONST char *__src, size_t __n));

/* Append SRC onto DEST.  */
extern char *EXFUN(strcat, (char *__src, CONST char *__dest));
/* Append no more than N characters from SRC onto DEST.  */
extern char *EXFUN(strncat, (char *__dest, CONST char *__src, size_t __n));

/* Compare S1 and S2.  */
extern int  EXFUN(strcmp, (CONST char *__s1, CONST char *__s2));
/* Compare N characters of S1 and S2.  */
extern int EXFUN(strncmp, (CONST char *__s1, CONST char *__s2, size_t __n));

/* Compare the collated forms of S1 and S2.  */
extern int EXFUN(strcoll, (CONST char *__s1, CONST char *__s2));
/* Put a transformation of SRC into no more than N bytes of DEST.  */
extern size_t EXFUN(strxfrm, (char *__dest, CONST char *__src, size_t __n));

#if defined (__USE_SVID) || defined (__USE_BSD)
/* Duplicate S, returning an identical malloc'd string.  */
extern char *EXFUN(strdup, (CONST char *__s));
#endif

/* Find the first occurrence of C in S.  */
extern char *EXFUN(strchr, (CONST char *__s, int __c));
/* Find the last occurrence of C in S.  */
extern char *EXFUN(strrchr, (CONST char *__s, int __c));

/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
extern size_t EXFUN(strcspn, (CONST char *__s, CONST char *__reject));
/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
extern size_t EXFUN(strspn, (CONST char *__s, CONST char *__accept));
/* Find the first occurence in S of any character in ACCEPT.  */
extern char *EXFUN(strpbrk, (CONST char *__s, CONST char *__accept));
/* Find the first occurence of NEEDLE in HAYSTACK.  */
extern char *EXFUN(strstr, (CONST char *__haystack, CONST char *__needle));
/* Divide S into tokens separated by characters in DELIM.  */
extern char *EXFUN(strtok, (char *__s, CONST char *__delim));

#ifdef	__USE_GNU
/* Find the first occurence of NEEDLE in HAYSTACK.
   NEEDLE is NEEDLE_LEN bytes long;
   HAYSTACK is HAYSTACK_LEN bytes long.  */
extern PTR EXFUN(memmem, (PTR __needle, size_t __needle_len,
			  PTR __haystack, size_t __haystack_len));
#endif

/* Return the length of S.  */
extern size_t EXFUN(strlen, (CONST char *__s));

/* Return a string describing the meaning of the errno code in ERRNUM.  */
extern char *EXFUN(strerror, (int __errnum));

#ifdef	__USE_BSD
/* Find the first occurrence of C in S (same as strchr).  */
extern char *EXFUN(index, (CONST char *__s, int __c));

/* Find the last occurrence of C in S (same as strrchr).  */
extern char *EXFUN(rindex, (CONST char *__s, int __c));

/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void EXFUN(bcopy, (CONST PTR __src, PTR __dest, size_t __n));

/* Set N bytes of S to 0.  */
extern void EXFUN(bzero, (PTR __s, size_t __n));

/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int EXFUN(bcmp, (CONST PTR __s1, CONST PTR __s2, size_t __n));

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int EXFUN(ffs, (int __i));

#if	defined(__GNUC__) && defined(__OPTIMIZE__)
#define	ffs()	__builtin_ffs()
#endif	/* GCC and optimizing.  */

/* Compare S1 and S2, ignoring case.  */
extern int EXFUN(strcasecmp, (CONST char *__s1, CONST char *__s2));

/* Return the next DELIM-delimited token from *STRINGP,
   terminating it with a '\0', and update *STRINGP to point past it.  */
extern char *EXFUN(strsep, (char **__stringp, CONST char *__delim));
#endif

#ifdef	__USE_GNU
/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int EXFUN(strncasecmp, (CONST char *__s1, CONST char *__s2,
			       size_t __n));

/* Return a string describing the meaning of the signal number in SIG.  */
extern char *EXFUN(strsignal, (int __sig));

/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
extern char *EXFUN(stpcpy, (char *__dest, CONST char *__src));
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* string.h  */
