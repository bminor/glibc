/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* AIX requires this to be the first thing in the file.  */
#if defined (_AIX) && !defined (__GNUC__)
 #pragma alloca
#endif

#ifdef	SHELL
#include "config.h"
#endif

#include <glob.h>
#include <fnmatch.h>

#include <errno.h>
#include <sys/types.h>

#ifdef	STDC_HEADERS
#include <stddef.h>
#endif

#if !defined(__GNU_LIBRARY__) && !defined(STDC_HEADERS)
extern int errno;
#endif

#ifndef	NULL
#define	NULL	0
#endif

#if defined (USGr3) && !defined (DIRENT)
#define DIRENT
#endif /* USGr3 */
#if defined (Xenix) && !defined (SYSNDIR)
#define SYSNDIR
#endif /* Xenix */

#if defined (POSIX) || defined (DIRENT) || defined (__GNU_LIBRARY__)
#include <dirent.h>
#ifndef	__GNU_LIBRARY__
#define D_NAMLEN(d) strlen((d)->d_name)
#else
#define D_NAMLEN(d) ((d)->d_namlen)
#endif
#else /* not POSIX or DIRENT */
#define	dirent		direct
#define D_NAMLEN(d)	((d)->d_namlen)
#if defined (USG) && !defined (sgi)
#if defined (SYSNDIR)
#include <sys/ndir.h>
#else /* SYSNDIR */
#include "ndir.h"
#endif /* not SYSNDIR */
#else /* not USG */
#include <sys/dir.h>
#endif /* USG */
#endif /* POSIX or DIRENT or __GNU_LIBRARY__ */

#if defined (POSIX) && !defined (__GNU_LIBRARY__)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
#define REAL_DIR_ENTRY(dp) 1
#else
#define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
#endif /* POSIX */

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif

#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__) \
	 || defined (POSIX))
#include <stdlib.h>
#include <string.h>
#define	ANSI_STRING
#else	/* No standard headers.  */

#ifdef	USG

#include <string.h>
#ifdef	NEED_MEMORY_H
#include <memory.h>
#endif
#define	ANSI_STRING

#else	/* Not USG.  */

#ifdef	NeXT

#include <string.h>

#else	/* Not NeXT.  */

#include <strings.h>

#ifndef	bcmp
extern int bcmp ();
#endif
#ifndef	bzero
extern void bzero ();
#endif
#ifndef	bcopy
extern void bcopy ();
#endif

#endif	/* NeXT. */

#endif	/* USG.  */

extern char *malloc (), *realloc ();
extern void free ();
extern void qsort ();

#endif /* Standard headers.  */

#ifndef	ANSI_STRING
#define	memcpy(d, s, n)	bcopy((s), (d), (n))
#define	strrchr	rindex
/* memset is only used for zero here, but let's be paranoid.  */
#define	memset(s, better_be_zero, n) \
  ((void) ((better_be_zero) == 0 ? (bzero((s), (n)), 0) : (abort(), 0)))
#endif	/* Not ANSI_STRING.  */

#ifndef	HAVE_STRCOLL
#define	strcoll	strcmp
#endif


#ifndef	__GNU_LIBRARY__
#ifdef	__GNUC__
__inline
#endif
static char *
my_realloc (p, n)
     char *p;
     unsigned int n;
{
  if (p == NULL)
    return malloc (n);
  return realloc (p, n);
}
#define	realloc	my_realloc
#endif


#if	!defined(__alloca) && !defined(__GNU_LIBRARY__)

#ifdef	__GNUC__
#undef	alloca
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#if	defined (sparc) || defined (HAVE_ALLOCA_H)
#include <alloca.h>
#else	/* Not sparc or HAVE_ALLOCA_H.  */
#ifndef	_AIX
extern char *alloca ();
#endif	/* Not _AIX.  */
#endif	/* sparc or HAVE_ALLOCA_H.  */
#endif	/* GCC.  */

#define	__alloca	alloca

#endif

#if defined (__STDC__) && __STDC__
#define	__ptr_t	void *
#else
#define	__ptr_t	char *
#endif

#ifndef	STDC_HEADERS
#undef	size_t
#define	size_t	unsigned int
#endif

static int glob_pattern_p __P ((const char *pattern, int quote));
static int glob_in_dir __P ((const char *pattern, const char *directory,
			     int flags,
			     int (*errfunc) __P ((const char *, int)),
			     glob_t * pglob));
static int prefix_array __P ((const char *prefix, char **array, size_t n));
static int collated_compare __P ((const __ptr_t, const __ptr_t));

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   `errno' value from the failing call; if it returns non-zero
   `glob' returns GLOB_ABEND; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, `glob' returns zero.  */
     int
       glob (pattern, flags, errfunc, pglob)
     const char *pattern;
     int flags;
     int (*errfunc) __P ((const char *, int));
     glob_t *pglob;
{
  const char *filename;
  char *dirname;
  size_t dirlen;
  int status;
  int oldcount;

  if (pattern == NULL || pglob == NULL || (flags & ~__GLOB_FLAGS) != 0)
    {
      errno = EINVAL;
      return -1;
    }

  /* Find the filename.  */
  filename = strrchr (pattern, '/');
  if (filename == NULL)
    {
      filename = pattern;
      dirname = (char *) ".";
      dirlen = 0;
    }
  else if (filename == pattern)
    {
      /* "/pattern".  */
      dirname = (char *) "/";
      dirlen = 1;
      ++filename;
    }
  else
    {
      dirlen = filename - pattern;
      dirname = (char *) __alloca (dirlen + 1);
      memcpy (dirname, pattern, dirlen);
      dirname[dirlen] = '\0';
      ++filename;
    }

  if (filename[0] == '\0' && dirlen > 1)
    /* "pattern/".  Expand "pattern", appending slashes.  */
    {
      int ret = glob (dirname, flags | GLOB_MARK, errfunc, pglob);
      if (ret == 0)
	pglob->gl_flags = (pglob->gl_flags & ~GLOB_MARK) | (flags & GLOB_MARK);
      return ret;
    }

  if (!(flags & GLOB_APPEND))
    {
      pglob->gl_pathc = 0;
      pglob->gl_pathv = NULL;
    }

  oldcount = pglob->gl_pathc;

  if (glob_pattern_p (dirname, !(flags & GLOB_NOESCAPE)))
    {
      /* The directory name contains metacharacters, so we
	 have to glob for the directory, and then glob for
	 the pattern in each directory found.  */
      glob_t dirs;
      register int i;

      status = glob (dirname,
		     ((flags & (GLOB_ERR | GLOB_NOCHECK | GLOB_NOESCAPE)) |
		      GLOB_NOSORT),
		     errfunc, &dirs);
      if (status != 0)
	return status;

      /* We have successfully globbed the preceding directory name.
	 For each name we found, call glob_in_dir on it and FILENAME,
	 appending the results to PGLOB.  */
      for (i = 0; i < dirs.gl_pathc; ++i)
	{
	  int oldcount;

#ifdef	SHELL
	  {
	    /* Make globbing interruptible in the bash shell. */
	    extern int interrupt_state;

	    if (interrupt_state)
	      {
		globfree (&dirs);
		globfree (&files);
		return GLOB_ABEND;
	      }
	  }
#endif /* SHELL.  */

	  oldcount = pglob->gl_pathc;
	  status = glob_in_dir (filename, dirs.gl_pathv[i],
				(flags | GLOB_APPEND) & ~GLOB_NOCHECK,
				errfunc, pglob);
	  if (status == GLOB_NOMATCH)
	    /* No matches in this directory.  Try the next.  */
	    continue;

	  if (status != 0)
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      return status;
	    }

	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirs.gl_pathv[i],
			    &pglob->gl_pathv[oldcount],
			    pglob->gl_pathc - oldcount))
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      return GLOB_NOSPACE;
	    }
	}

      flags |= GLOB_MAGCHAR;

      if (pglob->gl_pathc == oldcount)
	/* No matches.  */
	if (flags & GLOB_NOCHECK)
	  {
	    const size_t len = strlen (pattern) + 1;
	    char *patcopy = (char *) malloc (len);
	    if (patcopy == NULL)
	      return GLOB_NOSPACE;
	    memcpy (patcopy, pattern, len);

	    pglob->gl_pathv
	      = (char **) realloc (pglob->gl_pathv,
				   (pglob->gl_pathc +
				    ((flags & GLOB_DOOFFS) ?
				     pglob->gl_offs : 0) +
				    1 + 1) *
				   sizeof (char *));
	    if (pglob->gl_pathv == NULL)
	      {
		free (patcopy);
		return GLOB_NOSPACE;
	      }

	    if (flags & GLOB_DOOFFS)
	      while (pglob->gl_pathc < pglob->gl_offs)
		pglob->gl_pathv[pglob->gl_pathc++] = NULL;

	    pglob->gl_pathv[pglob->gl_pathc++] = patcopy;
	    pglob->gl_flags = flags;
	  }
	else
	  return GLOB_NOMATCH;
    }
  else
    {
      status = glob_in_dir (filename, dirname, flags, errfunc, pglob);
      if (status != 0)
	return status;

      if (dirlen > 0)
	{
	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirname,
			    &pglob->gl_pathv[oldcount],
			    pglob->gl_pathc - oldcount))
	    {
	      globfree (pglob);
	      return GLOB_NOSPACE;
	    }
	}

      if (!(flags & GLOB_NOSORT))
	qsort ((__ptr_t) & pglob->gl_pathv[oldcount],
	       pglob->gl_pathc - oldcount,
	       sizeof (char *), collated_compare);
    }

  return 0;
}


/* Free storage allocated in PGLOB by a previous `glob' call.  */
void
globfree (pglob)
     register glob_t *pglob;
{
  if (pglob->gl_pathv != NULL)
    {
      register int i;
      for (i = 0; i < pglob->gl_pathc; ++i)
	if (pglob->gl_pathv[i] != NULL)
	  free ((__ptr_t) pglob->gl_pathv[i]);
      free ((__ptr_t) pglob->gl_pathv);
    }
}


/* Do a collated comparison of A and B.  */
static int
collated_compare (a, b)
     const __ptr_t a;
     const __ptr_t b;
{
  const char *const s1 = *(const char *const * const) a;
  const char *const s2 = *(const char *const * const) b;

  if (s1 == s2)
    return 0;
  if (s1 == NULL)
    return 1;
  if (s2 == NULL)
    return -1;
  return strcoll (s1, s2);
}


/* Prepend PREFIX to each of N members of ARRAY, replacing ARRAY's
   elements in place.  Return nonzero if out of memory, zero if successful.
   A slash is inserted between PREFIX and each elt of ARRAY.
   Each old element of ARRAY is freed.  */
static int
prefix_array (prefix, array, n)
     const char *prefix;
     char **array;
     const size_t n;
{
  register size_t i;
  const size_t prelen = strlen (prefix);

  for (i = 0; i < n; ++i)
    {
      const size_t eltlen = strlen (array[i]) + 1;
      char *new = (char *) malloc (prelen + 1 + eltlen);
      if (new == NULL)
	{
	  while (i > 0)
	    free ((__ptr_t) array[--i]);
	  return 1;
	}

      memcpy (new, prefix, prelen);
      new[prelen] = '/';
      memcpy (&new[prelen + 1], array[i], eltlen);
      free ((__ptr_t) array[i]);
      array[i] = new;
    }

  return 0;
}


/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.  */
static int
glob_pattern_p (pattern, quote)
     const char *pattern;
     const int quote;
{
  register const char *p;
  int open = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
	return 1;

      case '\\':
	if (quote)
	  ++p;
	break;

      case '[':
	open = 1;
	break;

      case ']':
	if (open)
	  return 1;
	break;
      }

  return 0;
}


/* Like `glob', but PATTERN is a final pathname component,
   and matches are searched for in DIRECTORY.
   The GLOB_NOSORT bit in FLAGS is ignored.  No sorting is ever done.
   The GLOB_APPEND flag is assumed to be set (always appends).  */
static int
glob_in_dir (pattern, directory, flags, errfunc, pglob)
     const char *pattern;
     const char *directory;
     int flags;
     int (*errfunc) __P ((const char *, int));
     glob_t *pglob;
{
  register DIR *stream;
  register struct dirent *d;

  struct globlink
  {
    struct globlink *next;
    char *name;
  };
  struct globlink *names = NULL;
  size_t nfound = 0;

  if (!glob_pattern_p (pattern, !(flags & GLOB_NOESCAPE)))
    {
      stream = NULL;
      flags |= GLOB_NOCHECK;
    }
  else
    {
      flags |= GLOB_MAGCHAR;
      stream = opendir (directory);
      if (stream == NULL)
	{
	  if ((errfunc != NULL && (*errfunc) (directory, errno)) ||
	      (flags & GLOB_ERR))
	    return GLOB_ABEND;
	}
      else
	while ((d = readdir (stream)) != NULL)
	  if (REAL_DIR_ENTRY (d) &&
	      fnmatch (pattern, d->d_name,
		       (!(flags & GLOB_PERIOD) ? FNM_PERIOD : 0) |
		       ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)) == 0)
	    {
	      struct globlink *new = (struct globlink *)
	      __alloca (sizeof (struct globlink));
	      size_t len = D_NAMLEN (d);
	      new->name = (char *) malloc (len +
					 ((flags & GLOB_MARK) ? 1 : 0) + 1);
	      if (new->name == NULL)
		goto memory_error;
	      memcpy ((__ptr_t) new->name, d->d_name, len);
	      if (flags & GLOB_MARK)
		new->name[len++] = '/';
	      new->name[len] = '\0';
	      new->next = names;
	      names = new;
	      ++nfound;
	    }
    }

  if (nfound == 0 && (flags & GLOB_NOCHECK))
    {
      size_t len = strlen (pattern);
      nfound = 1;
      names = (struct globlink *) __alloca (sizeof (struct globlink));
      names->next = NULL;
      names->name = (char *) malloc (len + ((flags & GLOB_MARK) ? 1 : 0) + 1);
      if (names->name == NULL)
	goto memory_error;
      memcpy (names->name, pattern, len);
      if (flags & GLOB_MARK)
	names->name[len++] = '/';
      names->name[len] = '\0';
    }

  pglob->gl_pathv
    = (char **) realloc (pglob->gl_pathv,
			 (pglob->gl_pathc +
			  ((flags & GLOB_DOOFFS) ? pglob->gl_offs : 0) +
			  nfound + 1) *
			 sizeof (char *));
  if (pglob->gl_pathv == NULL)
    goto memory_error;

  if (flags & GLOB_DOOFFS)
    while (pglob->gl_pathc < pglob->gl_offs)
      pglob->gl_pathv[pglob->gl_pathc++] = NULL;

  for (; names != NULL; names = names->next)
    pglob->gl_pathv[pglob->gl_pathc++] = names->name;
  pglob->gl_pathv[pglob->gl_pathc] = NULL;

  pglob->gl_flags = flags;

  if (stream != NULL)
    {
      int save = errno;
      (void) closedir (stream);
      errno = save;
    }
  return nfound == 0 ? GLOB_NOMATCH : 0;

memory_error:
  {
    int save = errno;
    (void) closedir (stream);
    errno = save;
  }
  while (names != NULL)
    {
      if (names->name != NULL)
	free ((__ptr_t) names->name);
      names = names->next;
    }
  return GLOB_NOSPACE;
}
