/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysdep.h>


CONST char *__assert_program_name;

/* This function, when passed a string containing an asserted
   expression, a filename, and a line number, prints a message
   on the standard error stream of the form:
   	a.c:10: foobar: Assertion `a == b' failed.
   It then aborts program execution via a call to `abort'.  */

__NORETURN int
DEFUN(__assert_fail, (assertion, file, line, function),
      CONST char *assertion AND
      CONST char *file AND unsigned int line AND CONST char *function)      
{
#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  /* Print the message.  */
  (void) fprintf (stderr, "%s%s%s:%u: %s%sAssertion `%s' failed.\n",
		  __assert_program_name ? __assert_program_name : "",
		  __assert_program_name ? ": " : "",
		  file, line,
		  function ? function : "", function ? ": " : "",
		  assertion);
  (void) fflush (stderr);

  abort ();

  /* This function never returns, so making it void would make sense,
     but returning something makes the assert macro easier to write.  */
  return 0;
}

#ifdef	HAVE_GNU_LD

#include <string.h>
#include <gnu-stabs.h>

static void
DEFUN(set_progname, (argc, argv, envp),
      int argc AND char **argv AND char **envp)
{
  char *p;

  if (argv && argv[0])
    {
      p = strrchr (argv[0], '/');
      if (p == NULL)
	__assert_program_name = argv[0];
      else
	__assert_program_name = p + 1;
    }

  (void) &set_progname;		/* Avoid "defined but not used" warning.  */
}

text_set_element (__libc_subinit, set_progname);

#endif
