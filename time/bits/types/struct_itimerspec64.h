#ifndef __itimerspec64_defined
#define __itimerspec64_defined 1

#include <bits/types.h>
#include <bits/types/struct_timespec.h>
#include <bits/types/struct_timespec64.h>

/* POSIX.1b structure for timer start values and intervals.  */
struct __itimerspec64
{
  struct __timespec64 it_interval;
  struct __timespec64 it_value;
};

#endif
