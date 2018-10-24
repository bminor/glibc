#include <time.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  struct tm tm = { .tm_year = INT_MIN, .tm_mon = INT_MIN, .tm_mday = INT_MIN,
		    .tm_hour = INT_MIN, .tm_min = INT_MIN, .tm_sec = INT_MIN };
  errno = 0;
  time_t tt = mktime (&tm);
  if (tt != -1)
    {
      printf ("mktime() should have returned -1, returned %ld\n", (long int) tt);
      return 1;
    }
  if (errno != EOVERFLOW)
    {
      printf ("mktime() returned -1, errno should be %d (EOVERFLOW) but is %d (%s)\n", EOVERFLOW, errno, strerror(errno));
      return 1;
    }
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
