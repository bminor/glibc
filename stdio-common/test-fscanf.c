/* Test fscanf of long double without <stdio.h>.
   Copyright (C) 2024 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <string.h>

struct FILE;
extern struct FILE *stdin;
extern int fscanf (struct FILE *, const char *, ...);

int
main (void)
{
  int n, i;
  long double x;
  char name[50];
  n = fscanf (stdin, "%d%Lf%s", &i, &x, name);

  if (n != 3 || i != 25 || x != 24.5 || strcmp (name, "thompson"))
    return 1;

  return 0;
}
