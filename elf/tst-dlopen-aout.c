/* Test case for BZ #16634 and BZ#24900.

   Verify that incorrectly dlopen()ing an executable without
   __RTLD_OPENEXEC does not cause assertion in ld.so, and that it
   actually results in an error.

   Copyright (C) 2014-2019 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xthread.h>

__thread int x;

void *
fn (void *p)
{
  return p;
}

/* Call dlopen on PATH and check that fails with an error message
   indicating an attempt to open an ET_EXEC or PIE object.  */
static void
check_dlopen_failure (const char *path)
{
  void *handle = dlopen (path, RTLD_LAZY);
  if (handle != NULL)
    FAIL_EXIT1 ("dlopen succeeded unexpectedly: %s", path);

  const char *message = dlerror ();
  TEST_VERIFY_EXIT (message != NULL);
  if ((strstr (message,
	       "cannot dynamically load position-independent executable")
       == NULL)
      && strstr (message, "cannot dynamically load executable") == NULL)
    FAIL_EXIT1 ("invalid dlopen error message: \"%s\"", message);
}

static int
do_test (int argc, char *argv[])
{
  int j;

  for (j = 0; j < 100; ++j)
    {
      pthread_t thr;

      check_dlopen_failure (argv[0]);

      /* We create threads to force TLS allocation, which triggers
	 the original bug i.e. running out of surplus slotinfo entries
	 for TLS.  */
      thr = xpthread_create (NULL, fn, NULL);
      xpthread_join (thr);
    }

  /* The elf subdirectory (or $ORIGIN in the container case) is on the
     library search path.  */
  check_dlopen_failure ("tst-dlopen-aout");

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
