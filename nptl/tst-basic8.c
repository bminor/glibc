#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
/* Test that stack guards do not become resident, even with
mlockall(MCL_FUTURE) by verifying that spawning NTHREADS with these
stack/guard sizes use a reasonable (<100kb/thread) amount of RSS. */
#define NTHREADS 100
#define STACKSIZE (64 * 1024)
#define GUARDSIZE (1024 * 1024)

pthread_barrier_t barrier;


static void fail (const char *msg)
{
  puts (msg);
  exit (1);
}

static size_t get_rss_bytes (void)
{
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage) != 0)
    fail ("getrusage failed");

  return usage.ru_maxrss * 1024;
}

static void *
child (void *arg)
{
  int ret = pthread_barrier_wait(&barrier);
  if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    fail("pthread_barrier_wait failed");
  while (1)
    {
      sleep (10);
    }
  return NULL;
}

static int
do_test (void)
{
  int err;
  int i;
  size_t bytes;
  char buffer[2048];
  size_t overhead, count;
  int fd;
  mlockall(MCL_CURRENT);
  overhead = get_rss_bytes();
  mlockall(MCL_CURRENT|MCL_FUTURE);
  if (pthread_barrier_init(&barrier, NULL, NTHREADS + 1) != 0)
    fail("pthread_barrier_init failed");
  for (i = 0; i < NTHREADS; ++i)
    {
      pthread_attr_t attr;
      pthread_t tid;
      pthread_attr_init (&attr);
      pthread_attr_setstacksize (&attr, STACKSIZE + GUARDSIZE);
      pthread_attr_setguardsize (&attr, GUARDSIZE);
      err = pthread_create (&tid, &attr, child, NULL);
      if (err != 0)
        fail("pthread_create failed");
    }
  err = pthread_barrier_wait(&barrier);
  if (err != 0 && err != PTHREAD_BARRIER_SERIAL_THREAD)
    fail("pthread_barrier_wait failed");
  bytes = get_rss_bytes ();
  fd = open("/proc/self/smaps", O_RDONLY);
  while(0 < (count = read(fd, buffer, 2048)))
    write(1, buffer, count);
  close(fd);
  printf ("%d threads with %d stacks and %d guards using %zu per thread\n",
          NTHREADS, STACKSIZE, GUARDSIZE, (bytes-overhead)/NTHREADS);
  if ((bytes-overhead) > (NTHREADS * GUARDSIZE))
    fail ("memory usage too high");

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
