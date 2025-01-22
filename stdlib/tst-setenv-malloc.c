/* Test using setenv with a malloc-allocated environ variable.
   Copyright (C) 2025 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

/* This test is not in the scope for POSIX or any other standard, but
   some applications assume that environ is a heap-allocated pointer
   after a call to setenv on an empty environment.  */

#include <stdlib.h>
#include <unistd.h>
#include <support/check.h>
#include <support/support.h>

static const char *original_path;
static char **save_environ;

static void
rewrite_environ (void)
{
  save_environ = environ;
  environ = xmalloc (sizeof (*environ));
  *environ = NULL;
  TEST_COMPARE (setenv ("A", "1", 1), 0);
  TEST_COMPARE (setenv ("B", "2", 1), 0);
  TEST_VERIFY (environ != save_environ);
  TEST_COMPARE_STRING (environ[0], "A=1");
  TEST_COMPARE_STRING (environ[1], "B=2");
  TEST_COMPARE_STRING (environ[2], NULL);
  TEST_COMPARE_STRING (getenv ("PATH"), NULL);
  free (environ);
  environ = save_environ;
  TEST_COMPARE_STRING (getenv ("PATH"), original_path);
}

static int
do_test (void)
{
  original_path = getenv ("PATH");
  rewrite_environ ();

  /* Test again after reallocated the environment due to an initial
     setenv call.  */
  TEST_COMPARE (setenv ("TST_SETENV_MALLOC", "1", 1), 0);
  TEST_VERIFY (environ != save_environ);
  rewrite_environ ();

  return 0;
}

#include <support/test-driver.c>
