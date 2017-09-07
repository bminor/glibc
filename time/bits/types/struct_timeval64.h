#ifndef __timeval64_defined
#define __timeval64_defined 1

#include <bits/types.h>

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct __timeval64
{
  __time64_t tv_sec;		/* Seconds */
  __int64_t tv_usec;		/* Microseconds */
};
#endif
