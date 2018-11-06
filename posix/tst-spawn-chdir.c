/* Test the posix_spawn_file_actions_addchdir_np function.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <errno.h>
#include <fcntl.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xstdio.h>
#include <support/xunistd.h>
#include <unistd.h>

/* Reads the file at PATH, which must consist of exactly one line.
   Removes the line terminator at the end of the file.  */
static char *
read_one_line (const char *path)
{
  FILE *fp = xfopen (path, "r");
  char *buffer = NULL;
  size_t length = 0;
  ssize_t ret = getline (&buffer, &length, fp);
  if (ferror (fp))
    FAIL_EXIT1 ("getline: %m");
  if (ret < 1)
    FAIL_EXIT1 ("getline returned %zd", ret);
  if (fgetc (fp) != EOF)
    FAIL_EXIT1 ("trailing bytes in %s", path);
  if (ferror (fp))
    FAIL_EXIT1 ("fgetc: %m");
  xfclose (fp);
  if (buffer[ret - 1] != '\n')
    FAIL_EXIT1 ("missing line terminator in %s", path);
  buffer[ret - 1] = 0;
  return buffer;
}

/* Return the path to the "pwd" program.  */
const char *
get_pwd_program (void)
{
  const char *const paths[] = { "/bin/pwd", "/usr/bin/pwd" };
  for (size_t i = 0; i < array_length (paths); ++i)
    if (access (paths[i], X_OK) == 0)
      return paths[i];
  FAIL_EXIT1 ("cannot find pwd program");
}

static int
do_test (void)
{
  /* Directory for temporary file data.  Each subtest uses a numeric
     subdirectory.  */
  char *directory = support_create_temp_directory ("tst-spawn-chdir-");
  {
    /* Avoid symbolic links, to get more consistent behavior from the
       pwd command.  */
    char *tmp = realpath (directory, NULL);
    if (tmp == NULL)
      FAIL_EXIT1 ("realpath: %m");
    free (directory);
    directory = tmp;
  }

  char *original_cwd = get_current_dir_name ();
  if (original_cwd == NULL)
    FAIL_EXIT1 ("get_current_dir_name: %m");

  int iteration = 0;
  for (int do_spawnp = 0; do_spawnp < 2; ++do_spawnp)
    for (int do_overwrite = 0; do_overwrite < 2; ++do_overwrite)
      {
        ++iteration;
        printf ("info: iteration=%d do_spawnp=%d do_overwrite=%d\n",
                iteration, do_spawnp, do_overwrite);

        /* The "pwd" program runs in this directory.  */
        char *iteration_directory = xasprintf ("%s/%d", directory, iteration);
        add_temp_file (iteration_directory);
        xmkdir (iteration_directory, 0777);

        /* This file receives output from "pwd".  */
        char *output_file_path
          = xasprintf ("%s/output-file", iteration_directory);
        add_temp_file (output_file_path);

        /* This subdirectory is used for chdir ordering checks.  */
        char *subdir_path = xasprintf ("%s/subdir", iteration_directory);
        add_temp_file (subdir_path);
        xmkdir (subdir_path, 0777);

        /* Also used for checking the order of actions.  */
        char *probe_file_path
          = xasprintf ("%s/subdir/probe-file", iteration_directory);
        add_temp_file (probe_file_path);
        TEST_COMPARE (access (probe_file_path, F_OK), -1);
        TEST_COMPARE (errno, ENOENT);

        /* This symbolic link is used in a relative path with
           posix_spawn.  */
        char *pwd_symlink_path
          = xasprintf ("%s/subdir/pwd-symlink", iteration_directory);
        xsymlink (get_pwd_program (), pwd_symlink_path);
        add_temp_file (pwd_symlink_path);

        posix_spawn_file_actions_t actions;
        TEST_COMPARE (posix_spawn_file_actions_init (&actions), 0);
        TEST_COMPARE (posix_spawn_file_actions_addchdir_np
                      (&actions, subdir_path), 0);
        TEST_COMPARE (posix_spawn_file_actions_addopen
                      (&actions, 3, /* Arbitrary unused descriptor.  */
                       "probe-file",
                       O_WRONLY | O_CREAT | O_EXCL, 0777), 0);
        TEST_COMPARE (posix_spawn_file_actions_addclose (&actions, 3), 0);
        /* Run the actual in iteration_directory.  */
        TEST_COMPARE (posix_spawn_file_actions_addchdir_np (&actions, ".."), 0);
        TEST_COMPARE (posix_spawn_file_actions_addopen
                      (&actions, STDOUT_FILENO, "output-file",
                       O_WRONLY | O_CREAT | O_EXCL, 0777), 0);

        /* Check that posix_spawn_file_actions_addchdir_np made a copy
           of the path.  */
        if (do_overwrite)
          subdir_path[0] = '\0';

        char *const argv[] = { (char *) "pwd", NULL };
        char *const envp[] = { NULL } ;
        pid_t pid;
        if (do_spawnp)
          TEST_COMPARE (posix_spawnp (&pid, "pwd", &actions,
                                      NULL, argv, envp), 0);
        else
          TEST_COMPARE (posix_spawn (&pid, "subdir/pwd-symlink", &actions,
                                     NULL, argv, envp), 0);
        TEST_VERIFY (pid > 0);
        int status;
        xwaitpid (pid, &status, 0);
        TEST_COMPARE (status, 0);

        /* Check that the current directory did not change.  */
        {
          char *cwd = get_current_dir_name ();
          if (cwd == NULL)
            FAIL_EXIT1 ("get_current_dir_name: %m");
          TEST_COMPARE_BLOB (original_cwd, strlen (original_cwd),
                             cwd, strlen (cwd));
          free (cwd);
        }


        /* Check the output from "pwd".  */
        {
          char *pwd = read_one_line (output_file_path);
          TEST_COMPARE_BLOB (iteration_directory, strlen (iteration_directory),
                             pwd, strlen (pwd));
          free (pwd);
        }

        /* This file must now exist.  */
        TEST_COMPARE (access (probe_file_path, F_OK), 0);

        TEST_COMPARE (posix_spawn_file_actions_destroy (&actions), 0);
        free (pwd_symlink_path);
        free (probe_file_path);
        free (subdir_path);
        free (output_file_path);
      }

  free (directory);

  return 0;
}

#include <support/test-driver.c>
