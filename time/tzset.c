/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <localeinfo.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef	HAVE_GNU_LD
#define	__tzname	tzname
#define	__daylight	daylight
#define	__timezone	timezone
#endif

extern int __use_tzfile;
extern void EXFUN(__tzfile_read, (CONST char *file));
extern void EXFUN(__tzfile_default, (char *std AND char *dst AND
				     long int stdoff AND long int dstoff));
extern int EXFUN(__tzfile_compute, (time_t, struct tm));

char *__tzname[2] = { (char *) "GMT", (char *) "GMT" };
int __daylight = 0;
long int __timezone = 0L;


#define	min(a, b)	((a) < (b) ? (a) : (b))
#define	max(a, b)	((a) > (b) ? (a) : (b))
#define	sign(x)		((x) < 0 ? -1 : 1)


/* This structure contains all the information about a
   timezone given in the POSIX standard TZ envariable.  */
typedef struct
  {
    char *name;

    /* When to change.  */
    enum { J0, J1, M } type;	/* Interpretation of:  */
    unsigned short int m, n, d;	/* Month, week, day.  */
    unsigned int secs:17;	/* Time of day.  */

    long int offset;		/* Seconds east of GMT (west if < 0).  */

    /* We cache the computed time of change for a
       given year so we don't have to recompute it.  */
    time_t change;	/* When to change to this zone.  */
    int computed_for;	/* Year above is computed for.  */
  } tz_rule;

/* tz_rules[0] is standard, tz_rules[1] is daylight.  */
static tz_rule tz_rules[2];

static int tzset_run = 0;

/* Interpret the TZ envariable.  */
void
DEFUN_VOID(__tzset)
{
  register CONST char *tz;
  register size_t l;
  unsigned short int hh, mm, ss;
  unsigned short int whichrule;

  /* Free old storage.  */
  if (tz_rules[0].name != NULL && *tz_rules[0].name != '\0')
    free((PTR) tz_rules[0].name);
  if (tz_rules[1].name != NULL && *tz_rules[1].name != '\0' &&
      tz_rules[1].name != tz_rules[0].name)
    free((PTR) tz_rules[1].name);

  tz = getenv("TZ");

  if (tz != NULL && *tz == ':')
    {
      __tzfile_read(tz + 1);
      if (__use_tzfile)
	{
	  tzset_run = 1;
	  return;
	}
      else
	tz = NULL;
    }

  if (tz == NULL || *tz == '\0')
    tz = _time_info->tz;
  if (tz == NULL || *tz == '\0')
    {
      __tzfile_read((char *) NULL);
      if (!__use_tzfile)
	{
	  size_t len = strlen(_time_info->ut0) + 1;
	  tz_rules[0].name = (char *) malloc(len);
	  if (tz_rules[0].name == NULL)
	    return;
	  tz_rules[1].name = (char *) malloc(len);
	  if (tz_rules[1].name == NULL)
	    return;
	  memcpy((PTR) tz_rules[0].name, _time_info->ut0, len);
	  memcpy((PTR) tz_rules[1].name, _time_info->ut0, len);
	  tz_rules[0].type = tz_rules[1].type = J0;
	  tz_rules[0].m = tz_rules[0].n = tz_rules[0].d = 0;
	  tz_rules[1].m = tz_rules[1].n = tz_rules[1].d = 0;
	  tz_rules[0].secs = tz_rules[1].secs = 0;
	  tz_rules[0].offset = tz_rules[1].offset = 0L;
	  tz_rules[0].change = tz_rules[1].change = (time_t) -1;
	  tz_rules[0].computed_for = tz_rules[1].computed_for = 0;
	}
      tzset_run = 1;
      return;
    }

  /* Get the standard timezone name.  */
  tz_rules[0].name = (char *) malloc(strlen(tz) + 1);
  if (tz_rules[0].name == NULL)
    return;

  if (sscanf(tz, "%[^0-9,+-]", tz_rules[0].name) != 1 ||
      (l = strlen(tz_rules[0].name)) < 3)
    return;

  tz_rules[0].name = (char *) realloc((PTR) tz_rules[0].name, l + 1);
  if (tz_rules[0].name == NULL)
    return;

  tz += l;

  /* Figure out the standard offset from GMT.  */
  if (*tz == '\0' || (*tz != '+' && *tz != '-' && !isdigit(*tz)))
    return;

  if (*tz == '-' || *tz == '+')
    tz_rules[0].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[0].offset = -1L;
  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
    {
    default:
      return;
    case 1:
      mm = 0;
    case 2:
      ss = 0;
    case 3:
      break;
    }
  tz_rules[0].offset *= (min(ss, 59) + (min(mm, 59) * 60) +
			 (min(hh, 12) * 60 * 60));

  for (l = 0; l < 3; ++l)
    {
      while (isdigit(*tz))
	++tz;
      if (l < 2 && *tz == ':')
	++tz;
    }

  /* Get the DST timezone name (if any).  */
  if (*tz == '\0')
    tz_rules[1].name = "";
  else
    {
      tz_rules[1].name = (char *) malloc(strlen(tz) + 1);
      if (tz_rules[1].name == NULL)
	return;
      if (sscanf(tz, "%[^0-9,+-]", tz_rules[1].name) != 1 ||
	  (l = strlen(tz_rules[1].name)) < 3)
	return;
      tz_rules[1].name = (char *) realloc((PTR) tz_rules[1].name, l + 1);
      if (tz_rules[1].name == NULL)
	return;
    }

  tz += l;

  /* Figure out the DST offset from GMT.  */
  if (*tz == '-' || *tz == '+')
    tz_rules[1].offset = *tz++ == '-' ? 1L : -1L;
  else
    tz_rules[1].offset = -1L;

  switch (sscanf (tz, "%hu:%hu:%hu", &hh, &mm, &ss))
    {
    default:
      /* Default to one hour later than standard time.  */
      tz_rules[1].offset = tz_rules[0].offset + (60 * 60);
      break;

    case 1:
      mm = 0;
    case 2:
      ss = 0;
    case 3:
      tz_rules[1].offset *= (min(ss, 59) + (min(mm, 59) * 60) +
			     (min(hh, 12) * (60 * 60)));
      break;
    }
  for (l = 0; l < 3; ++l)
    {
      while (isdigit(*tz))
	++tz;
      if (l < 2 && *tz == ':')
	++tz;
    }

  /* If no standard or DST offset was given, default to GMT
     for standard and one hour later than standard for DST.  */
  if (*tz_rules[0].name == '\0')
    tz_rules[0].offset = 0L;
  if (*tz_rules[1].name == '\0')
    tz_rules[1].offset = tz_rules[0].offset + (60 * 60);

  if (*tz == '\0' || (tz[0] == ',' && tz[1] == '\0'))
    {
      /* There is no rule.  See if there is a default rule file.  */
      __tzfile_default (tz_rules[0].name, tz_rules[1].name,
			tz_rules[0].offset, tz_rules[1].offset);
      if (__use_tzfile)
	return;
    }

  /* Figure out the standard <-> DST rules.  */
  for (whichrule = 0; whichrule < 2; ++whichrule)
    {
      register tz_rule *tzr = &tz_rules[whichrule];
      
      if (*tz != '\0' && *tz == ',')
	{
	  ++tz;
	  if (*tz == '\0')
	    return;
	}
      
      /* Get the date of the change.  */
      if (*tz == 'J' || isdigit(*tz))
	{
	  char *end;
	  tzr->type = *tz == 'J' ? J1 : J0;
	  if (tzr->type == J1 && !isdigit(*++tz))
	    return;
	  tzr->n = (unsigned short int) strtoul(tz, &end, 10);
	  if (end == tz || tzr->n > 365)
	    return;
	  else if (tzr->type == J1 && tzr->n == 0)
	    return;
	  if (tzr->type == J1 && tzr->n == 60)
	    /* Can't do February 29.  */
	    ++tzr->n;
	  tz = end;
	}
      else if (*tz == 'M')
	{
	  int n;
	  tzr->type = M;
	  if (sscanf (tz, "M%hu.%hu.%hu%n",
		      &tzr->m, &tzr->n, &tzr->d, &n) != 3 ||
	      tzr->m < 1 || tzr->m > 12 ||
	      tzr->n < 1 || tzr->n > 5 || tzr->d > 6)
	    return;
	  tz += n;
	}
      else if (*tz == '\0')
	{
	  /* United States Federal Law, the equivalent of "M3.1.0,M8.5.0".  */
	  tzr->type = M;
	  if (tzr == &tz_rules[0])
	    {
	      tzr->m = 4;
	      tzr->n = 1;
	      tzr->d = 0;
	    }
	  else
	    {
	      tzr->m = 10;
	      tzr->n = 5;
	      tzr->d = 0;
	    }
	}
      else
	return;
      
      if (*tz != '\0' && *tz != '/' && *tz != ',')
	return;
      else if (*tz == '/')
	{
	  /* Get the time of day of the change.  */
	  ++tz;
	  if (*tz == '\0')
	    return;
	  switch (sscanf(tz, "%hu:%hu:%hu", &hh, &mm, &ss))
	    {
	    default:
	      return;
	    case 1:
	      mm = 0;
	    case 2:
	      ss = 0;
	    case 3:
	      break;
	    }
	  for (l = 0; l < 3; ++l)
	    {
	      while (isdigit(*tz))
		++tz;
	      if (l < 2 && *tz == ':')
		++tz;
	    }
	  tzr->secs = (hh * 60 * 60) + (mm * 60) + ss;
	}
      else
	/* Default to 2:00 AM.  */
	tzr->secs = 2 * 60 * 60;

      tzr->computed_for = -1;
    }

  tzset_run = 1;
}

/* Figure out the exact time (as a time_t) in YEAR
   when the change described by RULE will occur and
   put it in RULE->change, saving YEAR in RULE->computed_for.
   Return nonzero if successful, zero on failure.  */
static int
DEFUN(compute_change, (rule, year), tz_rule *rule AND int year)
{
  register unsigned short int m = rule->m, n = rule->n, d = rule->d;
  struct tm tbuf;
  register time_t t;

  if (year != -1 && rule->computed_for == year)
    /* Operations on times in 1969 will be slower.  Oh well.  */
    return 1;

  memset((PTR) &tbuf, 0, sizeof(tbuf));
  tbuf.tm_year = year;

  if (rule->type == M)
    {
      /* Defined in _offtime.c.  */
      extern CONST unsigned short int __mon_lengths[2][12];
      unsigned short int ml = __mon_lengths[__isleap(tbuf.tm_year)][m - 1];
      tbuf.tm_mon = m - 1;
      if (n == 5)
	tbuf.tm_mday = ml;
      else
	tbuf.tm_mday = max((n - 1) * 7, 1);
      tbuf.tm_sec = rule->secs;
      t = mktime(&tbuf);
      if (t == (time_t) -1)
	return 0;
      if (tbuf.tm_wday != d)
	{
	  if (d > tbuf.tm_wday)
	    {
	      tbuf.tm_mday -= 7;
	      tbuf.tm_mday += tbuf.tm_wday - d;
	    }
	  else
	    tbuf.tm_mday -= tbuf.tm_wday - d;
	  if (tbuf.tm_mday < 1)
	    tbuf.tm_mday += 7;
	  else
	    if (tbuf.tm_mday > ml) tbuf.tm_mday -= 7;
	  t = mktime(&tbuf);
	  if (t == (time_t) -1)
	    return 0;
	}
    }
  else
    {
      tbuf.tm_mon = 0;
      if (rule->type == J1)
	--n;
      tbuf.tm_mday = n;
      tbuf.tm_sec = rule->secs;
      t = mktime(&tbuf);
      if (t == (time_t) -1)
	return 0;
    }

  rule->change = t;
  rule->computed_for = year;
  return 1;
}


/* Figure out the correct timezone for *TIMER and TM (which must be the same)
   and set `__tzname', `__timezone', and `__daylight' accordingly.
   Return nonzero on success, zero on failure.  */
int
DEFUN(__tz_compute, (timer, tm),
      time_t timer AND struct tm tm)
{
  if (!tzset_run)
    __tzset();

  if (__use_tzfile)
    return __tzfile_compute(timer, tm);

  if (!compute_change(&tz_rules[0], tm.tm_year) ||
      !compute_change(&tz_rules[1], tm.tm_year))
    return 0;

  __daylight = timer >= tz_rules[0].change && timer < tz_rules[1].change;
  __timezone = tz_rules[__daylight ? 1 : 0].offset;
  __tzname[0] = (char *) tz_rules[0].name;
  __tzname[1] = (char *) tz_rules[1].name;

  return 1;
}


long int
DEFUN_VOID (__tzname_max)
{
  size_t len0 = strlen (__tzname[0]), len1 = strlen (__tzname[1]);
  return len0 > len1 ? len0 : len1;
}
