#ifndef __itimerspec_defined
#define __itimerspec_defined 1

#include <bits/types.h>
#include <bits/types/struct_timespec.h>

/* Use the original definition for 64-bit arches
   or when 64-bit-time by default has *not* been requested */ 
#if __WORDSIZE > 32 || ! defined(__USE_TIME_BITS64)
/* POSIX.1b structure for timer start values and intervals.  */
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
#else
# define itimerspec __itimerspec64
#endif

#endif
