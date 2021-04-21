/* Smoke testing GDB process attach with thread-local variable access.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

/* This test runs GDB against a forked copy of itself, to check
   whether libthread_db can be loaded, and that access to thread-local
   variables works.  */

#include <errno.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/test-driver.h>
#include <support/xstdio.h>
#include <support/xthread.h>
#include <support/xunistd.h>
#include <unistd.h>

/* Starts out as zero, changed to 1 or 2 by the debugger, depending on
   the thread.  */
__thread volatile int altered_by_debugger;

/* Writes the GDB script to run the test to PATH.  */
static void
write_gdbscript (const char *path, int tested_pid)
{
  FILE *fp = xfopen (path, "w");
  fprintf (fp,
           "set trace-commands on\n"
           "set debug libthread-db 1\n"
#if DO_ADD_SYMBOL_FILE
           /* Do not do this unconditionally to work around a GDB
              assertion failure: ../../gdb/symtab.c:6404:
              internal-error: CORE_ADDR get_msymbol_address(objfile*,
              const minimal_symbol*): Assertion `(objf->flags &
              OBJF_MAINLINE) == 0' failed.  */
           "add-symbol-file %1$s/nptl/tst-pthread-gdb-attach\n"
#endif
           "set auto-load safe-path %1$s/nptl_db\n"
           "set libthread-db-search-path %1$s/nptl_db\n"
           "attach %2$d\n",
           support_objdir_root, tested_pid);
  fputs ("break debugger_inspection_point\n"
         "continue\n"
         "thread 1\n"
         "print altered_by_debugger\n"
         "print altered_by_debugger = 1\n"
         "thread 2\n"
         "print altered_by_debugger\n"
         "print altered_by_debugger = 2\n"
         "continue\n",
         fp);
  xfclose (fp);
}

/* The test sets a breakpoint on this function and alters the
   altered_by_debugger thread-local variable.  */
void __attribute__ ((weak))
debugger_inspection_point (void)
{
}

/* Thread function for the test thread in the subprocess.  */
static void *
subprocess_thread (void *closure)
{
  /* Wait until altered_by_debugger changes the value away from 0.  */
  while (altered_by_debugger == 0)
    {
      usleep (100 * 1000);
      debugger_inspection_point ();
    }

  TEST_COMPARE (altered_by_debugger, 2);
  return NULL;
}

/* This function implements the subprocess under test.  It creates a
   second thread, waiting for its value to change to 2, and checks
   that the main thread also changed its value to 1.  */
static void
in_subprocess (void)
{
  pthread_t thr = xpthread_create (NULL, subprocess_thread, NULL);
  TEST_VERIFY (xpthread_join (thr) == NULL);
  TEST_COMPARE (altered_by_debugger, 1);
  _exit (0);
}

static int
do_test (void)
{
  pid_t tested_pid = xfork ();
  if (tested_pid == 0)
    in_subprocess ();
  char *tested_pid_string = xasprintf ("%d", tested_pid);

  char *gdbscript;
  xclose (create_temp_file ("tst-pthread-gdb-attach-", &gdbscript));
  write_gdbscript (gdbscript, tested_pid);

  pid_t gdb_pid = xfork ();
  if (gdb_pid == 0)
    {
      clearenv ();
      xdup2 (STDOUT_FILENO, STDERR_FILENO);
      execlp ("gdb", "gdb", "-nx", "-batch", "-x", gdbscript, NULL);
      if (errno == ENOENT)
        _exit (EXIT_UNSUPPORTED);
      else
        _exit (1);
    }

  int status;
  TEST_COMPARE (xwaitpid (gdb_pid, &status, 0), gdb_pid);
  if (WIFEXITED (status) && WEXITSTATUS (status) == EXIT_UNSUPPORTED)
    /* gdb is not installed.  */
    return EXIT_UNSUPPORTED;
  TEST_COMPARE (status, 0);
  TEST_COMPARE (xwaitpid (tested_pid, &status, 0), tested_pid);
  TEST_COMPARE (status, 0);

  free (tested_pid_string);
  free (gdbscript);
  return 0;
}

#include <support/test-driver.c>
