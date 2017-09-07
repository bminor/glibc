#ifndef __timespec64_defined
#define __timespec64_defined 1

#include <bits/types.h>
#include <bits/timesize.h>
#include <endian.h>

/* The glibc-internal Y2038-proof structure for a time value.
   To keep things Posix-ish, we keep the nanoseconds field a 32-bit
   signed long, but since the Linux field is a 64-bit signed int, we
   pad our tv_nsec with a 32-bit int, which should always be 0.
   Note that this is the glibc-internal type; in the public type, the
   padding is an anonymous 32-bit bitfield, so that source-code initializers
   keep working.  */
#if __TIMESIZE==64
# define __timespec64 timespec
#else
#define TIMSPEC64_TV_PAD_DEFINED
# if BYTE_ORDER == BIG_ENDIAN
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
};
# else
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
};
# endif
#endif

#endif
