/* Copyright (C) 1992, 1993 Free Software Foundation, Inc.
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

#include <ctype.h>

/* Different systems have different names for the array.
   This order is important for some systems.  */
#ifdef	HAVE__CTYPE__
#define	TABLE	_ctype__
#endif
#if !defined(TABLE) && defined(HAVE__CTYPE_)
#define	TABLE	_ctype_
#endif
#if !defined(TABLE) && defined(HAVE___CTYPE_)
#define	TABLE	__ctype_
#endif
#if !defined(TABLE) && defined(HAVE___CTYPE)
#define	TABLE	__ctype
#endif

#if defined (__STDC__) && __STDC__
#define	STRINGIFY(arg)	#arg
#else
#define	STRINGIFY(arg)	"arg"
#endif

#define EVALLED_STRINGIFY(x)	STRINGIFY (x)

main ()
{
#ifdef TABLE

  int i;

  puts ("#include <ansidecl.h>");
  printf ("CONST char %s[] =\n  {\n", EVALLED_STRINGIFY (TABLE));

  for (i = -1; i < 256; ++i)
    printf ("    %d,\n", (int) ((TABLE+1)[i]));

  puts ("  };");

#else

  puts ("/* I don't know what the ctype table is called on this system.");
  puts ("   If there is a table, and you want the ctype glue to work,");
  puts ("   edit configure.in and glue-ctype.c in sysdeps/unix/common/");
  puts ("   to check for the right name.  */");

#endif

  exit (0);
}
