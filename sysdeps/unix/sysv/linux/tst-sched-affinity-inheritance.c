/* CPU Affinity inheritance test - sched_{gs}etaffinity.
   Copyright The GNU Toolchain Authors.
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

/* See top level comment in nptl/tst-skeleton-affinity-inheritance.c for a
   description of this test.  */

#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <support/check.h>

static void
set_my_affinity (size_t size, const cpu_set_t *set)
{
  int ret = sched_setaffinity (0, size, set);

  if (ret != 0)
    FAIL ("sched_setaffinity returned %d (%s)", ret, strerror (ret));
}

static void
verify_my_affinity (int nproc, int nproc_configured, size_t size,
		    const cpu_set_t *expected_set)
{
  cpu_set_t *set = CPU_ALLOC (nproc_configured);
  cpu_set_t *xor_set = CPU_ALLOC (nproc_configured);

  if (set == NULL || xor_set== NULL)
    FAIL_EXIT1 ("verify_my_affinity: Failed to allocate cpuset: %m\n");

  int ret = sched_getaffinity (0, size, set);
  if (ret != 0)
    FAIL ("sched_getaffinity returned %d (%s)", ret, strerror (ret));

  CPU_XOR_S (size, xor_set, expected_set, set);

  int cpucount = CPU_COUNT_S (size, xor_set);

  if (cpucount > 0)
    {
      FAIL ("Affinity mask not inherited, "
	    "following %d CPUs mismatched in the expected and actual sets:\n",
	    cpucount);
      for (int cur = 0; cur < nproc && cpucount >= 0; cur++)
	if (CPU_ISSET_S (size, cur, xor_set))
	  {
	    printf ("%d ", cur);
	    cpucount--;
	  }
      printf ("\n");
    }

  CPU_FREE (set);
  CPU_FREE (xor_set);
}

#include <nptl/tst-skeleton-affinity-inheritance.c>
