#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <internaltypes.h>
#include <support/check.h>


static int
do_test (void)
{
  union
  {
    sem_t s;
    struct new_sem ns;
  } u;

  TEST_COMPARE (sem_init (&u.s, 0, 0), 0);

  struct timespec ts = { 0, 1000000001 };	/* Invalid.  */
  errno = 0;
  TEST_VERIFY_EXIT (sem_timedwait (&u.s, &ts) < 0);
  TEST_COMPARE (errno, EINVAL);

#if __HAVE_64B_ATOMICS
  unsigned int nwaiters = (u.ns.data >> SEM_NWAITERS_SHIFT);
#else
  unsigned int nwaiters = u.ns.nwaiters;
#endif
  TEST_COMPARE (nwaiters, 0);

  ts.tv_sec = /* Invalid.  */ -2;
  ts.tv_nsec = 0;
  errno = 0;
  TEST_VERIFY_EXIT (sem_timedwait (&u.s, &ts) < 0);
  TEST_COMPARE (errno, ETIMEDOUT);
#if __HAVE_64B_ATOMICS
  nwaiters = (u.ns.data >> SEM_NWAITERS_SHIFT);
#else
  nwaiters = u.ns.nwaiters;
#endif
  TEST_COMPARE (nwaiters, 0);

  return 0;
}

#include <support/test-driver.c>
