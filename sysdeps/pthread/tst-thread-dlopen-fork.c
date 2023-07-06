/* Test that dlopen works after concurrent fork.
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

#include <array_length.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xdlfcn.h>
#include <support/xthread.h>
#include <support/xunistd.h>
#include <unistd.h>

/* Use atomics to make sure that issues with dlopen/fork come from
   implementation problems, and not from exposing the handle of a
   partially initialized link map after dlopen returned.  */
static void *_Atomic handles[17];

/* Set to true if the dlopen thread has exited.  */
static _Atomic bool requested_exit;

/* Used to start the forking and the dlopen thread at the same time.  */
static pthread_barrier_t barrier;

/* Returns the file to open for IDX.  */
static char *
dso_name (int idx)
{
  return xasprintf ("tst-thread-dlopen-fork-mod-%d.so", idx);
}

static void *
forking_thread (void *closure)
{
  xpthread_barrier_wait (&barrier);
  int mode = 0;
  while (!requested_exit)
    {
      pid_t pid = xfork ();
      if (pid == 0)
        {
          switch (mode)
            {
            case 0:
              /* Try to open all handles.  */
              for (int i = 0; i < array_length (handles); ++i)
                {
                  char *name = dso_name (i);
                  void *handle = xdlopen (name, RTLD_LAZY);
                  if (handles[i] != NULL)
                    TEST_VERIFY (handle == handles[i]);
                  free (name);
                }
              mode = 1;
              break;
            case 1:
              /* Try to open all unopened handles.  */
              for (int i = 0; i < array_length (handles); ++i)
                if (handles[i] == NULL)
                  {
                    char *name = dso_name (i);
                    xdlopen (name, RTLD_LAZY);
                    free (name);
                  }
              mode = 2;
              break;
            case 2:
              /* Try to close all opened handles.  */
              for (int i = 0; i < array_length (handles); ++i)
                if (handles[i] != NULL)
                  xdlclose (handles[i]);
              break;
            }
          _exit (0);
        }
      int status;
      xwaitpid (pid, &status, 0);
      TEST_COMPARE (status, 0);
    }
  return NULL;
}

static void *
dlopen_thread (void *closure)
{
  xpthread_barrier_wait (&barrier);
  srand (1);
  for (int i = 0; i <= 1000; ++i)
    {
      int idx = rand () % array_length (handles);
      void *handle = handles[idx];
      if (handle == NULL)
        {
          char *name = dso_name (idx);
          handles[idx] = xdlopen (name, RTLD_LAZY);
          free (name);
        }
      else
        {
          handles[idx] = NULL;
          xdlclose (handle);
        }
    }
  return NULL;
}

static int
do_test (void)
{
  xpthread_barrier_init (&barrier, NULL, 2);
  pthread_t thr_fork = xpthread_create (NULL, forking_thread, NULL);
  pthread_t thr_dlopen = xpthread_create (NULL, dlopen_thread, NULL);
  xpthread_join (thr_dlopen);
  requested_exit = true;
  xpthread_join (thr_fork);
  return 0;
}

#include <support/test-driver.c>
