#ifndef _TIME_H
#include <time/time.h>

#ifndef _ISOMAC
# include <bits/types/locale_t.h>
# include <bits/types/struct___timespec64.h>
# include <stdbool.h>

extern __typeof (strftime_l) __strftime_l;
libc_hidden_proto (__strftime_l)
extern __typeof (strptime_l) __strptime_l;

libc_hidden_proto (time)
libc_hidden_proto (asctime)
libc_hidden_proto (mktime)
libc_hidden_proto (timelocal)
libc_hidden_proto (localtime)
libc_hidden_proto (strftime)
libc_hidden_proto (strptime)

#if __TIMESIZE == 64
# define __timegm64 timegm
# define __mktime64 mktime
# define __timelocal64 timelocal
#else
extern __time64_t __timegm64 (struct tm *__tp) __THROW;
extern __time64_t __mktime64 (struct tm *__tp) __THROW;
/* Another name for `__mktime64'.  */
extern __time64_t __timelocal64 (struct tm *__tp) __THROW;

libc_hidden_proto (__mktime64)
libc_hidden_proto (__timelocal64)
#endif


extern __typeof (clock_getres) __clock_getres;
extern __typeof (clock_gettime) __clock_gettime;
libc_hidden_proto (__clock_gettime)
extern __typeof (clock_settime) __clock_settime;
extern __typeof (clock_nanosleep) __clock_nanosleep;
extern __typeof (clock_getcpuclockid) __clock_getcpuclockid;

extern int __clock_gettime64 (clockid_t __clock_id,
			      struct __timespec64 *__tp) __THROW;
extern int __clock_settime64 (clockid_t __clock_id,
			       const struct __timespec64 *__tp) __THROW;
extern int __clock_getres_time64 (clockid_t __clock_id,
				  struct __timespec64 *__res) __THROW;
extern int __clock_nanosleep64 (clockid_t __clock_id, int __flags,
				const struct __timespec64 *__req,
				struct __timespec64 *__rem);

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13] attribute_hidden;

/* Defined in localtime.c.  */
extern struct tm _tmbuf attribute_hidden;

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string) attribute_hidden;

extern int __use_tzfile attribute_hidden;

extern void __tzfile_read (const char *file, size_t extra,
			   char **extrap) attribute_hidden;
extern void __tzfile_compute (__time64_t timer, int use_localtime,
			      long int *leap_correct, int *leap_hit,
			      struct tm *tp) attribute_hidden;
extern void __tzfile_default (const char *std, const char *dst,
			      long int stdoff, long int dstoff)
  attribute_hidden;
extern void __tzset_parse_tz (const char *tz) attribute_hidden;
extern void __tz_compute (__time64_t timer, struct tm *tm, int use_localtime)
  __THROW attribute_hidden;

/* Subroutine of mktime.  Return the __time64_t representation of TP and
   normalize TP, given that a struct tm * maps to a __time64_t as performed
   by FUNC.  Record next guess for localtime-gmtime offset in *OFFSET.  */
extern __time64_t __mktime_internal (struct tm *__tp,
				     struct tm *(*__func) (const __time64_t *,
							   struct tm *),
				     long int *__offset) attribute_hidden;

/* nis/nis_print.c needs ctime, so even if ctime is not declared here,
   we define __ctime64 as ctime so that nis/nis_print.c can get linked
   against a function called ctime. */
#if __TIMESIZE == 64
# define __ctime64 ctime
#endif

#if __TIMESIZE == 64
# define __ctime64_r ctime_r
#endif

#if __TIMESIZE == 64
# define __localtime64 localtime
#else
extern struct tm *__localtime64 (const __time64_t *__timer);
libc_hidden_proto (__localtime64)
#endif

extern struct tm *__localtime_r (const time_t *__timer,
				 struct tm *__tp) attribute_hidden;

#if __TIMESIZE == 64
# define __localtime64_r __localtime_r
#else
extern struct tm *__localtime64_r (const __time64_t *__timer,
				   struct tm *__tp) attribute_hidden;
#endif

extern struct tm *__gmtime_r (const time_t *__restrict __timer,
			      struct tm *__restrict __tp);
libc_hidden_proto (__gmtime_r)

#if __TIMESIZE == 64
# define __gmtime64 gmtime
# define __gmtime64_r __gmtime_r
#else
extern struct tm *__gmtime64_r (const __time64_t *__restrict __timer,
			        struct tm *__restrict __tp);
#endif

/* Compute the `struct tm' representation of T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime (__time64_t __timer,
		      long int __offset,
		      struct tm *__tp) attribute_hidden;

extern char *__asctime_r (const struct tm *__tp, char *__buf)
  attribute_hidden;
extern void __tzset (void) attribute_hidden;

/* Prototype for the internal function to get information based on TZ.  */
extern struct tm *__tz_convert (__time64_t timer, int use_localtime,
			        struct tm *tp) attribute_hidden;

extern int __nanosleep (const struct timespec *__requested_time,
			struct timespec *__remaining);
hidden_proto (__nanosleep)
extern int __getdate_r (const char *__string, struct tm *__resbufp)
  attribute_hidden;


/* Determine CLK_TCK value.  */
extern int __getclktck (void) attribute_hidden;


/* strptime support.  */
extern char * __strptime_internal (const char *rp, const char *fmt,
				   struct tm *tm, void *statep,
				   locale_t locparam) attribute_hidden;

#if __TIMESIZE == 64
# define __difftime64 __difftime
#else
extern double __difftime64 (__time64_t time1, __time64_t time0);
#endif

extern double __difftime (time_t time1, time_t time0);


/* Use in the clock_* functions.  Size of the field representing the
   actual clock ID.  */
#define CLOCK_IDFIELD_SIZE	3

/* Check whether a time64_t value fits in a time_t.  */
static inline bool
fits_in_time_t (__time64_t t64)
{
  time_t t = t64;
  return t == t64;
}

/* Convert a known valid struct timeval into a struct __timespec64.  */
static inline void
valid_timeval_to_timespec64 (const struct timeval *tv32,
			     struct __timespec64 *ts64)
{
  ts64->tv_sec = tv32->tv_sec;
  ts64->tv_nsec = tv32->tv_usec * 1000;
}

/* Convert a known valid struct timespec into a struct __timespec64.  */
static inline void
valid_timespec_to_timespec64 (const struct timespec *ts32,
			      struct __timespec64 *ts64)
{
  ts64->tv_sec = ts32->tv_sec;
  ts64->tv_nsec = ts32->tv_nsec;
  /* We only need to zero ts64->tv_pad if we pass it to the kernel.  */
}

/* Convert a known valid struct __timespec64 into a struct timespec.  */
static inline void
valid_timespec64_to_timespec (const struct __timespec64 *ts64,
			      struct timespec *ts32)
{
  ts32->tv_sec = (time_t) ts64->tv_sec;
  ts32->tv_nsec = ts64->tv_nsec;
}

/* Convert a known valid struct __timespec64 into a struct timeval.  */
static inline void
valid_timespec64_to_timeval (const struct __timespec64 *ts64,
			     struct timeval *tv32)
{
  tv32->tv_sec = (time_t) ts64->tv_sec;
  tv32->tv_usec = ts64->tv_nsec / 1000;
}

/* Check if a value lies with the valid nanoseconds range.  */
#define IS_VALID_NANOSECONDS(ns) (ns >= 0 && ns <= 999999999)

/* Check and convert a struct timespec into a struct __timespec64.  */
static inline bool
timespec_to_timespec64 (const struct timespec *ts32,
					   struct __timespec64 *ts64)
{
  /* Check that ts32 holds a valid count of nanoseconds.  */
  if (! IS_VALID_NANOSECONDS (ts32->tv_nsec))
    return false;
  /* All ts32 fields can fit in ts64, so copy them.  */
  valid_timespec_to_timespec64 (ts32, ts64);
  /* We only need to zero ts64->tv_pad if we pass it to the kernel.  */
  return true;
}

/* Check and convert a struct __timespec64 into a struct timespec.  */
static inline bool
timespec64_to_timespec (const struct __timespec64 *ts64,
					   struct timespec *ts32)
{
  /* Check that tv_nsec holds a valid count of nanoseconds.  */
  if (! IS_VALID_NANOSECONDS (ts64->tv_nsec))
    return false;
  /* Check that tv_sec can fit in a __time_t.  */
  if (! fits_in_time_t (ts64->tv_sec))
    return false;
  /* All ts64 fields can fit in ts32, so copy them.  */
  valid_timespec64_to_timespec (ts64, ts32);
  return true;
}

/* Check and convert a struct __timespec64 into a struct timeval.  */
static inline bool
timespec64_to_timeval (const struct __timespec64 *ts64,
					  struct timeval *tv32)
{
  /* Check that tv_nsec holds a valid count of nanoseconds.  */
  if (! IS_VALID_NANOSECONDS (ts64->tv_nsec))
    return false;
  /* Check that tv_sec can fit in a __time_t.  */
  if (! fits_in_time_t (ts64->tv_sec))
    return false;
  /* All ts64 fields can fit in tv32, so copy them.  */
  valid_timespec64_to_timeval (ts64, tv32);
  return true;
}
#endif
#endif
