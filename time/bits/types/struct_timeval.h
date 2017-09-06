#ifndef __timeval_defined
#define __timeval_defined 1

#include <bits/types.h>

/* Use the original definition for 64-bit arches
   or when 64-bit-time by default has *not* been requested */ 
# if __WORDSIZE > 32 || ! defined(__USE_TIME_BITS64)
/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval
{
  __time_t tv_sec;		/* Seconds.  */
  __suseconds_t tv_usec;	/* Microseconds.  */
};
# else
/* Use the 64-bit-time timespec by default */
# define timeval __timeval64
# endif

#endif
