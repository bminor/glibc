/* Basic tests for pldd program.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <array_length.h>
#include <gnu/lib-names.h>

#include <support/subprocess.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>

static void
target_process (void *arg)
{
  pause ();
}

/* The test runs in a container because pldd does not support tracing
   a binary started by the loader iself (as with testrun.sh).  */

static bool
in_str_list (const char *libname, const char *const strlist[])
{
  for (const char *const *str = strlist; *str != NULL; str++)
    if (strcmp (libname, *str) == 0)
      return true;
  return false;
}

static int
do_test (void)
{
  /* Create a copy of current test to check with pldd.  */
  struct support_subprocess target = support_subprocess (target_process, NULL);

  /* Run 'pldd' on test subprocess.  */
  struct support_capture_subprocess pldd;
  {
    /* Three digits per byte plus null terminator.  */
    char pid[3 * sizeof (uint32_t) + 1];
    snprintf (pid, array_length (pid), "%d", target.pid);

    char *prog = xasprintf ("%s/pldd", support_bindir_prefix);

    pldd = support_capture_subprogram (prog,
      (char *const []) { (char *) prog, pid, NULL });

    support_capture_subprocess_check (&pldd, "pldd", 0, sc_allow_stdout);

    free (prog);
  }

  /* Check 'pldd' output.  The test is expected to be linked against only
     loader and libc.  */
  {
    pid_t pid;
    char buffer[512];
#define STRINPUT(size) "%" # size "s"

    FILE *out = fmemopen (pldd.out.buffer, pldd.out.length, "r");
    TEST_VERIFY (out != NULL);

    /* First line is in the form of <pid>: <full path of executable>  */
    TEST_COMPARE (fscanf (out, "%u: " STRINPUT (512), &pid, buffer), 2);

    TEST_COMPARE (pid, target.pid);
    TEST_COMPARE (strcmp (basename (buffer), "tst-pldd"), 0);

    /* It expects only one loader and libc loaded by the program.  */
    bool interpreter_found = false, libc_found = false;
    while (fgets (buffer, array_length (buffer), out) != NULL)
      {
	/* Ignore vDSO.  */
        if (buffer[0] != '/')
	  continue;

	/* Remove newline so baseline (buffer) can compare against the
	   LD_SO and LIBC_SO macros unmodified.  */
	if (buffer[strlen(buffer)-1] == '\n')
	  buffer[strlen(buffer)-1] = '\0';

	const char *libname = basename (buffer);

	/* It checks for default names in case of build configure with
	   --enable-hardcoded-path-in-tests (BZ #24506).  */
	if (in_str_list (libname,
			 (const char *const []) { "ld.so", LD_SO, NULL }))
	  {
	    TEST_COMPARE (interpreter_found, false);
	    interpreter_found = true;
	    continue;
	  }

	if (in_str_list (libname,
			 (const char *const []) { "libc.so", LIBC_SO, NULL }))
	  {
	    TEST_COMPARE (libc_found, false);
	    libc_found = true;
	    continue;
	  }
      }
    TEST_COMPARE (interpreter_found, true);
    TEST_COMPARE (libc_found, true);

    fclose (out);
  }

  support_capture_subprocess_free (&pldd);
  support_process_terminate (&target);

  return 0;
}

#include <support/test-driver.c>
