/* Copyright (C) 2020 Free Software Foundation, Inc.
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

/* This test checks that leakage does not happen from repeated loads
   of a module with TLS.  */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int (*tlsmod_fun) (void);

/* Async-signal-safe TLS space is gotten via custom allocator using mmap(), so
   it doesn't work to use functions like mtrace() or mallinfo().  */

size_t get_vm_size (void)
{
  long result;
  FILE *f = fopen ("/proc/self/statm", "r");

  if (!f)
    {
      fprintf (stderr, "get_vm_size could not open /proc/self/statm\n");
      return 0;
    }
  if (fscanf (f, "%lu", &result) != 1)
    {
      fprintf (stderr, "get_vm_size failed to read size\n");
      return 0;
    }
  if (fclose (f) != 0)
    {
      fprintf (stderr, "get_vm_size fclose failed\n");
      return 0;
    }

  return (size_t) result;
}

int
do_test (void)
{
  int count = 0;

  size_t start_size = get_vm_size ();

  if (start_size == 0)
    {
      puts ("Cannot get vm size, test cannot run");
      exit (0);
    }

  /* Open, use, and close a shared library repeatedly.  */
  for (int i = 0; i < 1000; ++i)
    {
      void *h = dlopen ("tst-tls7leakmod.so", RTLD_NOW);
      if (h == NULL)
        {
          puts ("dlopen failed");
          exit (1);
        }

      tlsmod_fun = dlsym (h, "module_fun");
      if (tlsmod_fun == NULL)
        {
          puts ("dlsym for module_fun failed");
          exit (1);
        }

      count += tlsmod_fun ();

      /* Don't leave dangling pointer after dlclose.  */
      tlsmod_fun = NULL;

      if (dlclose (h))
        {
          puts ("dlclose failed");
          exit (1);
        }

    }

  size_t end_size = get_vm_size ();
  printf ("VM usage went from %ld to %ld pages", start_size, end_size);

  /* Allow for some growth in usage.  */
  if ((end_size - start_size) > 10)
    {
      printf (", which indicates a leak in TLS allocation\n");
      return 1;
    }
  else
    {
      printf (", which is acceptable\n");
      return 0;
    }
}

#define TIMEOUT 8

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
