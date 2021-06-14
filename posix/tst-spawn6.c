/* Check posix_spawn set controlling terminal extension.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <intprops.h>
#include <paths.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <sys/wait.h>

static int
handle_restart (const char *argv1)
{
  int fd = xopen (_PATH_TTY, O_RDONLY, 0600);

  /* If process group is not changed (POSIX_SPAWN_SETPGROUP), then check
     the creating process one, otherwise check against the process group
     itself.  */
  pid_t pgrp;
  if (strcmp (argv1, "setgrpr") != 0)
    TEST_COMPARE (sscanf (argv1, "%d", &pgrp), 1);
  else
    {
      pgrp = getpgrp ();
      /* Check if a new process group was actually created.  */
      pid_t ppid = getppid ();
      pid_t pgid = getpgid (ppid);
      TEST_VERIFY (pgid != pgrp);
    }

  TEST_COMPARE (tcgetpgrp (fd), pgrp);

  xclose (fd);
  return 0;
}

static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

static void
run_subprogram (int argc, char *argv[], const posix_spawnattr_t *attr,
		int exp_err)
{
  short int flags;
  TEST_COMPARE (posix_spawnattr_getflags (attr, &flags), 0);
  bool setpgrp = flags & POSIX_SPAWN_SETPGROUP;

  char *spargv[9];
  char pgrp[INT_STRLEN_BOUND (pid_t)];

  int i = 0;
  for (; i < argc - 1; i++)
    spargv[i] = argv[i + 1];
  spargv[i++] = (char *) "--direct";
  spargv[i++] = (char *) "--restart";
  if (setpgrp)
    spargv[i++] = (char *) "setgrpr";
  else
    {
      snprintf (pgrp, sizeof pgrp, "%d", getpgrp ());
      spargv[i++] = pgrp;
    }
  spargv[i] = NULL;
  TEST_VERIFY_EXIT (i < array_length (spargv));

  pid_t pid;
  TEST_COMPARE (posix_spawn (&pid, argv[1], NULL, attr, spargv, environ),
		exp_err);
  if (exp_err != 0)
    return;

  int status;
  TEST_COMPARE (xwaitpid (pid, &status, WUNTRACED), pid);
  TEST_VERIFY (WIFEXITED (status));
  TEST_VERIFY (!WIFSTOPPED (status));
  TEST_VERIFY (!WIFSIGNALED (status));
  TEST_COMPARE (WEXITSTATUS (status), 0);
}

static int
do_test (int argc, char *argv[])
{
  /* We must have either:
     - four parameters left if called initially:
       + path to ld.so         optional
       + "--library-path"      optional
       + the library path      optional
       + the application name
     - six parameters left if called through re-execution:
       + --setgrpr             optional
   */

  if (restart)
    return handle_restart (argv[1]);

  int tcfd = xopen (_PATH_TTY, O_RDONLY, 0600);

  /* Check getters and setters.  */
  {
    posix_spawnattr_t attr;
    TEST_COMPARE (posix_spawnattr_init (&attr), 0);
    TEST_COMPARE (posix_spawnattr_tcsetpgrp_np (&attr, tcfd), 0);

    int fd;
    TEST_COMPARE (posix_spawnattr_tcgetpgrp_np (&attr, &fd), 0);
    TEST_COMPARE (tcfd, fd);
  }

  /* Check setting the controlling terminal without changing the group.  */
  {
    posix_spawnattr_t attr;
    TEST_COMPARE (posix_spawnattr_init (&attr), 0);
    TEST_COMPARE (posix_spawnattr_setflags (&attr, POSIX_SPAWN_TCSETPGROUP),
		  0);
    TEST_COMPARE (posix_spawnattr_tcsetpgrp_np (&attr, tcfd), 0);

    run_subprogram (argc, argv, &attr, 0);
  }

  /* Check setting both the controlling terminal and the create a new process
     group.  */
  {
    posix_spawnattr_t attr;
    TEST_COMPARE (posix_spawnattr_init (&attr), 0);
    TEST_COMPARE (posix_spawnattr_setflags (&attr, POSIX_SPAWN_TCSETPGROUP
						   | POSIX_SPAWN_SETPGROUP),
		  0);
    TEST_COMPARE (posix_spawnattr_setpgroup (&attr, 0), 0);
    TEST_COMPARE (posix_spawnattr_tcsetpgrp_np (&attr, tcfd), 0);

    run_subprogram (argc, argv, &attr, 0);
  }

  /* Trying to set the controlling terminal after a setsid incurs in a ENOTTY
     from tcsetpgrp.  */
  {
    posix_spawnattr_t attr;
    TEST_COMPARE (posix_spawnattr_init (&attr), 0);
    TEST_COMPARE (posix_spawnattr_setflags (&attr, POSIX_SPAWN_TCSETPGROUP
						   | POSIX_SPAWN_SETSID), 0);
    TEST_COMPARE (posix_spawnattr_tcsetpgrp_np (&attr, tcfd), 0);

    run_subprogram (argc, argv, &attr, ENOTTY);
  }

  xclose (tcfd);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
