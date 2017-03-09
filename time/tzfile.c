/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
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

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>

#include <time/time-private.h>
#include <timezone/tzfile.h>

#include <endian.h>
#include <byteswap.h>

/* Decode the four bytes at PTR as a signed integer in network byte order.  */
static inline int
__attribute ((always_inline))
decode (const void *ptr)
{
  if (BYTE_ORDER == BIG_ENDIAN && sizeof (int) == 4)
    return *(const int *) ptr;
  if (sizeof (int) == 4)
    return bswap_32 (*(const int *) ptr);

  const unsigned char *p = ptr;
  int result = *p & (1 << (CHAR_BIT - 1)) ? ~0 : 0;

  result = (result << 8) | *p++;
  result = (result << 8) | *p++;
  result = (result << 8) | *p++;
  result = (result << 8) | *p++;

  return result;
}


static inline int64_t
__attribute ((always_inline))
decode64 (const void *ptr)
{
  if ((BYTE_ORDER == BIG_ENDIAN))
    return *(const int64_t *) ptr;

  return bswap_64 (*(const int64_t *) ptr);
}

char *
internal_function
__tzfile_determine_path (const char *file)
{
  static const char default_tzdir[] = TZDIR;
  if (file == NULL)
    /* No user specification; use the site-wide default.  */
    file = TZDEFAULT;
  else if (*file == '\0')
    /* User specified the empty string; use UTC with no leap seconds.  */
    ;
  else
    {
      /* We must not allow to read an arbitrary file in a setuid
	 program.  So we fail for any file which is not in the
	 directory hierachy starting at TZDIR
	 and which is not the system wide default TZDEFAULT.  */
      if (__libc_enable_secure
	  && ((*file == '/'
	       && memcmp (file, TZDEFAULT, sizeof TZDEFAULT)
	       && memcmp (file, default_tzdir, sizeof (default_tzdir) - 1))
	      || strstr (file, "../") != NULL))
	/* This test is certainly a bit too restrictive but it should
	   catch all critical cases.  */
	file = "";		/* Use UTC.  */
    }

  if (*file != '/')
    {
      const char *tzdir;

      tzdir = getenv ("TZDIR");
      if (tzdir == NULL || *tzdir == '\0')
	tzdir = default_tzdir;
      char *new;
      if (__asprintf (&new, "%s/%s", tzdir, file) == -1)
	return NULL;
      return new;
    }

  return strdup (file);
}

bool
internal_function
__tzfile_is_current (const char *path, const struct tzdata *tzdata)
{
  /* If we were already using tzfile, check whether the file changed.  */
  struct stat64 st;
  return tzdata->use_tzfile
    && stat64 (path, &st) == 0
    && tzdata->tzfile_ino == st.st_ino
    && tzdata->tzfile_dev == st.st_dev
    && tzdata->tzfile_mtime == st.st_mtime;
}

void
internal_function
__tzfile_read (struct tzdata *tzdata, const char *file,
	       size_t extra, char **extrap)
{
  size_t num_isstd, num_isgmt;
  FILE *f;
  struct tzhead tzhead;
  size_t chars;
  size_t i;
  size_t total_size;
  size_t types_idx;
  size_t leaps_idx;
  int trans_width = 4;
  size_t tzspec_len;

  tzdata->use_tzfile = false;

  /* Note the file is opened with cancellation in the I/O functions
     disabled and if available FD_CLOEXEC set.  */
  f = fopen (file, "rce");
  if (f == NULL)
    goto ret_free_transitions;

  /* Get information about the file we are actually using.  */
  struct stat64 st;
  if (fstat64 (__fileno (f), &st) != 0)
    {
      fclose (f);
      goto ret_free_transitions;
    }

  free (tzdata->transitions);
  tzdata->transitions = NULL;

  /* Remember the inode and device number and modification time.  */
  tzdata->tzfile_dev = st.st_dev;
  tzdata->tzfile_ino = st.st_ino;
  tzdata->tzfile_mtime = st.st_mtime;

  /* No threads reading this stream.  */
  __fsetlocking (f, FSETLOCKING_BYCALLER);

 read_again:
  if (__builtin_expect (__fread_unlocked ((void *) &tzhead, sizeof (tzhead),
					  1, f) != 1, 0)
      || memcmp (tzhead.tzh_magic, TZ_MAGIC, sizeof (tzhead.tzh_magic)) != 0)
    goto lose;

  tzdata->num_transitions = (size_t) decode (tzhead.tzh_timecnt);
  tzdata->num_types = (size_t) decode (tzhead.tzh_typecnt);
  chars = (size_t) decode (tzhead.tzh_charcnt);
  tzdata->num_leaps = (size_t) decode (tzhead.tzh_leapcnt);
  num_isstd = (size_t) decode (tzhead.tzh_ttisstdcnt);
  num_isgmt = (size_t) decode (tzhead.tzh_ttisgmtcnt);

  if (__glibc_unlikely (num_isstd > tzdata->num_types
			|| num_isgmt > tzdata->num_types))
    goto lose;

  /* For platforms with 64-bit time_t we use the new format if available.  */
  if (sizeof (time_t) == 8 && trans_width == 4
      && tzhead.tzh_version[0] != '\0')
    {
      /* We use the 8-byte format.  */
      trans_width = 8;

      /* Position the stream before the second header.  */
      size_t to_skip = (tzdata->num_transitions * (4 + 1)
			+ tzdata->num_types * 6
			+ chars
			+ tzdata->num_leaps * 8
			+ num_isstd
			+ num_isgmt);
      if (fseek (f, to_skip, SEEK_CUR) != 0)
	goto lose;

      goto read_again;
    }

  if (__builtin_expect (tzdata->num_transitions
			> ((SIZE_MAX - (__alignof__ (struct ttinfo) - 1))
			   / (sizeof (time_t) + 1)), 0))
    goto lose;
  total_size = tzdata->num_transitions * (sizeof (time_t) + 1);
  total_size = ((total_size + __alignof__ (struct ttinfo) - 1)
		& ~(__alignof__ (struct ttinfo) - 1));
  types_idx = total_size;
  if (__builtin_expect (tzdata->num_types
			> (SIZE_MAX - total_size) / sizeof (struct ttinfo), 0))
    goto lose;
  total_size += tzdata->num_types * sizeof (struct ttinfo);
  if (__glibc_unlikely (chars > SIZE_MAX - total_size))
    goto lose;
  total_size += chars;
  if (__builtin_expect (__alignof__ (struct leap) - 1
			> SIZE_MAX - total_size, 0))
    goto lose;
  total_size = ((total_size + __alignof__ (struct leap) - 1)
		& ~(__alignof__ (struct leap) - 1));
  leaps_idx = total_size;
  if (__builtin_expect (tzdata->num_leaps
			> (SIZE_MAX - total_size) / sizeof (struct leap), 0))
    goto lose;
  total_size += tzdata->num_leaps * sizeof (struct leap);
  tzspec_len = 0;
  if (sizeof (time_t) == 8 && trans_width == 8)
    {
      off_t rem = st.st_size - __ftello (f);
      if (__builtin_expect (rem < 0
			    || (size_t) rem < (tzdata->num_transitions * (8 + 1)
					       + tzdata->num_types * 6
					       + chars), 0))
	goto lose;
      tzspec_len = (size_t) rem - (tzdata->num_transitions * (8 + 1)
				   + tzdata->num_types * 6
				   + chars);
      if (__builtin_expect (tzdata->num_leaps > SIZE_MAX / 12
			    || tzspec_len < tzdata->num_leaps * 12, 0))
	goto lose;
      tzspec_len -= tzdata->num_leaps * 12;
      if (__glibc_unlikely (tzspec_len < num_isstd))
	goto lose;
      tzspec_len -= num_isstd;
      if (__glibc_unlikely (tzspec_len == 0 || tzspec_len - 1 < num_isgmt))
	goto lose;
      tzspec_len -= num_isgmt + 1;
      if (__glibc_unlikely (tzspec_len == 0
			    || SIZE_MAX - total_size < tzspec_len))
	goto lose;
    }
  if (__glibc_unlikely (SIZE_MAX - total_size - tzspec_len < extra))
    goto lose;

  /* Allocate enough memory including the extra block requested by the
     caller.  */
  tzdata->transitions = (time_t *) malloc (total_size + tzspec_len + extra);
  if (tzdata->transitions == NULL)
    goto lose;

  tzdata->type_idxs = (unsigned char *) tzdata->transitions
    + (tzdata->num_transitions * sizeof (time_t));
  tzdata->types = (struct ttinfo *) ((char *) tzdata->transitions + types_idx);
  tzdata->zone_names = (char *) tzdata->types
    + tzdata->num_types * sizeof (struct ttinfo);
  tzdata->leaps = (struct leap *) ((char *) tzdata->transitions + leaps_idx);
  if (sizeof (time_t) == 8 && trans_width == 8)
    tzdata->tzspec = (char *) tzdata->leaps
      + tzdata->num_leaps * sizeof (struct leap) + extra;
  else
    tzdata->tzspec = NULL;
  if (extra > 0)
    *extrap = (char *) &tzdata->leaps[tzdata->num_leaps];

  if (sizeof (time_t) == 4 || __builtin_expect (trans_width == 8, 1))
    {
      if (__builtin_expect (__fread_unlocked
			    (tzdata->transitions, trans_width + 1,
			     tzdata->num_transitions, f)
			    != tzdata->num_transitions, 0))
	goto lose;
    }
  else
    {
      if (__builtin_expect (__fread_unlocked (tzdata->transitions, 4,
					      tzdata->num_transitions, f)
			    != tzdata->num_transitions, 0)
	  || __builtin_expect (__fread_unlocked
			       (tzdata->type_idxs, 1, tzdata->num_transitions,
				f) != tzdata->num_transitions, 0))
	goto lose;
    }

  /* Check for bogus indices in the data file, so we can hereafter
     safely use type_idxs[T] as indices into `types' and never crash.  */
  for (i = 0; i < tzdata->num_transitions; ++i)
    if (__glibc_unlikely (tzdata->type_idxs[i] >= tzdata->num_types))
      goto lose;

  if ((BYTE_ORDER != BIG_ENDIAN && (sizeof (time_t) == 4 || trans_width == 4))
      || (BYTE_ORDER == BIG_ENDIAN && sizeof (time_t) == 8
	  && trans_width == 4))
    {
      /* Decode the transition times, stored as 4-byte integers in
	 network (big-endian) byte order.  We work from the end of
	 the array so as not to clobber the next element to be
	 processed when sizeof (time_t) > 4.  */
      i = tzdata->num_transitions;
      while (i-- > 0)
	tzdata->transitions[i] = decode ((char *) tzdata->transitions + i * 4);
    }
  else if (BYTE_ORDER != BIG_ENDIAN && sizeof (time_t) == 8)
    {
      /* Decode the transition times, stored as 8-byte integers in
	 network (big-endian) byte order.  */
      for (i = 0; i < tzdata->num_transitions; ++i)
	tzdata->transitions[i]
	  = decode64 ((char *) tzdata->transitions + i * 8);
    }

  for (i = 0; i < tzdata->num_types; ++i)
    {
      unsigned char x[4];
      int c;
      if (__builtin_expect (__fread_unlocked (x, 1,
					      sizeof (x), f) != sizeof (x),
			    0))
	goto lose;
      c = getc_unlocked (f);
      if (__glibc_unlikely ((unsigned int) c > 1u))
	goto lose;
      tzdata->types[i].isdst = c;
      c = getc_unlocked (f);
      if (__glibc_unlikely ((size_t) c > chars))
	/* Bogus index in data file.  */
	goto lose;
      tzdata->types[i].idx = c;
      tzdata->types[i].offset = (long int) decode (x);
    }

  if (__glibc_unlikely (__fread_unlocked (tzdata->zone_names, 1, chars, f)
			!= chars))
    goto lose;

  for (i = 0; i < tzdata->num_leaps; ++i)
    {
      unsigned char x[8];
      if (__builtin_expect (__fread_unlocked (x, 1, trans_width, f)
			    != trans_width, 0))
	goto lose;
      if (sizeof (time_t) == 4 || trans_width == 4)
	tzdata->leaps[i].transition = (time_t) decode (x);
      else
	tzdata->leaps[i].transition = (time_t) decode64 (x);

      if (__glibc_unlikely (__fread_unlocked (x, 1, 4, f) != 4))
	goto lose;
      tzdata->leaps[i].change = (long int) decode (x);
    }

  for (i = 0; i < num_isstd; ++i)
    {
      int c = getc_unlocked (f);
      if (__glibc_unlikely (c == EOF))
	goto lose;
      tzdata->types[i].isstd = c != 0;
    }
  while (i < tzdata->num_types)
    tzdata->types[i++].isstd = 0;

  for (i = 0; i < num_isgmt; ++i)
    {
      int c = getc_unlocked (f);
      if (__glibc_unlikely (c == EOF))
	goto lose;
      tzdata->types[i].isgmt = c != 0;
    }
  while (i < tzdata->num_types)
    tzdata->types[i++].isgmt = 0;

  /* Read the POSIX TZ-style information if possible.  */
  if (sizeof (time_t) == 8 && tzdata->tzspec != NULL)
    {
      /* Skip over the newline first.  */
      if (getc_unlocked (f) != '\n'
	  || (__fread_unlocked (tzdata->tzspec, 1, tzspec_len - 1, f)
	      != tzspec_len - 1))
	tzdata->tzspec = NULL;
      else
	tzdata->tzspec[tzspec_len - 1] = '\0';
    }
  else if (sizeof (time_t) == 4 && tzhead.tzh_version[0] != '\0')
    {
      /* Get the TZ string.  */
      if (__builtin_expect (__fread_unlocked ((void *) &tzhead,
					      sizeof (tzhead), 1, f) != 1, 0)
	  || (memcmp (tzhead.tzh_magic, TZ_MAGIC, sizeof (tzhead.tzh_magic))
	      != 0))
	goto lose;

      size_t num_transitions2 = (size_t) decode (tzhead.tzh_timecnt);
      size_t num_types2 = (size_t) decode (tzhead.tzh_typecnt);
      size_t chars2 = (size_t) decode (tzhead.tzh_charcnt);
      size_t num_leaps2 = (size_t) decode (tzhead.tzh_leapcnt);
      size_t num_isstd2 = (size_t) decode (tzhead.tzh_ttisstdcnt);
      size_t num_isgmt2 = (size_t) decode (tzhead.tzh_ttisgmtcnt);

      /* Position the stream before the second header.  */
      size_t to_skip = (num_transitions2 * (8 + 1)
			+ num_types2 * 6
			+ chars2
			+ num_leaps2 * 12
			+ num_isstd2
			+ num_isgmt2);
      off_t off;
      if (fseek (f, to_skip, SEEK_CUR) != 0
	  || (off = __ftello (f)) < 0
	  || st.st_size < off + 2)
	goto lose;

      tzspec_len = st.st_size - off - 1;
      if (tzspec_len == 0)
	goto lose;
      char *tzstr = malloc (tzspec_len);
      if (tzstr == NULL)
	goto lose;
      if (getc_unlocked (f) != '\n'
	  || (__fread_unlocked (tzstr, 1, tzspec_len - 1, f)
	      != tzspec_len - 1))
	{
	  free (tzstr);
	  goto lose;
	}
      tzstr[tzspec_len - 1] = '\0';
      tzdata->tzspec = __tzstring (tzstr);
      free (tzstr);
    }

  /* Don't use an empty TZ string.  */
  if (tzdata->tzspec != NULL && tzdata->tzspec[0] == '\0')
    tzdata->tzspec = NULL;

  fclose (f);

  /* First "register" all timezone names.  */
  for (i = 0; i < tzdata->num_types; ++i)
    (void) __tzstring (&tzdata->zone_names[tzdata->types[i].idx]);

  /* Find the standard and daylight time offsets used by the rule file.
     We choose the offsets in the types of each flavor that are
     transitioned to earliest in time.  */
  tzdata->tzname[0] = NULL;
  tzdata->tzname[1] = NULL;
  for (i = tzdata->num_transitions; i > 0; )
    {
      int type = tzdata->type_idxs[--i];
      int dst = tzdata->types[type].isdst;

      if (tzdata->tzname[dst] == NULL)
	{
	  int idx = tzdata->types[type].idx;

	  tzdata->tzname[dst] = __tzstring (&tzdata->zone_names[idx]);

	  if (tzdata->tzname[1 - dst] != NULL)
	    break;
	}
    }
  if (tzdata->tzname[0] == NULL)
    {
      /* This should only happen if there are no transition rules.
	 In this case there should be only one single type.  */
      assert (tzdata->num_types == 1);
      tzdata->tzname[0] = __tzstring (tzdata->zone_names);
    }
  if (tzdata->tzname[1] == NULL)
    tzdata->tzname[1] = tzdata->tzname[0];

  if (tzdata->num_transitions == 0)
    /* Use the first rule (which should also be the only one).  */
    tzdata->rule_stdoff = tzdata->rule_dstoff = tzdata->types[0].offset;
  else
    {
      int stdoff_set = 0, dstoff_set = 0;
      tzdata->rule_stdoff = tzdata->rule_dstoff = 0;
      i = tzdata->num_transitions - 1;
      do
	{
	  if (!stdoff_set && !tzdata->types[tzdata->type_idxs[i]].isdst)
	    {
	      stdoff_set = 1;
	      tzdata->rule_stdoff = tzdata->types[tzdata->type_idxs[i]].offset;
	    }
	  else if (!dstoff_set && tzdata->types[tzdata->type_idxs[i]].isdst)
	    {
	      dstoff_set = 1;
	      tzdata->rule_dstoff = tzdata->types[tzdata->type_idxs[i]].offset;
	    }
	  if (stdoff_set && dstoff_set)
	    break;
	}
      while (i-- > 0);

      if (!dstoff_set)
	tzdata->rule_dstoff = tzdata->rule_stdoff;
    }

  tzdata->daylight = tzdata->rule_stdoff != tzdata->rule_dstoff;
  tzdata->timezone = -tzdata->rule_stdoff;
  tzdata->use_tzfile = true;
  return;

 lose:
  fclose (f);
 ret_free_transitions:
  free (tzdata->transitions);
  tzdata->transitions = NULL;
}

/* The user specified a hand-made timezone, but not its DST rules.
   We will use the names and offsets from the user, and the rules
   from the TZDEFRULES file.  */
void
internal_function
__tzfile_default (struct tzdata *tzdata,
		  const char *std, const char *dst,
		  long int stdoff, long int dstoff)
{
  size_t stdlen = strlen (std) + 1;
  size_t dstlen = strlen (dst) + 1;
  size_t i;
  int isdst;
  char *cp;

  __tzfile_read (tzdata, TZDEFRULES, stdlen + dstlen, &cp);
  if (!tzdata->use_tzfile)
    return;

  if (tzdata->num_types < 2)
    {
      tzdata->use_tzfile = false;
      return;
    }

  /* Ignore the zone names read from the file and use the given ones
     instead.  */
  __mempcpy (__mempcpy (cp, std, stdlen), dst, dstlen);
  tzdata->zone_names = cp;

  /* Now there are only two zones, regardless of what the file contained.  */
  tzdata->num_types = 2;

  /* Reset the zone names to point to the user's names.  */
  tzdata->tzname[0] = tzdata->zone_names;
  tzdata->tzname[1] = tzdata->zone_names + stdlen;

  /* Now correct the transition times for the user-specified standard and
     daylight offsets from GMT.  */
  isdst = 0;
  for (i = 0; i < tzdata->num_transitions; ++i)
    {
      struct ttinfo *trans_type = &tzdata->types[tzdata->type_idxs[i]];

      /* We will use only types 0 (standard) and 1 (daylight).
	 Fix up this transition to point to whichever matches
	 the flavor of its original type.  */
      tzdata->type_idxs[i] = trans_type->isdst;

      if (trans_type->isgmt)
	/* The transition time is in GMT.  No correction to apply.  */ ;
      else if (isdst && !trans_type->isstd)
	/* The type says this transition is in "local wall clock time", and
	   wall clock time as of the previous transition was DST.  Correct
	   for the difference between the rule's DST offset and the user's
	   DST offset.  */
	tzdata->transitions[i] += dstoff - tzdata->rule_dstoff;
      else
	/* This transition is in "local wall clock time", and wall clock
	   time as of this iteration is non-DST.  Correct for the
	   difference between the rule's standard offset and the user's
	   standard offset.  */
	tzdata->transitions[i] += stdoff - tzdata->rule_stdoff;

      /* The DST state of "local wall clock time" for the next iteration is
	 as specified by this transition.  */
      isdst = trans_type->isdst;
    }

  /* Now that we adjusted the transitions to the requested offsets,
     reset the rule_stdoff and rule_dstoff values appropriately.  They
     are used elsewhere.  */
  tzdata->rule_stdoff = stdoff;
  tzdata->rule_dstoff = dstoff;

  /* Reset types 0 and 1 to describe the user's settings.  */
  tzdata->types[0].idx = 0;
  tzdata->types[0].offset = stdoff;
  tzdata->types[0].isdst = 0;
  tzdata->types[1].idx = stdlen;
  tzdata->types[1].offset = dstoff;
  tzdata->types[1].isdst = 1;

  /* Set the timezone.  */
  __timezone = -tzdata->types[0].offset;

  /* Invalidate the tzfile attribute cache to force rereading
     TZDEFRULES the next time it is used.  */
  tzdata->tzfile_dev = 0;
  tzdata->tzfile_ino = 0;
  tzdata->tzfile_mtime = 0;
}

void
internal_function
__tzfile_compute (const struct tzdata *tzdata, time_t timer, int use_localtime,
		  long int *leap_correct, int *leap_hit,
		  struct tm *tp)
{
  size_t i;

  if (use_localtime)
    {
      __tzname[0] = NULL;
      __tzname[1] = NULL;

      if (__glibc_unlikely (tzdata->num_transitions == 0
			    || timer < tzdata->transitions[0]))
	{
	  /* TIMER is before any transition (or there are no transitions).
	     Choose the first non-DST type
	     (or the first if they're all DST types).  */
	  i = 0;
	  while (i < tzdata->num_types && tzdata->types[i].isdst)
	    {
	      if (__tzname[1] == NULL)
		__tzname[1] = __tzstring
		  (&tzdata->zone_names[tzdata->types[i].idx]);

	      ++i;
	    }

	  if (i == tzdata->num_types)
	    i = 0;
	  __tzname[0] = __tzstring
	    (&tzdata->zone_names[tzdata->types[i].idx]);
	  if (__tzname[1] == NULL)
	    {
	      size_t j = i;
	      while (j < tzdata->num_types)
		if (tzdata->types[j].isdst)
		  {
		    __tzname[1] = __tzstring
		      (&tzdata->zone_names[tzdata->types[j].idx]);
		    break;
		  }
		else
		  ++j;
	    }
	}
      else if (__glibc_unlikely (timer >= transitions[num_transitions - 1]))
	{
	  if (__glibc_unlikely (tzdata->tzspec == NULL))
	    {
	    use_last:
	      i = num_transitions;
	      goto found;
	    }

	  /* Parse the POSIX TZ-style string.  */
	  __tzset_parse_tz (tzdata, tzdata->tzspec);

	  /* Convert to broken down structure.  If this fails do not
	     use the string.  */
	  if (__glibc_unlikely (! __offtime (&timer, 0, tp)))
	    goto use_last;

	  /* Use the rules from the TZ string to compute the change.  */
	  __tz_compute (tzdata, timer, tp, 1);

	  /* If tzspec comes from posixrules loaded by __tzfile_default,
	     override the STD and DST zone names with the ones user
	     requested in TZ envvar.  */
	  if (__glibc_unlikely (tzdata->zone_names
				== (char *) &tzdata->leaps[tzdata->num_leaps]))
	    {
	      assert (tzdata->num_types == 2);
	      __tzname[0] = __tzstring (tzdata->zone_names);
	      __tzname[1] = __tzstring
		(&tzdata->zone_names[strlen (tzdata->zone_names) + 1]);
	    }

	  goto leap;
	}
      else
	{
	  /* Find the first transition after TIMER, and
	     then pick the type of the transition before it.  */
	  size_t lo = 0;
	  size_t hi = tzdata->num_transitions - 1;
	  /* Assume that DST is changing twice a year and guess initial
	     search spot from it.
	     Half of a gregorian year has on average 365.2425 * 86400 / 2
	     = 15778476 seconds.  */
	  i = (tzdata->transitions[tzdata->num_transitions - 1] - timer)
	    / 15778476;
	  if (i < tzdata->num_transitions)
	    {
	      i = num_transitions - 1 - i;
	      if (timer < tzdata->transitions[i])
		{
		  if (i < 10 || timer >= tzdata->transitions[i - 10])
		    {
		      /* Linear search.  */
		      while (timer < tzdata->transitions[i - 1])
			--i;
		      goto found;
		    }
		  hi = i - 10;
		}
	      else
		{
		  if (i + 10 >= tzdata->num_transitions
		      || timer < tzdata->transitions[i + 10])
		    {
		      /* Linear search.  */
		      while (timer >= tzdata->transitions[i])
			++i;
		      goto found;
		    }
		  lo = i + 10;
		}
	    }

	  /* Binary search.  */
	  /* assert (timer >= transitions[lo] && timer < transitions[hi]); */
	  while (lo + 1 < hi)
	    {
	      i = (lo + hi) / 2;
	      if (timer < transitions[i])
		hi = i;
	      else
		lo = i;
	    }
	  i = hi;

	found:
	  /* assert (timer >= transitions[i - 1]
	     && (i == tzdata->num_transitions || timer < transitions[i])); */
	  __tzname[tzdata->types[tzdata->type_idxs[i - 1]].isdst]
	    = __tzstring (&tzdata->zone_names
			  [tzdata->types[tzdata->type_idxs[i - 1]].idx]);
	  size_t j = i;
	  while (j < tzdata->num_transitions)
	    {
	      int type = tzdata->type_idxs[j];
	      int dst = tzdata->types[type].isdst;
	      int idx = tzdata->types[type].idx;

	      if (__tzname[dst] == NULL)
		{
		  __tzname[dst] = __tzstring (&tzdata->zone_names[idx]);

		  if (__tzname[1 - dst] != NULL)
		    break;
		}

	      ++j;
	    }

	  if (__glibc_unlikely (__tzname[0] == NULL))
	    __tzname[0] = __tzname[1];

	  i = tzdata->type_idxs[i - 1];
	}

      struct ttinfo *info = &tzdata->types[i];
      __daylight = tzdata->rule_stdoff != tzdata->rule_dstoff;
      __timezone = -tzdata->rule_stdoff;

      if (__tzname[0] == NULL)
	{
	  /* This should only happen if there are no transition rules.
	     In this case there should be only one single type.  */
	  assert (tzdata->num_types == 1);
	  __tzname[0] = __tzstring (tzdata->zone_names);
	}
      if (__tzname[1] == NULL)
	/* There is no daylight saving time.  */
	__tzname[1] = __tzname[0];
      tp->tm_isdst = info->isdst;
      assert (strcmp (&tzdata->zone_names[info->idx],
		      __tzname[tp->tm_isdst]) == 0);
      tp->tm_zone = __tzname[tp->tm_isdst];
      tp->tm_gmtoff = info->offset;
    }

 leap:
  *leap_correct = 0L;
  *leap_hit = 0;

  /* Find the last leap second correction transition time before TIMER.  */
  i = tzdata->num_leaps;
  do
    if (i-- == 0)
      return;
  while (timer < tzdata->leaps[i].transition);

  /* Apply its correction.  */
  *leap_correct = tzdata->leaps[i].change;

  if (/* Exactly at the transition time.  */
      timer == tzdata->leaps[i].transition
      && ((i == 0 && tzdata->leaps[i].change > 0) ||
	  tzdata->leaps[i].change > tzdata->leaps[i - 1].change))
    {
      *leap_hit = 1;
      while (i > 0
	     && (tzdata->leaps[i].transition
		 == tzdata->leaps[i - 1].transition + 1)
	     && tzdata->leaps[i].change == tzdata->leaps[i - 1].change + 1)
	{
	  ++*leap_hit;
	  --i;
	}
    }
}

void
internal_function
__tzdata_free (struct tzdata *tzdata)
{
  free (tzdata->transitions);
  free (tzdata->old_tz);
  free (tzdata);
}
