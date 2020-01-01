/* Capture output from a subprocess.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
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

#include <support/subprocess.h>
#include <support/capture_subprocess.h>

#include <errno.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/xsocket.h>
#include <support/xspawn.h>

static void
transfer (const char *what, struct pollfd *pfd, struct xmemstream *stream)
{
  if (pfd->revents != 0)
    {
      char buf[1024];
      ssize_t ret = TEMP_FAILURE_RETRY (read (pfd->fd, buf, sizeof (buf)));
      if (ret < 0)
        {
          support_record_failure ();
          printf ("error: reading from subprocess %s: %m", what);
          pfd->events = 0;
          pfd->revents = 0;
        }
      else if (ret == 0)
        {
          /* EOF reached.  Stop listening.  */
          pfd->events = 0;
          pfd->revents = 0;
        }
      else
        /* Store the data just read.   */
        TEST_VERIFY (fwrite (buf, ret, 1, stream->out) == 1);
    }
}

static void
support_capture_poll (struct support_capture_subprocess *result,
		      struct support_subprocess *proc)
{
  struct pollfd fds[2] =
    {
      { .fd = proc->stdout_pipe[0], .events = POLLIN },
      { .fd = proc->stderr_pipe[0], .events = POLLIN },
    };

  do
    {
      xpoll (fds, 2, -1);
      transfer ("stdout", &fds[0], &result->out);
      transfer ("stderr", &fds[1], &result->err);
    }
  while (fds[0].events != 0 || fds[1].events != 0);

  xfclose_memstream (&result->out);
  xfclose_memstream (&result->err);

  result->status = support_process_wait (proc);
}

struct support_capture_subprocess
support_capture_subprocess (void (*callback) (void *), void *closure)
{
  struct support_capture_subprocess result;
  xopen_memstream (&result.out);
  xopen_memstream (&result.err);

  struct support_subprocess proc = support_subprocess (callback, closure);

  support_capture_poll (&result, &proc);
  return result;
}

struct support_capture_subprocess
support_capture_subprogram (const char *file, char *const argv[])
{
  struct support_capture_subprocess result;
  xopen_memstream (&result.out);
  xopen_memstream (&result.err);

  struct support_subprocess proc = support_subprogram (file, argv);

  support_capture_poll (&result, &proc);
  return result;
}

void
support_capture_subprocess_free (struct support_capture_subprocess *p)
{
  free (p->out.buffer);
  free (p->err.buffer);
}
