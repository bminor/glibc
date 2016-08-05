/* Check that getaddrinfo still works after recovery from failure.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

/* Thanks to Dave Clausen for the test concept.  */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>

#define NUMFDS 10
int fds[NUMFDS];

static int
do_test (void)
{
  char hostname[1024];
  struct rlimit rlim;
  int gaierr;
  struct addrinfo hints, *ai;

  gethostname (hostname, sizeof (hostname));

  /* Bring the limit of open files down to a small predictable value.  */

  if (getrlimit (RLIMIT_NOFILE, &rlim) < 0)
    {
      printf ("error: getrlimit: %m\n");
      return 1;
    }
  rlim.rlim_cur = NUMFDS;
  if (setrlimit (RLIMIT_NOFILE, &rlim) < 0)
    {
      printf ("error: setrlimit: %m\n");
      return 1;
    }

  memset (&hints, '\0', sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  gaierr = getaddrinfo (hostname, NULL, &hints, &ai);
  if (gaierr != 0)
    {
      printf ("error: getaddrinfo: %s\n", gai_strerror (gaierr));
      return 1;
    }

  // Create file descriptors until we completely run out.

  memset (fds, '\0', sizeof (fds));
  for (int i = 0; i < NUMFDS; ++i)
    {
      int fd = dup (STDOUT_FILENO);

      if (fd == -1 && errno == EMFILE)
	break;

      fds[i] = fd;
    }

  gaierr = getaddrinfo (hostname, NULL, &hints, &ai);
  if (gaierr != 0)
    {
      printf ("error (expected): getaddrinfo: %s\n", gai_strerror (gaierr));
      /* Don't give up, we expected this error.  */
    }

  // Now close the fds, freeing them up for use.

  for (int i = 0; i < NUMFDS; ++i)
    {
      if (fds[i] == 0)
	break;

      close (fds[i]);
    }

  /* This one should succeed again.  */

  gaierr = getaddrinfo (hostname, NULL, &hints, &ai);
  if (gaierr != 0)
    {
      printf ("error: getaddrinfo: %s\n", gai_strerror (gaierr));
      return 1;
    }

  return 0;
}

#define TEST_FUNCTION do_test ()

#include "../test-skeleton.c"
