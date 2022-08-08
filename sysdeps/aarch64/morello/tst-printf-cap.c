/* Check printf for capabilities functionality.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/test-driver.h>

int check_match (char *ptr, const char *pattern)
{
  char str[100];
  sprintf (str, "%#p", ptr);
  verbose_printf ("Test permission \"%s\" in \"%s\".\n", pattern, str);
  if (!strstr(str, pattern))
    {
      printf ("FAIL: permission \"%s\" in \"%s\".\n", pattern, str);
      return 1;
    }
  return 0;
}

int check_bounds (char *ptr, uint64_t len)
{
  char str[100];
  char u_bound[100];
  sprintf (str, "%#p", ptr);
  sprintf (u_bound, "%p", (ptr + len));
  verbose_printf ("Test bounds \"%s\" in \"%s\"\n", u_bound, str);
  if (!strstr (str, u_bound))
    {
      printf ("FAIL: bounds \"%s\" in \"%s\"\n", u_bound, str);
      return 1;
    }
  return 0;
}

static void *
map (int prot)
{
  return xmmap (0, 16, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1);
}

static int
do_test (void)
{
  static const int x;
  static int y;
  void *r_global = (void *) &x;
  void *rw_global = &y;
  void *rx_global = (void *) do_test;
  void *rw_malloc = malloc (16);
  void *none_map = map (PROT_NONE);
  void *r_map = map (PROT_READ);
  void *rw_map = map (PROT_READ | PROT_WRITE);
  void *rx_map = map (PROT_READ | PROT_EXEC);
  void *rwx_map = map (PROT_READ | PROT_WRITE | PROT_EXEC);
  int ret = 0;

  /* Test permissions.  */
  ret += check_match (none_map, "[,");
  ret += check_match (r_global, "[rR,");
  ret += check_match (r_map, "[rR,");
  ret += check_match (rw_global, "[rwRW,");
  ret += check_match (rw_malloc, "[rwRW,");
  ret += check_match (rw_map, "[rwRW,");
  ret += check_match (rx_global, "[rxRE,");
  ret += check_match (rx_map, "[rxRE,");
  ret += check_match (rwx_map, "[rwxRWE,");

  /* Test bounds.  */
  ret += check_bounds (rw_malloc, 16);

  return ret;
}

#include <support/test-driver.c>
