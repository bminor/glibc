/* pthread_getname_np -- Get  thread name.  Linux version
   Copyright (C) 2010-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <intprops.h>
#include <libc-lock.h>
#include <not-cancel.h>
#include <pthreadP.h>
#include <shlib-compat.h>
#include <stdio.h>
#include <sys/prctl.h>

static int
getname (pid_t tid, char *buf, size_t len)
{
  char fname[sizeof ("/proc/self/task//comm") + INT_BUFSIZE_BOUND (pid_t)];
  __snprintf (fname, sizeof (fname), "/proc/self/task/%d/comm", tid);

  int fd = __open64_nocancel (fname, O_RDONLY);
  if (fd == -1)
    return EINVAL;

  int res = 0;
  ssize_t n = TEMP_FAILURE_RETRY (__read_nocancel (fd, buf, len));
  if (n > 0)
    {
      if (buf[n - 1] == '\n')
	buf[n - 1] = '\0';
      else if (n == len)
	res = ERANGE;
      else
	buf[n] = '\0';
    }
  else
    res = errno == ENOENT ? EINVAL : errno;;

  __close_nocancel_nostatus (fd);

  return res;
}

int
__pthread_getname_np (pthread_t th, char *buf, size_t len)
{
  struct pthread *pd = (struct pthread *) th;

  /* Unfortunately the kernel headers do not export the TASK_COMM_LEN
     macro.  So we have to define it here.  */
#define TASK_COMM_LEN 16
  if (len < TASK_COMM_LEN)
    return ERANGE;

  if (pd == THREAD_SELF)
    return __prctl (PR_GET_NAME, buf) ? errno : 0;

  /* Block all signals, as required by pd->exit_lock.  */
  internal_sigset_t old_mask;
  internal_signal_block_all (&old_mask);
  __libc_lock_lock (pd->exit_lock);

  int res = pd->tid > 0 ? getname (pd->tid, buf, len) : EINVAL;

  __libc_lock_unlock (pd->exit_lock);
  internal_signal_restore_set (&old_mask);

  return res;
}
versioned_symbol (libc, __pthread_getname_np, pthread_getname_np,
		  GLIBC_2_34);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_12, GLIBC_2_34)
compat_symbol (libpthread, __pthread_getname_np, pthread_getname_np,
	       GLIBC_2_12);
#endif
