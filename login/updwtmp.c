/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "utmp-private.h"


void
updwtmp (const char *wtmp_file, const struct utmp *utmp)
{
  struct stat st;
  size_t written;
  int fd;

  /* Open WTMP file.  */
  fd = __open (wtmp_file, O_WRONLY | O_APPEND);
  if (fd < 0)
      return;

  /* Try to lock the file.  */
  if (__flock (fd, LOCK_EX | LOCK_NB) < 0 && errno != ENOSYS)
    {
      /* Oh, oh.  The file is already locked.  Wait a bit and try again.  */
      sleep (1);

      /* This time we ignore the error.  */
      __flock (fd, LOCK_EX | LOCK_NB);
    }

  /* Remember original size of log file: */
  if (__fstat (fd, &st) < 0)
    goto done;

  /* Write the entry.  If we can't write all the bytes, reset the file
     size back to the original size.  That way, no partial entries
     will remain.  */
  written = __write (fd, utmp, sizeof (struct utmp));
  if (written > 0 && written != sizeof (struct utmp))
    ftruncate (fd, st.st_size);

done:
  /* And unlock the file.  */
  __flock (fd, LOCK_UN);

  /* Close WTMP file.  */
  __close (fd);
}
