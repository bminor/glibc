/* mktime.c */

/* Copyright (C) 1993 Free Software Foundation, Inc.

This file is part of the GNU Accounting Utilities

The GNU Accounting Utilities are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
2, or (at your option) any later version.

The GNU Accounting Utilities are distributed in the hope that they will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU Accounting Utilities; see the file COPYING.  If
not, write to the Free Software Foundation, 675 Mass Ave, Cambridge,
MA 02139, USA.  */

/* provided for those systems that don't have their own.
 * 
 * after testing this, the maximum number of iterations that I had on
 * any number that I tried was 3!  Not bad.
 *
 * -----------------------THE-LOWDOWN-ON-MKTIME-----------------------------
 *
 * time_t mktime (timeptr)
 * struct tm *timeptr;
 *
 * --- the struct tm from the time.h header file
 *
 *     struct tm {
 *             int tm_sec;     seconds (0 - 59)
 *             int tm_min;     minutes (0 - 59)
 *             int tm_hour;    hours (0 - 23)
 *             int tm_mday;    day of month (1 - 31)
 *             int tm_mon;     month of year (0 - 11)
 *             int tm_year;    year - 1900
 *             int tm_wday;    day of week (Sunday = 0)
 *             int tm_yday;    day of year (0 - 365)
 *             int tm_isdst;   daylight savings time flag
 *             long tm_gmtoff; offset from GMT in seconds
 *             char *tm_zone;  abbreviation of timezone name
 *     };
 *
 *     tm_isdst is nonzero if DST is in effect.
 *
 *     tm_gmtoff is the offset (in seconds) of the time represented from GMT,
 *     with positive values indicating East of Greenwich.
 *
 * --- description
 *
 * mktime converts a struct tm (broken-down local time) into a time_t
 * -- it it is the opposite of localtime(3).  It is possible to put
 * the following values out of range and have mktime compensate:
 * tm_sec, tm_min, tm_hour, tm_mday, tm_year.  The other values in the
 * structure are ignored.
 *
 * The manual claims that a negative value for tm_isdst makes mktime
 * determine whether daylight savings time was in effect for the
 * specified time, but through observation this does not seem to be
 * the case.  mktime always returns the correct DST flag.
 *
 * The manual page ALSO claims that if a calendar time cannot be
 * represented, it will return -1.  Not so.  I've tried the entire
 * range of time_t (longs) on several systems and never had -1
 * returned, save for Dec 31, 1969, 23:59:59... */

#include "config.h"

#ifndef HAVE_MKTIME

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

/* #define DEBUG */    /* define this to have a standalone shell to test
                        * this implementation of mktime
                        */

static char rcsid[] = "$Id$";

static int times_through_search; /* this library routine should never
				    hang -- make sure we always return
				    when we're searching for a value */

#ifdef DEBUG

int debugging_enabled = 0;

/* print the values in a tm structure */
void
printtm (struct tm *it)
{
  printf ("%d/%d/%d %d:%d:%d (%s) yday:%d f:%d o:%ld",
	  it->tm_mon,
	  it->tm_mday,
	  it->tm_year,
	  it->tm_hour,
	  it->tm_min,
	  it->tm_sec,
	  it->tm_zone,
	  it->tm_yday,
	  it->tm_isdst,
	  it->tm_gmtoff);
}
#endif


/* return 1 if the YR is a leap year
 *
 * actually, it's a Steve Tibbets album
 */
static
int
leap_year (int yr)
{
  int year = yr + 1900;
  
  if ((year % 400) == 0)
    return 1;
  else if (((year % 4) == 0) && ((year % 100) != 0))
    return 1;
  else
    return 0;
}


/* not pretty, but it's fast */

static
time_t
olddist_tm (struct tm *t1, struct tm *t2)
{
  time_t distance = 0;		/* linear distance in seconds */
  int diff_flag = 0;

#define doit(x,secs); \
  if (t1->x != t2->x) diff_flag = 1; \
  distance -= (t2->x - t1->x) * secs;
  
  doit (tm_year, 31536000);
  doit (tm_yday, 86400);
  doit (tm_hour, 3600);
  doit (tm_min, 60);
  doit (tm_sec, 1);
  
#undef doit
  
  /* we need this DIFF_FLAG business because it is forseeable that
   * the distance may be zero when, in actuality, the two structures
   * are different.  This is usually the case when the dates are
   * 366 days apart and one of the years is a leap year */

  if ((distance == 0) && diff_flag)
    distance += 86400;

  return distance;
}
      

static
time_t
dist_tm (struct tm *t1, struct tm *t2)
{
  time_t distance = 0;
  unsigned long v1, v2;
  int diff_flag = 0;

  v1 = v2 = 0;

#define doit(x,secs); \
  v1 += t1->x * secs; \
  v2 += t2->x * secs; \
  if (!diff_flag) \
  { \
    if (t1->x < t2->x) \
      { \
	diff_flag = -1; \
      } \
    else if (t1->x > t2->x) \
      { \
	diff_flag = 1; \
      } \
  }
  
  doit (tm_year, 31536000);	/* okay, not all years have 365 days */
  doit (tm_mon, 2592000);	/* okay, not all months have 30 days */
  doit (tm_mday, 86400);
  doit (tm_hour, 3600);
  doit (tm_min, 60);
  doit (tm_sec, 1);
  
#undef doit
  
  /* we should also make sure that the sign of DISTANCE is correct --
   * if DIFF_FLAG is positive, the distance should be positive and
   * vice versa. */
  
  distance = (v1 > v2) ? (v1 - v2) : (v2 - v1);
  if (diff_flag < 0)
    distance = -distance;

  if (times_through_search > 20) /* arbitrary # of calls, but makes
				    sure we never hang if there's a
				    problem with this algorithm */
    {
      distance = diff_flag;
    }

  /* we need this DIFF_FLAG business because it is forseeable that
   * the distance may be zero when, in actuality, the two structures
   * are different.  This is usually the case when the dates are
   * 366 days apart and one of the years is a leap year */

  if ((distance == 0) && diff_flag) distance = 86400 * diff_flag;

  return distance;
}
      

/* modified b-search -- make intelligent guesses as to where the time
 * might lie along the timeline, assuming that our target time lies a
 * linear distance (w/o considering time jumps of a particular region)
 *
 * assume that time does not fluctuate at all along the timeline --
 * e.g., assume that a day will always take 86400 seconds, etc. -- and
 * come up with a hypothetical value for the time_t representation of
 * the struct tm TARGET, in relation to the guess variable -- it should
 * be pretty close! */

static
time_t
search (struct tm *target)
{
  struct tm *guess_tm;
  time_t guess = 0;
  time_t distance;

  times_through_search = 0;

  for (;;)
    {
      guess_tm = localtime (&guess);
      
#ifdef DEBUG
      if (debugging_enabled)
	{
	  printf ("guess %d == ", guess);
	  printtm (guess_tm);
	  printf ("\n");
	}
#endif
      
      /* are we on the money? */
      distance = dist_tm (target, guess_tm);
      
      if (distance == 0)
	{
	  /* yes, we got it!  Get out of here! */
	  return guess;
	}

      guess += distance;

      times_through_search++;
    }
}

/* since this function will call localtime many times (and the user might
 * be passing their struct tm * right from localtime, let's make a copy
 * for ourselves and run the search on the copy.
 *
 * also, we have to normalize the timeptr because it's possible to call mktime
 * with values that are out of range for a specific item (like 30th Feb)...
 */
#ifdef DEBUG
time_t
my_mktime (struct tm *timeptr)
#else
time_t
mktime (struct tm *timeptr)
#endif
{
  struct tm private_mktime_struct_tm; /* yes, users can get a ptr to this */
  struct tm *me;
  time_t result;

  int month_data[2][12] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
  };

  me = &private_mktime_struct_tm;
  
  memcpy ((void *) me, (void *) timeptr, sizeof (struct tm));

#define normalize(foo,x,y,bar); \
  while (me->foo < x) \
    { \
      me->bar--; \
      me->foo = (y - (x - me->foo)); \
    } \
  while (me->foo > y) \
    { \
      me->bar++; \
      me->foo = (x + (me->foo - y)); \
    }
  
  normalize (tm_sec,0,59,tm_min);
  normalize (tm_min,0,59,tm_hour);
  normalize (tm_hour,0,23,tm_mday);
  
  /* do month first, so day range can be found */
  normalize (tm_mon,0,11,tm_year);
  normalize (tm_mday,1,month_data[leap_year (me->tm_year)][me->tm_mon],tm_mon);

  /* do month again, because day may have pushed it out of range */
  normalize (tm_mon,0,11,tm_year);

  /* do day again, because month may have changed the range */
  normalize (tm_mday,1,month_data[leap_year (me->tm_year)][me->tm_mon],tm_mon);
  
#ifdef DEBUG
  if (debugging_enabled)
    {
      printf ("After normalizing: ");
      printtm (me);
      printf ("\n\n");
    }
#endif

  result = search (me);

  return result;
}


#ifdef DEBUG
void
main (int argc, char *argv[])
{
  int time;
  int result_time;
  struct tm *tmptr;
  
  if (argc == 1)
    {
      long q;
      
      printf ("starting long test...\n");

      for (q = 10000000; q < 1000000000; q++)
	{
	  struct tm *tm = localtime (&q);
	  if ((q % 10000) == 0) { printf ("%ld\n", q); fflush (stdout); }
	  if (q != my_mktime (tm))
	    { printf ("failed for %ld\n", q); fflush (stdout); }
	}
      
      printf ("test finished\n");

      exit (0);
    }
  
  if (argc != 2)
    {
      printf ("wrong # of args\n");
      exit (0);
    }
  
  debugging_enabled = 1;	/* we want to see the info */

  ++argv;
  time = atoi (*argv);
  
  printf ("Time: %d %s\n", time, ctime (&time));

  tmptr = localtime (&time);
  printf ("localtime returns: ");
  printtm (tmptr);
  printf ("\n");
#ifdef HAVE_MKTIME
  printf ("system mktime: %d\n\n", mktime (tmptr));
#endif
  printf ("  my mktime(): %d\n\n", my_mktime (tmptr));

  tmptr->tm_sec -= 20;
  tmptr->tm_min -= 20;
  tmptr->tm_hour -= 20;
  tmptr->tm_mday -= 20;
  tmptr->tm_mon -= 20;
  tmptr->tm_year -= 20;
  tmptr->tm_gmtoff -= 20000;	/* this has no effect! */
  tmptr->tm_zone = NULL;	/* nor does this! */
  tmptr->tm_isdst = -1;

  printf ("changed ranges: ");
  printtm (tmptr);
  printf ("\n\n");

  result_time = my_mktime (tmptr);
  printf ("\n  mine: %d\n", result_time);
#ifdef HAVE_MKTIME
  printf ("system: %d\n", mktime (tmptr));
#endif
}
#endif /* DEBUG */

#endif /* HAVE_MKTIME */

