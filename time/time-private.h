/* Private declarations for the time subsystem.
   Copyright (C) 1991-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef TIME_PRIVATE_H
#define TIME_PRIVATE_H

#include <stdbool.h>
#include <sys/types.h>

struct ttinfo
{
  long int offset;            /* Seconds east of GMT.  */
  unsigned char isdst;        /* Used to set tm_isdst.  */
  unsigned char idx;          /* Index into `zone_names'.  */
  unsigned char isstd;        /* Transition times are in standard time.  */
  unsigned char isgmt;        /* Transition times are in GMT.  */
};

struct leap
{
  time_t transition;          /* Time the transition takes effect.  */
  long int change;            /* Seconds of correction to apply.  */
};

/* This structure contains all the information about a
   timezone given in the POSIX standard TZ envariable.  */
struct tz_rule
{
  const char *name;

  /* When to change.  */
  enum { J0, J1, M } type;      /* Interpretation of:  */
  unsigned short int m, n, d;   /* Month, week, day.  */
  int secs;                     /* Time of day.  */

  long int offset;              /* Seconds east of GMT (west if < 0).  */
};

struct tzdata
{
  /* Updated by tzfile.c.  */

  /* Used to recognize unchanged tzdata files.  */
  dev_t tzfile_dev;
  ino64_t tzfile_ino;
  time_t tzfile_mtime;

  size_t num_transitions;
  time_t *transitions;
  unsigned char *type_idxs;
  size_t num_types;
  struct ttinfo *types;
  char *zone_names;
  long int rule_stdoff;
  long int rule_dstoff;
  size_t num_leaps;
  struct leap *leaps;
  char *tzspec;
  bool use_tzfile;

  /* Updated by tzset.c.  */

  /* Used to recognize an unchanged TZ environment variable.  */
  char *old_tz;

  /* tz_rules[0] is standard, tz_rules[1] is daylight.  */
  struct tz_rule tz_rules[2];

  /* The remaining members correspond to the global variables of the
     same name (with a __ prefix within glibc).  */

  bool daylight;
  int timezone;
  char *tzname[2];
};

/* Determine the actual location of the timezone file based on FILE.
   Return a newly allocated string (which the caller should free), and
   NULL on memory allocation failure.  */
char *__tzfile_determine_path (const char *file)
  internal_function attribute_hidden;

/* Return true if the timezone data in *TZDATA is up-to-date with
   regards to the modification time of the file at PATH.  */
bool __tzfile_is_current (const char *path, const struct tzdata *tzdata)
  internal_function attribute_hidden __nonnull ((1, 2));

/* Load the timezone data specified by FILE in *TZDATA.  All errors
   result in a tzdata object for the UTC time zone without any leap
   seconds.  */
void __tzfile_read (struct tzdata *tzdata, const char *file,
                    size_t extra, char **extrap)
  internal_function attribute_hidden __nonnull ((1));

/* Deallocate the data in the struct tzdata object, but not the object
   itself.  */
void __tzdata_free (struct tzdata *tzdata)
  internal_function attribute_hidden __nonnull ((1));

/* The user specified a hand-made timezone, but not its DST rules.  We
   will use the names and offsets from the user, and the rules from
   the TZDEFRULES file.  */
void __tzfile_default (struct tzdata *tzdata,
                       const char *std, const char *dst,
                       long int stdoff, long int dstoff)
  internal_function attribute_hidden __nonnull ((1, 2, 3));

/* Parse the POSIX TZ-style string.  */
void __tzset_parse_tz (struct tzdata *tzdata, const char *tz)
  internal_function attribute_hidden __nonnull ((1, 2));

/* Figure out the correct timezone for TM and set TM->tm_zone,
   TM->tm_isdst, and TM->tm_gmtoff accordingly.  */
void __tz_compute (const struct tzdata *tzdata,
                   time_t timer, struct tm *tm, int use_localtime)
  internal_function attribute_hidden __nonnull ((1, 3));

/* Write the struct tm representation of *TIMER in the local timezone
   to *TP.  Use local time if USE_LOCALTIME, UTC otherwise.  If
   USE_REENTRANT, the global timezone variables are not updated.
   Return TP.  */
struct tm *__tz_convert (const time_t *timer, bool use_reentrant,
                         bool use_localtime, struct tm *tp)
  internal_function attribute_hidden __nonnull ((1, 4));

extern void __tzfile_compute (const struct tzdata *tzdata,
                              time_t timer, int use_localtime,
                              long int *leap_correct, int *leap_hit,
                              struct tm *tp);
  internal_function attribute_hidden __nonnull ((1, 4, 5, 6));


#endif /* TIME_PRIVATE_H */
