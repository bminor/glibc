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
#ifdef	BSD
#include </usr/include/stdio.h>
#define EXIT_SUCCESS 0
#else
#include <stdio.h>
#include <stdlib.h>
#endif


void
DEFUN(fmtchk, (fmt), CONST char *fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`");
  (void) printf(fmt, 0x12);
  (void) printf("'\n");
}

void
DEFUN(fmtst1chk, (fmt), CONST char *fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`");
  (void) printf(fmt, 4, 0x12);
  (void) printf("'\n");
}

void
DEFUN(fmtst2chk, (fmt), CONST char *fmt)
{
  (void) fputs(fmt, stdout);
  (void) printf(":\t`");
  (void) printf(fmt, 4, 4, 0x12);
  (void) printf("'\n");
}

int
DEFUN_VOID(main)
{
  static char shortstr[] = "Hi, Z.";
  static char longstr[] = "Good morning, Doctor Chandra.  This is Hal.  \
I am ready for my first lesson today.";

  fmtchk("%.4x");
  fmtchk("%04x");
  fmtchk("%4.4x");
  fmtchk("%04.4x");
  fmtchk("%4.3x");
  fmtchk("%04.3x");

  fmtst1chk("%.*x");
  fmtst1chk("%0*x");
  fmtst2chk("%*.*x");
  fmtst2chk("%0*.*x");

#ifndef	BSD
  printf("bad format:\t\"%z\"\n");
  printf("nil pointer:\t\"%p\"\n", (PTR) NULL);
#endif

  printf("decimal negative:\t\"%d\"\n", -2345);
  printf("octal negative:\t\"%o\"\n", -2345);
  printf("hex negative:\t\"%x\"\n", -2345);
  printf("long decimal number:\t\"%ld\"\n", -123456);
  printf("long octal negative:\t\"%lo\"\n", -2345L);
  printf("long unsigned decimal number:\t\"%lu\"\n", -123456);
  printf("zero-padded LDN:\t\"%010ld\"\n", -123456);
  printf("left-adjusted ZLDN:\t\"%-010ld\"\n", -123456);
  printf("space-padded LDN:\t\"%10ld\"\n", -123456);
  printf("left-adjusted SLDN:\t\"%-10ld\"\n", -123456);

  printf("zero-padded string:\t\"%010s\"\n", shortstr);
  printf("left-adjusted Z string:\t\"%-010s\"\n", shortstr);
  printf("space-padded string:\t\"%10s\"\n", shortstr);
  printf("left-adjusted S string:\t\"%-10s\"\n", shortstr);
  printf("null string:\t\"%s\"\n", (char *)NULL);
  printf("limited string:\t\"%.22s\"\n", longstr);

  printf("e-style >= 1:\t\"%e\"\n", 12.34);
  printf("e-style >= .1:\t\"%e\"\n", 0.1234);
  printf("e-style < .1:\t\"%e\"\n", 0.001234);
  printf("e-style big:\t\"%.60e\"\n", 1e20);
  printf("f-style >= 1:\t\"%f\"\n", 12.34);
  printf("f-style >= .1:\t\"%f\"\n", 0.1234);
  printf("f-style < .1:\t\"%f\"\n", 0.001234);
  printf("g-style >= 1:\t\"%g\"\n", 12.34);
  printf("g-style >= .1:\t\"%g\"\n", 0.1234);
  printf("g-style < .1:\t\"%g\"\n", 0.001234);
  printf("g-style big:\t\"%.60g\"\n", 1e20);

#define FORMAT "|%12.4f|%12.4e|%12.4g|\n"
  printf (FORMAT, 0.0, 0.0, 0.0);
  printf (FORMAT, 1.0, 1.0, 1.0);
  printf (FORMAT, -1.0, -1.0, -1.0);
  printf (FORMAT, 100.0, 100.0, 100.0);
  printf (FORMAT, 1000.0, 1000.0, 1000.0);
  printf (FORMAT, 10000.0, 10000.0, 10000.0);
  printf (FORMAT, 12345.0, 12345.0, 12345.0);
  printf (FORMAT, 100000.0, 100000.0, 100000.0);
  printf (FORMAT, 123456.0, 123456.0, 123456.0);
#undef	FORMAT

  {
    char buf[20];
    printf ("snprintf (\"%%30s\", \"foo\") == %d, \"%.*s\"\n",
	    snprintf (buf, sizeof (buf), "%30s", "foo"), sizeof (buf), buf);
  }

  exit(EXIT_SUCCESS);
}
