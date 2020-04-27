/* Copyright (C) 2017-2020 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <support/check.h>
#include <support/xdlfcn.h>
#include <support/xstdio.h>
#include <support/xunistd.h>
#include <wait.h>
#include <support/test-driver.h>

int
call_execveat (int fd, const char *pathname, int flags, int expected_fail, int num)
{
  char *argv[] = { (char *) "sh", (char *) "-c", (char *) "exit 3", NULL };
  char *envp[] = { (char *) "FOO=BAR", NULL };
  pid_t pid;
  int status;

  printf("call number: %d\n", num);

  pid = xfork ();
  if (pid == 0)
  {

      TEST_COMPARE (execveat (fd, "sh", argv, envp, flags), -1);
      if (errno == ENOSYS)
          FAIL_UNSUPPORTED ("execveat is unimplemented");
      else if (errno == expected_fail)
      {
          if (test_verbose > 0)
              printf ("expected fail: errno %d\n", errno);
          _exit(0);
      }
      else
          FAIL_EXIT1 ("execveat failed, errno %d", errno);
  }
  xwaitpid (pid, &status, 0);

  if (WIFEXITED (status))
    if (expected_fail)
      TEST_COMPARE (WEXITSTATUS (status), 0);
    else
      TEST_COMPARE (WEXITSTATUS (status), 3);
  else if (!expected_fail)
      FAIL_EXIT1 ("execveat failed");
  return 0;
}

static int
do_test (void)
{
  DIR *dirp;
  int fd;

  dirp = opendir ("/bin");
  if (dirp == NULL)
    FAIL_EXIT1 ("failed to open /bin");
  fd = dirfd (dirp);

  /* Call execveat for various fd/pathname combinations  */

  /* fd: valid dir, pathname: relative, flags:: 0  */
  call_execveat (fd, "sh", 0, 0, 1);
  /* fd: valid dir, pathname: relative, flags: O_PATH  */
  call_execveat (fd, "sh", O_PATH, 0, 2);
  /* fd: AT_FDCWD, pathname: relative, flags: 0
     If pathname is relative and dirfd is the special value AT_FDCWD, then
     pathname is interpreted relative to the current working directory of
     the calling process  */
  chdir("/bin");
  call_execveat (AT_FDCWD, "sh", 0, 0, 3);
  xclose (fd);
  closedir (dirp);

  dirp = opendir ("/usr");
  fd = dirfd (dirp);
  chdir ("/etc");
  /* fd: AT_FDCWD, pathname: absolute in different dir, flags: 0  */
  call_execveat (AT_FDCWD, "/bin/sh", 0, 0, 4);

  /* fd: valid dir, pathname: absolute in differen dir, flags: 0  */
  call_execveat (fd, "/bin/sh", 0, 0, 5);
  /* fd: valid dir, pathname: absolute, flags: O_PATH  */
  call_execveat (fd, "/bin/sh", O_PATH, 0, 6);
  xclose (fd);
  closedir (dirp);

  fd = xopen ("/bin/sh", 0, 0);
  /* fd: regular file, pathname: relative, flags: 0  */
  call_execveat(fd, "sh", 0, ENOTDIR, 7);
  /* fd: regular file, pathname: absolute, flags: 0  */
  call_execveat (fd, "/bin/sh", 0, 0, 8);
  xclose (fd);

  fd = xopen ("/bin/sh", O_PATH, 0);
  /* fd: O_PATH of regular file, pathname: empty, flags: 0  */
  call_execveat (fd, "", 0, ENOTDIR, 10);
  /* fd: O_PATH of regular file, pathname: empty, flags: AT_EMPTY_PATH  */
  call_execveat (fd, "", AT_EMPTY_PATH, 0, 11); // fails with ENOTDIR (20)
  /* fd: O_PATH of regular file, pathname: empty,
     flags: AT_EMPTY_PATH  AT_SYMLINK_NOFOLLOW  */
  //   call_execveat (fd, "", AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW, NULL, 0, 12); //fails with ENOTDIR
  xclose (fd);

  fd = xopen ("/bin/sh", O_NOFOLLOW | O_PATH, 0);
  /* fd: O_PATH of symbolic link, pathname: empty, flags:  */
//    call_execveat(fd, "", 0, 1, 13); //fails with  errno ENOTDIR
  /* fd: O_PATH of symbolic link, pathname: empty, flags:  */
//    call_execveat (fd, "", AT_EMPTY_PATH, 0, 14); //fails with  errno ENOTDIR
  /* fd: O_PATH of symbolic link, pathname: empty,
    flags: AT_EMPTY_PATH  AT_SYMLINK_NOFOLLOW */
  call_execveat (fd, "", AT_EMPTY_PATH | AT_SYMLINK_NOFOLLOW, ENOTDIR, 15);
  xclose (fd);

  /* Call execveat with closed fd, we expect this to fail with EBADF  */
  call_execveat (fd, "sh", 0, EBADF, 16);
  /* Call execveat with closed fd, we expect this to pass because the pathname is
    absolute  */
  call_execveat (fd, "/bin/sh", 0, 0, 17);

  return 0;
}

#include <support/test-driver.c>
