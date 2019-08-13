/* Copyright (C) 1996-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <not-cancel.h>
#include <kernel-features.h>
#include <sigsetops.h>
#include <not-cancel.h>

#include "utmp-private.h"
#include "utmp-equal.h"


/* Descriptor for the file and position.  */
static int file_fd = -1;
static bool file_writable;
static off64_t file_offset;

/* Cache for the last read entry.  */
static struct utmp last_entry;


/* Locking timeout.  */
#ifndef TIMEOUT
# define TIMEOUT 10
#endif

/* Do-nothing handler for locking timeout.  */
static void timeout_handler (int signum) {};


/* try_file_lock (LOCKING, FD, TYPE) returns true if the locking
   operation failed and recovery needs to be performed.
   (file_lock_restore (LOCKING) still needs to be called.)

   file_unlock (FD) removes the lock (which must have been
   acquired).

   file_lock_restore (LOCKING) is needed to clean up in both
   cases.  */

struct file_locking
{
  struct sigaction old_action;
  unsigned int old_timeout;
};

static bool
try_file_lock (struct file_locking *locking, int fd, int type)
{
  /* Cancel any existing alarm.  */
  locking->old_timeout = alarm (0);

  /* Establish signal handler.  */
  struct sigaction action;
  action.sa_handler = timeout_handler;
  __sigemptyset (&action.sa_mask);
  action.sa_flags = 0;
  __sigaction (SIGALRM, &action, &locking->old_action);

  alarm (TIMEOUT);

  /* Try to get the lock.  */
 struct flock fl =
   {
    .l_type = type,
    fl.l_whence = SEEK_SET,
   };
 return __fcntl64_nocancel (fd, F_SETLKW, &fl) < 0;
}

static void
file_unlock (int fd)
{
  struct flock fl =
    {
      .l_type = F_UNLCK,
    };
  __fcntl64_nocancel (fd, F_SETLKW, &fl);
}

static void
file_lock_restore (struct file_locking *locking)
{
  /* Reset the signal handler and alarm.  We must reset the alarm
     before resetting the handler so our alarm does not generate a
     spurious SIGALRM seen by the user.  However, we cannot just set
     the user's old alarm before restoring the handler, because then
     it's possible our handler could catch the user alarm's SIGARLM
     and then the user would never see the signal he expected.  */
  alarm (0);
  __sigaction (SIGALRM, &locking->old_action, NULL);
  if (locking->old_timeout != 0)
    alarm (locking->old_timeout);
}

#ifndef TRANSFORM_UTMP_FILE_NAME
# define TRANSFORM_UTMP_FILE_NAME(file_name) (file_name)
#endif

int
__libc_setutent (void)
{
  if (file_fd < 0)
    {
      const char *file_name;

      file_name = TRANSFORM_UTMP_FILE_NAME (__libc_utmp_file_name);

      file_writable = false;
      file_fd = __open_nocancel
	(file_name, O_RDONLY | O_LARGEFILE | O_CLOEXEC);
      if (file_fd == -1)
	return 0;
    }

  __lseek64 (file_fd, 0, SEEK_SET);
  file_offset = 0;

  /* Make sure the entry won't match.  */
  last_entry.ut_type = -1;

  return 1;
}

/* Preform initialization if necessary.  */
static bool
maybe_setutent (void)
{
  return file_fd >= 0 || __libc_setutent ();
}

int
__libc_getutent_r (struct utmp *buffer, struct utmp **result)
{
  ssize_t nbytes;

  if (!maybe_setutent () || file_offset == -1l)
    {
      /* Not available.  */
      *result = NULL;
      return -1;
    }

  struct file_locking fl;
  if (try_file_lock (&fl, file_fd, F_RDLCK))
    nbytes = 0;
  else
    {
      /* Read the next entry.  */
      nbytes = __read_nocancel (file_fd, &last_entry, sizeof (struct utmp));
      file_unlock (file_fd);
    }
  file_lock_restore (&fl);

  if (nbytes != sizeof (struct utmp))
    {
      if (nbytes != 0)
	file_offset = -1l;
      *result = NULL;
      return -1;
    }

  /* Update position pointer.  */
  file_offset += sizeof (struct utmp);

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


static int
internal_getut_r (const struct utmp *id, struct utmp *buffer,
		  bool *lock_failed)
{
  int result = -1;

  struct file_locking fl;
  if (try_file_lock (&fl, file_fd, F_RDLCK))
    {
      *lock_failed = true;
      file_lock_restore (&fl);
      return -1;
    }

  if (id->ut_type == RUN_LVL || id->ut_type == BOOT_TIME
      || id->ut_type == OLD_TIME || id->ut_type == NEW_TIME)
    {
      /* Search for next entry with type RUN_LVL, BOOT_TIME,
	 OLD_TIME, or NEW_TIME.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (__read_nocancel (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      goto unlock_return;
	    }
	  file_offset += sizeof (struct utmp);

	  if (id->ut_type == buffer->ut_type)
	    break;
	}
    }
  else
    {
      /* Search for the next entry with the specified ID and with type
	 INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, or DEAD_PROCESS.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (__read_nocancel (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      goto unlock_return;
	    }
	  file_offset += sizeof (struct utmp);

	  if (__utmp_equal (buffer, id))
	    break;
	}
    }

  result = 0;

unlock_return:
  file_unlock (file_fd);
  file_lock_restore (&fl);

  return result;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
int
__libc_getutid_r (const struct utmp *id, struct utmp *buffer,
		  struct utmp **result)
{
  if (!maybe_setutent () || file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  /* We don't have to distinguish whether we can lock the file or
     whether there is no entry.  */
  bool lock_failed = false;
  if (internal_getut_r (id, &last_entry, &lock_failed) < 0)
    {
      *result = NULL;
      return -1;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
int
__libc_getutline_r (const struct utmp *line, struct utmp *buffer,
		    struct utmp **result)
{
  if (!maybe_setutent () || file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  struct file_locking fl;
  if (try_file_lock (&fl, file_fd, F_RDLCK))
    {
      *result = NULL;
      file_lock_restore (&fl);
      return -1;
    }

  while (1)
    {
      /* Read the next entry.  */
      if (__read_nocancel (file_fd, &last_entry, sizeof (struct utmp))
	  != sizeof (struct utmp))
	{
	  __set_errno (ESRCH);
	  file_offset = -1l;
	  *result = NULL;
	  goto unlock_return;
	}
      file_offset += sizeof (struct utmp);

      /* Stop if we found a user or login entry.  */
      if ((last_entry.ut_type == USER_PROCESS
	   || last_entry.ut_type == LOGIN_PROCESS)
	  && (strncmp (line->ut_line, last_entry.ut_line, sizeof line->ut_line)
	      == 0))
	break;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

unlock_return:
  file_unlock (file_fd);
  file_lock_restore (&fl);

  return ((*result == NULL) ? -1 : 0);
}


struct utmp *
__libc_pututline (const struct utmp *data)
{
  if (!maybe_setutent ())
    return NULL;

  struct utmp buffer;
  struct utmp *pbuf;
  int found;

  if (! file_writable)
    {
      /* We must make the file descriptor writable before going on.  */
      const char *file_name = TRANSFORM_UTMP_FILE_NAME (__libc_utmp_file_name);

      int new_fd = __open_nocancel
	(file_name, O_RDWR | O_LARGEFILE | O_CLOEXEC);
      if (new_fd == -1)
	return NULL;

      if (__lseek64 (new_fd, __lseek64 (file_fd, 0, SEEK_CUR), SEEK_SET) == -1
	  || __dup2 (new_fd, file_fd) < 0)
	{
	  __close_nocancel_nostatus (new_fd);
	  return NULL;
	}
      __close_nocancel_nostatus (new_fd);
      file_writable = true;
    }

  /* Find the correct place to insert the data.  */
  if (file_offset > 0
      && ((last_entry.ut_type == data->ut_type
	   && (last_entry.ut_type == RUN_LVL
	       || last_entry.ut_type == BOOT_TIME
	       || last_entry.ut_type == OLD_TIME
	       || last_entry.ut_type == NEW_TIME))
	  || __utmp_equal (&last_entry, data)))
    found = 1;
  else
    {
      bool lock_failed = false;
      found = internal_getut_r (data, &buffer, &lock_failed);

      if (__builtin_expect (lock_failed, false))
	{
	  __set_errno (EAGAIN);
	  return NULL;
	}
    }

  struct file_locking fl;
  if (try_file_lock (&fl, file_fd, F_WRLCK))
    {
      file_lock_restore (&fl);
      return NULL;
    }

  if (found < 0)
    {
      /* We append the next entry.  */
      file_offset = __lseek64 (file_fd, 0, SEEK_END);
      if (file_offset % sizeof (struct utmp) != 0)
	{
	  file_offset -= file_offset % sizeof (struct utmp);
	  __ftruncate64 (file_fd, file_offset);

	  if (__lseek64 (file_fd, 0, SEEK_END) < 0)
	    {
	      pbuf = NULL;
	      goto unlock_return;
	    }
	}
    }
  else
    {
      /* We replace the just read entry.  */
      file_offset -= sizeof (struct utmp);
      __lseek64 (file_fd, file_offset, SEEK_SET);
    }

  /* Write the new data.  */
  if (__write_nocancel (file_fd, data, sizeof (struct utmp))
      != sizeof (struct utmp))
    {
      /* If we appended a new record this is only partially written.
	 Remove it.  */
      if (found < 0)
	(void) __ftruncate64 (file_fd, file_offset);
      pbuf = NULL;
    }
  else
    {
      file_offset += sizeof (struct utmp);
      pbuf = (struct utmp *) data;
    }

 unlock_return:
  file_unlock (file_fd);
  file_lock_restore (&fl);

  return pbuf;
}


void
__libc_endutent (void)
{
  if (file_fd >= 0)
    {
      __close_nocancel_nostatus (file_fd);
      file_fd = -1;
    }
}


int
__libc_updwtmp (const char *file, const struct utmp *utmp)
{
  int result = -1;
  off64_t offset;
  int fd;

  /* Open WTMP file.  */
  fd = __open_nocancel (file, O_WRONLY | O_LARGEFILE);
  if (fd < 0)
    return -1;

  struct file_locking fl;
  if (try_file_lock (&fl, fd, F_WRLCK))
    {
      file_lock_restore (&fl);
      __close_nocancel_nostatus (fd);
      return -1;
    }

  /* Remember original size of log file.  */
  offset = __lseek64 (fd, 0, SEEK_END);
  if (offset % sizeof (struct utmp) != 0)
    {
      offset -= offset % sizeof (struct utmp);
      __ftruncate64 (fd, offset);

      if (__lseek64 (fd, 0, SEEK_END) < 0)
	goto unlock_return;
    }

  /* Write the entry.  If we can't write all the bytes, reset the file
     size back to the original size.  That way, no partial entries
     will remain.  */
  if (__write_nocancel (fd, utmp, sizeof (struct utmp))
      != sizeof (struct utmp))
    {
      __ftruncate64 (fd, offset);
      goto unlock_return;
    }

  result = 0;

unlock_return:
  file_unlock (file_fd);
  file_lock_restore (&fl);

  /* Close WTMP file.  */
  __close_nocancel_nostatus (fd);

  return result;
}
