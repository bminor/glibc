/* CPU Affinity inheritance test - common infrastructure.
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

/* The general idea of this test is to verify that the set of CPUs assigned to
   a task gets inherited by a child (thread or process) of that task.  This is
   a framework that is included by specific APIs for the test, e.g.
   sched_getaffinity/sched_setaffinity and
   pthread_setaffinity_np/pthread_getaffinity_np.  This is a framework, actual
   tests entry points are in nptl/tst-pthread-affinity-inheritance.c and
   sysdeps/unix/sysv/linux/tst-sched-affinity-inheritance.c.

   There are two levels to the test with two different CPU masks.  The first
   level verifies that the affinity set on the main process is inherited by its
   children subprocess or thread.  The second level verifies that a subprocess
   or subthread passes on its affinity to their respective subprocess or
   subthread.  We set a slightly different mask in both levels to ensure that
   they're both inherited.  */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <support/test-driver.h>
#include <support/xthread.h>
#include <support/xunistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>

struct test_param
{
  int nproc;
  int nproc_configured;
  cpu_set_t *set;
  size_t size;
  bool entry;
};

void __attribute__((noinline))
set_cpu_mask (struct test_param *param, bool entry)
{
  int cpus = param->nproc;

  /* Less CPUS for the first level, if that's possible.  */
  if (entry && cpus > 1)
    cpus--;

  CPU_ZERO_S (param->size, param->set);
  while (cpus > 0)
    CPU_SET_S (--cpus, param->size, param->set);

  if (CPU_COUNT_S (param->size, param->set) == 0)
    FAIL_EXIT1 ("Failed to add any CPUs to the affinity set\n");
}

static void *
child_test (void *arg)
{
  struct test_param *param = arg;

  printf ("%d:%d        child\n", getpid (), gettid ());
  verify_my_affinity (param->nproc, param->nproc_configured, param->size,
		      param->set);
  return NULL;
}

void *
do_one_test (void *arg)
{
  void *(*child) (void *) = NULL;
  struct test_param *param = arg;
  bool entry = param->entry;

  if (entry)
    {
      printf ("%d:%d Start test run\n", getpid (), gettid ());
      /* First level: Reenter as a subprocess and then as a subthread.  */
      child = do_one_test;
      set_cpu_mask (param, true);
      set_my_affinity (param->size, param->set);
      param->entry = false;
    }
  else
    {
      /* Verification for the first level.  */
      verify_my_affinity (param->nproc, param->nproc_configured, param->size,
			  param->set);

      /* Launch the second level test, launching CHILD_TEST as a subprocess and
	 then as a subthread.  Use a different mask to see if it gets
	 inherited.  */
      child = child_test;
      set_cpu_mask (param, false);
      set_my_affinity (param->size, param->set);
    }

  /* Verify that a child of a thread/process inherits the affinity mask.  */
  printf ("%d:%d%sdo_one_test: fork\n", getpid (), gettid (),
	  entry ? " " : "    ");
  int pid = xfork ();

  if (pid == 0)
    {
      child (param);
      return NULL;
    }

  xwaitpid (pid, NULL, 0);

  /* Verify that a subthread of a thread/process inherits the affinity
     mask.  */
  printf ("%d:%d%sdo_one_test: thread\n", getpid (), gettid (),
	  entry ? " " : "    ");
  pthread_t t = xpthread_create (NULL, child, param);
  xpthread_join (t);

  return NULL;
}

static int
do_test (void)
{
  /* Large enough in case the kernel decides to return the larger mask.  This
     seems to happen on some kernels for S390x.  */
  int num_configured_cpus = get_nprocs_conf ();
  int num_cpus = get_nprocs ();

  struct test_param param =
    {
      .nproc = num_cpus,
      .nproc_configured = num_configured_cpus,
      .set = CPU_ALLOC (num_configured_cpus),
      .size = CPU_ALLOC_SIZE (num_configured_cpus),
      .entry = true,
    };

  if (param.set == NULL)
    FAIL_EXIT1 ("error: CPU_ALLOC (%d) failed\n", num_cpus);

  do_one_test (&param);

  CPU_FREE (param.set);

  return 0;
}

#include <support/test-driver.c>
