/* Basic tests for sealing.
   Copyright (C) 2025 Free Software Foundation, Inc.
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
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xdlfcn.h>
#include <support/xstdio.h>
#include <support/xthread.h>

#if UINTPTR_MAX == UINT64_MAX
# define PTR_FMT "#018" PRIxPTR
#else
# define PTR_FMT "#010" PRIxPTR
#endif

static int
new_flags (const char flags[4])
{
  bool read_flag  = flags[0] == 'r';
  bool write_flag = flags[1] == 'w';
  bool exec_flag  = flags[2] == 'x';

  write_flag = !write_flag;

  return (read_flag ? PROT_READ : 0)
	 | (write_flag ? PROT_WRITE : 0)
	 | (exec_flag ? PROT_EXEC : 0);
}

/* Libraries/VMA that could not be sealed, and that checking for sealing
   does not work (kernel does not allow changing protection).  */
static const char *non_sealed_vmas[] =
{
  ".",				/* basename value for empty string anonymous
				   mappings.  */
  "[heap]",
  "[vsyscall]",
  "[vvar]",
  "[stack]",
  "[vvar_vclock]",
  "zero",			/* /dev/zero  */
};

static int
is_in_string_list (const char *s, const char *const list[], size_t len)
{
  for (size_t i = 0; i != len; i++)
    if (strcmp (s, list[i]) == 0)
      return i;
  return -1;
}
#define IS_IN_STRING_LIST(__s, __list) \
  is_in_string_list (__s, __list, array_length (__list))

static void *
tf (void *closure)
{
  pthread_exit (NULL);
  return NULL;
}

static int
handle_restart (void)
{
#ifndef TEST_STATIC
  xdlopen (LIB_DLOPEN_NODELETE, RTLD_NOW | RTLD_NODELETE);
  xdlopen (LIB_DLOPEN_DEFAULT, RTLD_NOW);
#endif

  /* pthread_exit will load LIBGCC_S_SO.  */
  xpthread_join (xpthread_create (NULL, tf, NULL));

  FILE *fp = xfopen ("/proc/self/maps", "r");
  char *line = NULL;
  size_t linesiz = 0;

  unsigned long pagesize = getpagesize ();

  bool found_expected[array_length(expected_sealed_vmas)] = { false };
  while (xgetline (&line, &linesiz, fp) > 0)
    {
      uintptr_t start;
      uintptr_t end;
      char flags[5] = { 0 };
      char name[256] = { 0 };
      int idx;

      /* The line is in the form:
	 start-end flags offset dev inode pathname   */
      int r = sscanf (line,
		      "%" SCNxPTR "-%" SCNxPTR " %4s %*s %*s %*s %256s",
		      &start,
		      &end,
		      flags,
		      name);
      TEST_VERIFY_EXIT (r == 3 || r == 4);

      int found = false;

      const char *libname = basename (name);
      if ((idx = IS_IN_STRING_LIST (libname, expected_sealed_vmas))
	   != -1)
	{
	  /* Check if we can change the protection flags of the segment.  */
	  int new_prot = new_flags (flags);
	  TEST_VERIFY_EXIT (mprotect ((void *) start, end - start,
				      new_prot) == -1);
	  TEST_VERIFY_EXIT (errno == EPERM);

	  /* Also checks trying to map over the sealed libraries.  */
	  {
	    char *p = mmap ((void *) start, pagesize, new_prot,
			    MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	    TEST_VERIFY_EXIT (p == MAP_FAILED);
	    TEST_VERIFY_EXIT (errno == EPERM);
	  }

	  /* And if remap is also blocked.  */
	  {
	    char *p = mremap ((void *) start, end - start, end - start, 0);
	    TEST_VERIFY_EXIT (p == MAP_FAILED);
	    TEST_VERIFY_EXIT (errno == EPERM);
	  }

	  printf ("sealed:     vma: %" PTR_FMT "-%" PTR_FMT " %s %s\n",
		  start,
		  end,
		  flags,
		  name);

	  found_expected[idx] = true;
	  found = true;
	}
      else if ((idx = IS_IN_STRING_LIST (libname, expected_non_sealed_vmas))
	       != -1)
	{
	  /* Check if expected non-sealed segments protection can indeed be
	     changed.  The idea is to use something that would not break
	     process execution, so just try to mprotect with all protection
	     bits.  */
	  int new_prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	  TEST_VERIFY_EXIT (mprotect ((void *) start, end - start, new_prot)
			    == 0);

	  printf ("not-sealed: vma: %" PTR_FMT "-%" PTR_FMT " %s %s\n",
		  start,
		  end,
		  flags,
		  name);

	  found = true;
	}
      else if (IS_IN_STRING_LIST (libname, expected_non_sealed_special) != -1)
	{
	  /* These pages protection can no be changed.  */
	  found = true;
	}

      if (!found)
	{
	  if (IS_IN_STRING_LIST (libname, non_sealed_vmas) != -1)
	    printf ("not-sealed: vma: %" PTR_FMT "-%" PTR_FMT " %s %s\n",
		    start,
		    end,
		    flags,
		    name);
	  else
	    FAIL_EXIT1 ("unexpected vma: %" PTR_FMT "-%" PTR_FMT " %s %s\n",
			start,
			end,
			flags,
			name);
	}
    }
  xfclose (fp);

  printf ("\n");

  /* Also check if all the expected sealed maps were found.  */
  for (int i = 0; i < array_length (expected_sealed_vmas); i++)
    if (expected_sealed_vmas[i][0] && !found_expected[i])
      FAIL_EXIT1 ("expected VMA %s not sealed\n", expected_sealed_vmas[i]);

  return 0;
}

static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

static int
do_test (int argc, char *argv[])
{
  /* We must have either:
     - One or four parameters left if called initially:
       + path to ld.so         optional
       + "--library-path"      optional
       + the library path      optional
       + the application name  */
  if (restart)
    return handle_restart ();

  /* Check the test requirements.  */
  {
    int r = mseal (NULL, 0, 0);
    if (r == -1 && (errno == ENOSYS || errno == EPERM))
      FAIL_UNSUPPORTED ("mseal is not supported by the kernel");
    else
      TEST_VERIFY_EXIT (r == 0);
  }
  support_need_proc ("Reads /proc/self/maps to get stack names.");

  char *spargv[9];
  int i = 0;
  for (; i < argc - 1; i++)
    spargv[i] = argv[i + 1];
  spargv[i++] = (char *) "--direct";
  spargv[i++] = (char *) "--restart";
  spargv[i] = NULL;

  char *envvarss[] = {
#ifndef TEST_STATIC
    (char *) "LD_PRELOAD=" LIB_PRELOAD,
    (char *) "LD_AUDIT=" LIB_AUDIT,
#endif
    NULL
  };

  struct support_capture_subprocess result =
    support_capture_subprogram (spargv[0], spargv, envvarss);
  support_capture_subprocess_check (&result, "tst-dl_mseal", 0,
				    sc_allow_stdout);

  {
    FILE *out = fmemopen (result.out.buffer, result.out.length, "r");
    TEST_VERIFY (out != NULL);
    char *line = NULL;
    size_t linesz = 0;
    while (xgetline (&line, &linesz, out))
      printf ("%s", line);
    fclose (out);
  }

  support_capture_subprocess_free (&result);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
