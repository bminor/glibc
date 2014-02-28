/* Copyright (C) 2013 Free Software Foundation, Inc.
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

/* This is a Google-local stub version of nis/ypclnt.c.  These functions
   are required for dynamic linking of some precompiled executables.  */

#include <stdlib.h>
#include <stdio.h>

int yp_get_default_domain(char **domp) {
  /* We duplicate glibc's error behavior and return a null pointer.  */
  *domp = NULL;
  return 12; /* YPERR_NODOM */
}

static const char err[] = "not implemented in Google-local stub";
char *yperr_string(int incode) {
  return err;
}

int yp_match(char *indomain, char *inmap, const char *inkey, int inkeylen,
             char **outval, int *outvallen) {
  *outval = malloc(2);
  (*outval)[0] = '\n';
  (*outval)[1] = '\0';
  *outvallen = 0;
  return 0;
}
