/* Huge Page support.  Linux implementation.
   Copyright (C) 2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <intprops.h>
#include <malloc-hugepages.h>
#include <not-cancel.h>

unsigned long int
__malloc_default_thp_pagesize (void)
{
  int fd = __open64_nocancel (
    "/sys/kernel/mm/transparent_hugepage/hpage_pmd_size", O_RDONLY);
  if (fd == -1)
    return 0;

  char str[INT_BUFSIZE_BOUND (unsigned long int)];
  ssize_t s = __read_nocancel (fd, str, sizeof (str));
  __close_nocancel (fd);
  if (s < 0)
    return 0;

  unsigned long int r = 0;
  for (ssize_t i = 0; i < s; i++)
    {
      if (str[i] == '\n')
	break;
      r *= 10;
      r += str[i] - '0';
    }
  return r;
}

enum malloc_thp_mode_t
__malloc_thp_mode (void)
{
  int fd = __open64_nocancel ("/sys/kernel/mm/transparent_hugepage/enabled",
			      O_RDONLY);
  if (fd == -1)
    return malloc_thp_mode_not_supported;

  static const char mode_always[]  = "[always] madvise never\n";
  static const char mode_madvise[] = "always [madvise] never\n";
  static const char mode_never[]   = "always madvise [never]\n";

  char str[sizeof(mode_always)];
  ssize_t s = __read_nocancel (fd, str, sizeof (str));
  __close_nocancel (fd);

  if (s == sizeof (mode_always) - 1)
    {
      if (strcmp (str, mode_always) == 0)
	return malloc_thp_mode_always;
      else if (strcmp (str, mode_madvise) == 0)
	return malloc_thp_mode_madvise;
      else if (strcmp (str, mode_never) == 0)
	return malloc_thp_mode_never;
    }
  return malloc_thp_mode_not_supported;
}
