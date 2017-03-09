#ifndef _TIME_H
#include <time/time.h>

#ifndef _ISOMAC
# include <xlocale.h>
# include <stdbool.h>

__BEGIN_DECLS

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

extern __typeof (clock_getres) __clock_getres;
extern __typeof (clock_gettime) __clock_gettime;
libc_hidden_proto (__clock_gettime)
extern __typeof (clock_settime) __clock_settime;
extern __typeof (clock_nanosleep) __clock_nanosleep;
extern __typeof (clock_getcpuclockid) __clock_getcpuclockid;

/* Now define the internal interfaces.  */
struct tm;

/* Defined in mktime.c.  */
extern const unsigned short int __mon_yday[2][13] attribute_hidden;

bool __time_isleap (int year) internal_function;
libc_hidden_proto (__time_isleap)

/* Defined in tzset.c.  */
extern char *__tzstring (const char *string);

/* Subroutine of `mktime'.  Return the `time_t' representation of TP and
   normalize TP, given that a `struct tm *' maps to a `time_t' as performed
   by FUNC.  Keep track of next guess for time_t offset in *OFFSET.  */
extern time_t __mktime_internal (struct tm *__tp,
				 struct tm *(*__func) (const time_t *,
						       struct tm *),
				 time_t *__offset);
extern struct tm *__localtime_r (const time_t *__timer,
				 struct tm *__tp) attribute_hidden;

extern struct tm *__gmtime_r (const time_t *__restrict __timer,
			      struct tm *__restrict __tp);
libc_hidden_proto (__gmtime_r)

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
extern int __offtime (const time_t *__timer,
		      long int __offset,
		      struct tm *__tp);

extern char *__asctime_r (const struct tm *__tp, char *__buf);
extern void __tzset (void);

extern int __nanosleep (const struct timespec *__requested_time,
			struct timespec *__remaining);
libc_hidden_proto (__nanosleep)
extern int __nanosleep_nocancel (const struct timespec *__requested_time,
				 struct timespec *__remaining)
  attribute_hidden;
extern int __getdate_r (const char *__string, struct tm *__resbufp);


/* Determine CLK_TCK value.  */
extern int __getclktck (void);


/* strptime support.  */
extern char * __strptime_internal (const char *rp, const char *fmt,
				   struct tm *tm, void *statep,
				   __locale_t locparam)
     internal_function;

extern double __difftime (time_t time1, time_t time0);


/* Use in the clock_* functions.  Size of the field representing the
   actual clock ID.  */
#define CLOCK_IDFIELD_SIZE	3

__END_DECLS

#endif
#endif
