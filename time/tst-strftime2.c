/* Verify the behavior of strftime on alternative representation for
   year.

   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

static const char *locales[] = { "ja_JP.UTF-8", "lo_LA.UTF-8", "th_TH.UTF-8" };

static const char *formats[] = { "%EY", "%_EY", "%-EY" };

static const struct
{
  const int d, m, y;
} dates[] =
  {
    { 1, 3, 88 },
    { 7, 0, 89 },
    { 8, 0, 89 },
    { 1, 3, 90 },
    { 1, 3, 97 },
    { 1, 3, 98 }
  };

static char ref[3][3][6][100];

static void
mkreftable (void)
{
  int i, j, k;
  char era[10];
  static const int yrj[] = { 63, 64, 1, 2, 9, 10 };
  static const int yrb[] = { 2531, 2532, 2532, 2533, 2540, 2541 };

  for (i = 0; i < array_length (locales); i++)
    for (j = 0; j < array_length (formats); j++)
      for (k = 0; k < array_length (dates); k++)
	{
	  if (i == 0)
	    {
	      sprintf (era, "%s", (k < 2) ? "\xe6\x98\xad\xe5\x92\x8c"
					  : "\xe5\xb9\xb3\xe6\x88\x90");
	      if (yrj[k] == 1)
		sprintf (ref[i][j][k], "%s\xe5\x85\x83\xe5\xb9\xb4", era);
	      else
		{
		  if (j == 0)
		    sprintf (ref[i][j][k], "%s%02d\xe5\xb9\xb4", era, yrj[k]);
		  else if (j == 1)
		    sprintf (ref[i][j][k], "%s%2d\xe5\xb9\xb4", era, yrj[k]);
		  else
		    sprintf (ref[i][j][k], "%s%d\xe5\xb9\xb4", era, yrj[k]);
		}
	    }
	  else if (i == 1)
	    {
	      sprintf (era, "\xe0\xba\x9e\x2e\xe0\xba\xaa\x2e ");
	      sprintf (ref[i][j][k], "%s%d", era, yrb[k]);
	    }
	  else
	    {
	      sprintf (era, "\xe0\xb8\x9e\x2e\xe0\xb8\xa8\x2e ");
	      sprintf (ref[i][j][k], "%s%d", era, yrb[k]);
	    }
	}
}

static int
do_test (void)
{
  int i, j, k, result = 0;
  struct tm ttm;
  char date[11], buf[100];
  size_t r, e;

  mkreftable ();
  for (i = 0; i < array_length (locales); i++)
    {
      if (setlocale (LC_ALL, locales[i]) == NULL)
	{
	  printf ("locale %s does not exist, skipping...\n", locales[i]);
	  continue;
	}
      printf ("[%s]\n", locales[i]);
      for (j = 0; j < array_length (formats); j++)
	{
	  for (k = 0; k < array_length (dates); k++)
	    {
	      ttm.tm_mday = dates[k].d;
	      ttm.tm_mon  = dates[k].m;
	      ttm.tm_year = dates[k].y;
	      strftime (date, sizeof (date), "%F", &ttm);
	      r = strftime (buf, sizeof (buf), formats[j], &ttm);
	      e = strlen (ref[i][j][k]);
	      printf ("%s\t\"%s\"\t\"%s\"", date, formats[j], buf);
	      if (strcmp (buf, ref[i][j][k]) != 0)
		{
		  printf ("\tshould be \"%s\"", ref[i][j][k]);
		  if (r != e)
		    printf ("\tgot: %zu, expected: %zu", r, e);
		  result = 1;
		}
	      else
		printf ("\tOK");
	      putchar ('\n');
	    }
	  putchar ('\n');
	}
    }
  return result;
}

#include <support/test-driver.c>
