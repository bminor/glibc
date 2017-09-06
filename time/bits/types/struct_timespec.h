/* NB: Include guard matches what <linux/time.h> uses.  */
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1

#include <bits/types.h>

/* Use the original definition for 64-bit arches
   or when 64-bit-time by default has *not* been requested */ 
#if __WORDSIZE > 32 || ! defined(__USE_TIME_BITS64)
/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
{
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};
#else
/* Use the 64-bit-time timespec by default */
#define timespec __timespec64
# endif

#endif
