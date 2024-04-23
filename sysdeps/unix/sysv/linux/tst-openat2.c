/* Linux openat2 tests.
   Copyright (C) 2024 Free Software Foundation, Inc.
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

/* openat2 is only support in LFS mode.  */
#define _FILE_OFFSET_BITS 64

#include <array_length.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <support/check.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

static int dir_fd;

static void
do_prepare (int argc, char *argv[])
{
  char *temp_dir = support_create_temp_directory ("tst-openat2");
  dir_fd = xopen (temp_dir, O_RDONLY | O_DIRECTORY, 0);
}
#define PREPARE do_prepare

static int
do_test_struct (void)
{
  static struct struct_test
  {
    struct open_how_ext
    {
      struct open_how inner;
      int extra1;
      int extra2;
      int extra3;
    } arg;
    size_t size;
    int err;
  } tests[] =
  {
    {
      /* Zero size.  */
      .arg.inner.flags = O_RDONLY,
      .size = 0,
      .err = EINVAL,
    },
    {
      /* Normal struct.  */
      .arg.inner.flags = O_RDONLY,
      .size = sizeof (struct open_how),
    },
    {
      /* Larger struct, zeroed out the unused values.  */
      .arg.inner.flags = O_RDONLY,
      .size = sizeof (struct open_how_ext),
    },
    {
      /* Larger struct, non-zeroed out the unused values.  */
      .arg.inner.flags = O_RDONLY,
      .arg.extra1 = 0xdeadbeef,
      .size = sizeof (struct open_how_ext),
      .err = E2BIG,
    },
    {
      /* Larger struct, non-zeroed out the unused values.  */
      .arg.inner.flags = O_RDONLY,
      .arg.extra2 = 0xdeadbeef,
      .size = sizeof (struct open_how_ext),
      .err = E2BIG,
    },
  };

  for (struct struct_test *t = tests; t != array_end (tests); t++)
    {
      int fd = openat2 (AT_FDCWD, ".", (struct open_how *) &t->arg, t->size);
      if (fd == -1 && errno == ENOSYS)
	FAIL_UNSUPPORTED ("openat2 is not supported by the kernel");

      if (t->err != 0)
	{
	  TEST_COMPARE (fd, -1);
	  TEST_COMPARE (errno, t->err);
	}
      else
	TEST_VERIFY (fd >= 0);
    }

  return 0;
}

static int
do_test_flags (void)
{
  static struct flag_test
  {
    struct open_how how;
    int err;
  } tests[] =
  {
    /* O_TMPFILE is incompatible with O_PATH and O_CREAT.  */
    { .how.flags = O_TMPFILE | O_PATH | O_RDWR, .err = EINVAL },
    { .how.flags = O_TMPFILE | O_CREAT | O_RDWR, .err = EINVAL },

    /* O_PATH only permits certain other flags to be set ...  */
    { .how.flags = O_PATH | O_CLOEXEC },
    { .how.flags = O_PATH | O_DIRECTORY },
    { .how.flags = O_PATH | O_NOFOLLOW },
    /* ... and others are absolutely not permitted. */
    { .how.flags = O_PATH | O_RDWR, .err = EINVAL },
    { .how.flags = O_PATH | O_CREAT, .err = EINVAL },
    { .how.flags = O_PATH | O_EXCL, .err = EINVAL },
    { .how.flags = O_PATH | O_NOCTTY, .err = EINVAL },
    { .how.flags = O_PATH | O_DIRECT, .err = EINVAL },

    /* ->mode must only be set with O_{CREAT,TMPFILE}. */
    { .how.flags = O_RDONLY, .how.mode = 0600, .err = EINVAL },
    { .how.flags = O_PATH,   .how.mode = 0600, .err = EINVAL },
    { .how.flags = O_CREAT,  .how.mode = 0600 },
    { .how.flags = O_TMPFILE | O_RDWR, .how.mode = 0600 },
    /* ->mode must only contain 0777 bits. */
    { .how.flags = O_CREAT, .how.mode = 0xFFFF, .err = EINVAL },
    { .how.flags = O_CREAT, .how.mode = 0xC000000000000000ULL, .err = EINVAL },
    { .how.flags = O_TMPFILE | O_RDWR, .how.mode = 0x1337, .err = EINVAL },
    { .how.flags = O_TMPFILE | O_RDWR, .how.mode = 0x0000A00000000000ULL,
      .err = EINVAL },

    /* ->resolve flags must not conflict. */
    { .how.flags = O_RDONLY,
      .how.resolve = RESOLVE_BENEATH | RESOLVE_IN_ROOT,
      .err = EINVAL },

    /* ->resolve must only contain RESOLVE_* flags.  */
    { .how.flags = O_RDONLY,
      .how.resolve = 0x1337, .err = EINVAL },
    { .how.flags = O_CREAT,
      .how.resolve = 0x1337, .err = EINVAL },
    { .how.flags = O_TMPFILE | O_RDWR,
      .how.resolve = 0x1337, .err = EINVAL },
    { .how.flags = O_PATH,
      .how.resolve = 0x1337, .err = EINVAL },

    /* currently unknown upper 32 bit rejected.  */
    { .how.flags = O_RDONLY | (1ULL << 63),
      .how.resolve = 0, .err = EINVAL },
  };

  for (struct flag_test *t = tests; t != array_end (tests); t++)
    {
      char *path;
      if (t->how.flags & O_CREAT)
	{
	  int temp_fd = create_temp_file ("tst-openat2.", &path);
	  TEST_VERIFY_EXIT (temp_fd != -1);
	  xunlink (path);
	}
      else
	path = (char *) ".";

      int fd = openat2 (AT_FDCWD, path, &t->how, sizeof (struct open_how));
      if (fd != 0 && errno == EOPNOTSUPP)
	{
	  /* Skip the testcase if FS does not support the operation (e.g.
	     valid O_TMPFILE on NFS).  */
	  continue;
	}

      if (t->err != 0)
	{
	  TEST_COMPARE (fd, -1);
	  TEST_COMPARE (errno, t->err);
	}
      else
	TEST_VERIFY (fd >= 0);
    }

  return 0;
}

static int
do_test_basic (void)
{
  int fd;

  fd = openat2 (dir_fd,
		"some-file",
		&(struct open_how)
		{
		  .flags = O_CREAT|O_RDWR|O_EXCL,
		  .mode = 0666,
		},
		sizeof (struct open_how));
  TEST_VERIFY (fd != -1);

  xwrite (fd, "hello", 5);

  /* Before closing the file, try using this file descriptor to open
     another file.  This must fail.  */
  {
    int fd2 = openat2 (fd,
		       "should-not-work",
		       &(struct open_how)
		       {
			 .flags = O_CREAT|O_RDWR|O_EXCL,
			 .mode = 0666,
		       },
		       sizeof (struct open_how));
    TEST_COMPARE (fd2, -1);
    TEST_COMPARE (errno, ENOTDIR);
  }

  /* Remove the created file.  */
  int cwdfd = xopen (".", O_RDONLY | O_DIRECTORY, 0);
  TEST_COMPARE (fchdir (dir_fd), 0);
  xunlink ("some-file");
  TEST_COMPARE (fchdir (cwdfd), 0);

  xclose (dir_fd);
  xclose (cwdfd);

  fd = openat2 (dir_fd,
		"some-file",
		&(struct open_how)
		{
		  .flags = O_CREAT|O_RDWR|O_EXCL,
		  .mode = 0666,
		},
		sizeof (struct open_how));
  TEST_COMPARE (fd, -1);
  TEST_COMPARE (errno, EBADF);

  return 0;
}

static int
do_test (void)
{
  do_test_struct ();
  do_test_flags ();
  do_test_basic ();

  return 0;
}

#include <support/test-driver.c>
