/* Extensions for GNU date that are still missing here:
   -
   _
   e
*/

/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define	add(n, f)							      \
  do									      \
    {									      \
      i += (n);								      \
      if (i >= maxsize)							      \
	return 0;							      \
      else								      \
	if (p != NULL)							      \
	  {								      \
	    f;								      \
	    p += (n);							      \
	  }								      \
    } while (0)
#define	cpy(n, s)	add((n), memcpy((PTR) p, (PTR) (s), (n)))
#define	fmt(n, args)	add((n), if (sprintf args != (n)) return 0)

/* Return the week in the year specified by TP,
   with weeks starting on STARTING_DAY.  */
#ifdef	__GNUC__
inline
#endif
static unsigned int
DEFUN(week, (tp, starting_day),
      CONST struct tm *CONST tp AND int starting_day)
{
  int wday, dl;

  wday = tp->tm_wday - starting_day;
  if (wday < 0)
    wday += 7;

  /* Set DL to the day in the year of the last day of the week previous to the
     one containing the day specified in TP.  If DL is negative or zero, the
     day specified in TP is in the first week of the year.  Otherwise,
     calculate the number of complete weeks before our week (DL / 7) and
     add any partial week at the start of the year (DL % 7).  */
  dl = tp->tm_yday - wday;
  return dl <= 0 ? 0 : ((dl / 7) + ((dl % 7) == 0 ? 0 : 1));
}


/* Write information from TP into S according to the format
   string FORMAT, writing no more that MAXSIZE characters
   (including the terminating '\0') and returning number of
   characters written.  If S is NULL, nothing will be written
   anywhere, so to determine how many characters would be
   written, use NULL for S and (size_t) UINT_MAX for MAXSIZE.  */
size_t
DEFUN(strftime, (s, maxsize, format, tp),
      char *s AND size_t maxsize AND
      CONST char *format AND register CONST struct tm *tp)
{
  CONST char *CONST a_wkday = _time_info->abbrev_wkday[tp->tm_wday];
  CONST char *CONST f_wkday = _time_info->full_wkday[tp->tm_wday];
  CONST char *CONST a_month = _time_info->abbrev_month[tp->tm_mon];
  CONST char *CONST f_month = _time_info->full_month[tp->tm_mon];
  size_t aw_len = strlen(a_wkday);
  size_t am_len = strlen(a_month);
  size_t wkday_len = strlen(f_wkday);
  size_t month_len = strlen(f_month);
  int hour12 = tp->tm_hour;
  CONST char *CONST ampm = _time_info->ampm[hour12 >= 12];
  size_t ap_len = strlen(ampm);
  CONST unsigned int y_week0 = week(tp, 0);
  CONST unsigned int y_week1 = week(tp, 1);
  CONST char *zone;
  size_t zonelen;
  register size_t i = 0;
  register char *p = s;
  register CONST char *f;

  if (tp->tm_isdst < 0)
    {
      zone = "";
      zonelen = 0;
    }
  else
    {
      zone = __tzname[tp->tm_isdst];
      zonelen = strlen(zone);
    }

  if (hour12 > 12)
    hour12 -= 12;
  else
    if (hour12 == 0) hour12 = 12;

  for (f = format; *f != '\0'; ++f)
    {
      CONST char *subfmt;

      if (!isascii(*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen(f, strlen(f));
	  if (len > 0)
	    {
	      cpy(len, f);
	      continue;
	    }
	}

      if (*f != '%')
	{
	  add(1, *p = *f);
	  continue;
	}

      ++f;
      switch (*f)
	{
	case '\0':
	case '%':
	  add(1, *p = *f);
	  break;
	case 'n':		/* GNU extension.  */
	  add (1, *p = '\n');
	  break;
	case 't':		/* GNU extenstion.  */
	  add (1, *p = '\t');
	  break;

	case 'a':
	  cpy(aw_len, a_wkday);
	  break;
	case 'A':
	  cpy(wkday_len, f_wkday);
	  break;
	case 'b':
	  cpy(am_len, a_month);
	  break;
	case 'B':
	  cpy(month_len, f_month);
	  break;
	case 'c':
	  subfmt = _time_info->date_time;
	subformat:;
	  {
	    size_t len = strftime(p, maxsize - i, subfmt, tp);
	    add(len, );
	  }
	  break;
	case 'd':
	  fmt(2, (p, "%.2d", tp->tm_mday));
	  break;
	case 'H':
	  fmt(2, (p, "%.2d", tp->tm_hour));
	  break;
	case 'I':
	  fmt(2, (p, "%.2d", hour12));
	  break;
	case 'j':
	  fmt(3, (p, "%.3d", tp->tm_yday));
	  break;
	case 'm':
	  fmt(2, (p, "%.2d", tp->tm_mon + 1));
	  break;
	case 'M':
	  fmt(2, (p, "%.2d", tp->tm_min));
	  break;
	case 'p':
	  cpy(ap_len, ampm);
	  break;
	case 'S':
	  fmt(2, (p, "%.2d", tp->tm_sec));
	  break;
	case 'U':
	  fmt(2, (p, "%.2u", y_week0));
	  break;
	case 'w':
	  fmt(2, (p, "%.2d", tp->tm_wday));
	  break;
	case 'W':
	  fmt(2, (p, "%.2u", y_week1));
	  break;
	case 'x':
	  subfmt = _time_info->date;
	  goto subformat;
	case 'X':
	  subfmt = _time_info->time;
	  goto subformat;
	case 'y':
	  fmt(2, (p, "%.2d", tp->tm_year));
	  break;
	case 'Y':
	  fmt(4, (p, "%.4d", 1900 + tp->tm_year));
	  break;
	case 'Z':
	  cpy(zonelen, zone);
	  break;

	  /* GNU extensions.  */
	case 'r':
	  subfmt = "%I:%M:%S %p";
	  goto subformat;
	case 'R':
	  subfmt = "%H:%M";
	  goto subformat;
	case 'T':
	  subfmt = "%H:%M:%S";
	  goto subformat;
	case 'D':
	  subfmt = "%m/%d/%y";
	  goto subformat;

	case 'e':		/* %d, but blank-padded.  */
	  /* XXX */
	  break;

	default:
	  /* Bad format.  */
	  break;
	}
    }

  if (p != NULL)
    *p = '\0';
  return(i);
}
