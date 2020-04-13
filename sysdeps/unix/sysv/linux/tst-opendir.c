/* Check multiple telldir and seekdir.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

/* Some filesystems returns an arbitrary value for d_off direnty entry (ext4
   for instance, where the value is an internal hash key).  The idea of create
   a large number of file is to try trigger a overflow d_off value in a entry
   to check if telldir/seekdir does work corretly in such case.  */
static const char *dirname;
/* The 2 extra files are '.' and '..'.  */
static const size_t nfiles = (1<<14) + 2;

static inline bool
in_ino_t_range (ino64_t v)
{
  ino_t s = v;
  return s == v;
}

static inline bool
in_off_t_range (off64_t v)
{
  off_t s = v;
  return s == v;
}

static void
do_prepare (int argc, char *argv[])
{
  dirname = support_create_temp_directory ("tst-opendir-nolfs-");

  for (size_t i = 0; i < nfiles - 2; i++)
    {
      int fd = create_temp_file_in_dir ("tempfile.", dirname, NULL);
      TEST_VERIFY_EXIT (fd > 0);
      close (fd);
    }
}
#define PREPARE do_prepare

static int
do_test (void)
{
  DIR *dirp = opendir (dirname);
  TEST_VERIFY_EXIT (dirp != NULL);

  long int *tdirp = xreallocarray (NULL, nfiles, sizeof (long int));
  struct dirent **ddirp = xreallocarray (NULL, nfiles,
					 sizeof (struct dirent *));

  /* For non-LFS, the entry is skipped if it can not be converted.  */
  int count = 0;
  for (; count < nfiles; count++)
    {
      struct dirent *dp = readdir (dirp);
      if (dp == NULL)
	break;
      tdirp[count] = telldir (dirp);
      ddirp[count] = xmalloc (dp->d_reclen);
      memcpy (ddirp[count], dp, dp->d_reclen);
    }

  closedir (dirp);

  /* Check against the getdents64 syscall.  */
  int fd = xopen (dirname, O_RDONLY | O_DIRECTORY, 0);
  int i = 0;
  while (true)
    {
      struct
      {
	char buffer[1024];
	struct dirent64 pad;
      } data;

      ssize_t ret = getdents64 (fd, &data.buffer, sizeof (data.buffer));
      if (ret < 0)
	FAIL_EXIT1 ("getdents64: %m");
      if (ret == 0)
	break;

      char *current = data.buffer;
      char *end = data.buffer + ret;
      while (current != end)
	{
	  struct dirent64 entry;
          memcpy (&entry, current, sizeof (entry));
          /* Truncate overlong strings.  */
          entry.d_name[sizeof (entry.d_name) - 1] = '\0';
          TEST_VERIFY (strlen (entry.d_name) < sizeof (entry.d_name) - 1);

	  if (in_ino_t_range (entry.d_ino))
	    {
	      TEST_COMPARE_STRING (entry.d_name, ddirp[i]->d_name);
	      TEST_COMPARE (entry.d_ino, ddirp[i]->d_ino);
	      TEST_COMPARE (entry.d_type, ddirp[i]->d_type);

	      /* Offset zero is reserved for the first entry.  */
	      TEST_VERIFY (entry.d_off != 0);

	      TEST_VERIFY_EXIT (entry.d_reclen <= end - current);
	      i++;
	    }

	  current += entry.d_reclen;
	}
    }

  TEST_COMPARE (count, i);

  free (tdirp);
  for (int i = 0; i < count; i++)
    free (ddirp[i]);
  free (ddirp);

  return 0;
}

#include <support/test-driver.c>
