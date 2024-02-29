/* Check GLIBC_TUNABLES parsing for enable_secure.
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

#include <array_length.h>
#include <dl-tunables.h>
#include <getopt.h>
#include <intprops.h>
#include <stdint.h>
#include <stdlib.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <sys/auxv.h>
#include <unistd.h>

static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

static const struct test_t
{
  const char *env;
  int32_t expected_malloc_check;
  int32_t expected_enable_secure;
} tests[] =
{
  /* Expected tunable format.  */
  /* Tunables should be ignored if enable_secure is set. */
  {
    "glibc.malloc.check=2:glibc.rtld.enable_secure=1",
    0,
    1,
  },
  /* Tunables should be ignored if enable_secure is set. */
  {
    "glibc.rtld.enable_secure=1:glibc.malloc.check=2",
    0,
    1,
  },
  /* Tunables should be set if enable_secure is unset. */
  {
    "glibc.rtld.enable_secure=0:glibc.malloc.check=2",
    2,
    0,
  },
};

static int
handle_restart (int i)
{
  if (tests[i].expected_enable_secure == 1)
    {
      TEST_COMPARE (1, __libc_enable_secure);
    }
  else
    {
      TEST_COMPARE (tests[i].expected_malloc_check,
		    TUNABLE_GET_FULL (glibc, malloc, check, int32_t, NULL));
      TEST_COMPARE (tests[i].expected_enable_secure,
		    TUNABLE_GET_FULL (glibc, rtld, enable_secure, int32_t,
		    NULL));
    }
  return 0;
}

static int
do_test (int argc, char *argv[])
{
  /* We must have either:
     - One or four parameters left if called initially:
       + path to ld.so         optional
       + "--library-path"      optional
       + the library path      optional
       + the application name
       + the test to check  */

  TEST_VERIFY_EXIT (argc == 2 || argc == 5);

  if (restart)
    return handle_restart (atoi (argv[1]));

  char nteststr[INT_BUFSIZE_BOUND (int)];

  char *spargv[10];
  {
    int i = 0;
    for (; i < argc - 1; i++)
      spargv[i] = argv[i + 1];
    spargv[i++] = (char *) "--direct";
    spargv[i++] = (char *) "--restart";
    spargv[i++] = nteststr;
    spargv[i] = NULL;
  }

  for (int i = 0; i < array_length (tests); i++)
    {
      snprintf (nteststr, sizeof nteststr, "%d", i);

      printf ("[%d] Spawned test for %s\n", i, tests[i].env);
      setenv ("GLIBC_TUNABLES", tests[i].env, 1);
      struct support_capture_subprocess result
	= support_capture_subprogram (spargv[0], spargv);
      support_capture_subprocess_check (&result, "tst-tunables-enable_secure",
		                        0, sc_allow_stderr);
      support_capture_subprocess_free (&result);
    }

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
