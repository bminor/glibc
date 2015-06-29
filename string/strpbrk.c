/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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

#include <string.h>
#include <stdint.h>
#undef strpbrk
#undef strcspn


#ifdef AS_STRCSPN
# ifndef STRPBRK
#  define STRPBRK strcspn
# endif
# define RETURN_TYPE size_t
# define RETURN(c) return c
#else
# define RETURN_TYPE char *
# define RETURN(c) return (char *) (s[c] != '\0' ? s + c : NULL)
#endif

#ifndef STRPBRK
#define STRPBRK strpbrk
#endif


/* Find the first occurrence in S of any character in ACCEPT.  */
RETURN_TYPE
STRPBRK (const char *_s, const char *_accept)
{
  unsigned char *s = (unsigned char *) _s;
  unsigned char *a = (unsigned char *) _accept;

#ifndef LATE_CHECK
  /* We need to align s to 4 bytes. We do check now to avoid expensive table
     construction.  */
  do
    {
      if (s[0] == *a)
        RETURN(0);
    }
  while (*a++);
  a = (unsigned char *) _accept;

  /* We couldn't do these checks in one loop as gcc
     messes up register allocation.  */
  do
    {
      if (s[1] == *a)
        RETURN(1);
    }
  while (*a++);
  a = (unsigned char *) _accept;

  do
    {
      if (s[2] == *a)
        RETURN(2);
    }
  while (*a++);
  a = (unsigned char *) _accept;

  do
    {
      if (s[3] == *a)
        RETURN(3);
    }
  while (*a++);
  a = (unsigned char *) _accept;

#endif

  unsigned char table[256];
  memset (table, 0, 256);
  do
    {
      table[*a] = 1;
    }
  while (*a++);
  unsigned char s0, s1, s2, s3;
  size_t count = 0;
#ifdef LATE_CHECK
  s0 = s[count + 0];
  s1 = s[count + 1];
  s2 = s[count + 2];
  s3 = s[count + 3];
  if (table[s0])
    goto ret0;
  if (table[s1])
    goto ret1;
  if (table[s2])
    goto ret2;
  if (table[s3])
    goto ret3;

#endif

  count = 4 - ((uintptr_t) s) % 4;

  while (1)
    {
      s0 = s[count + 0];
      s1 = s[count + 1];
      s2 = s[count + 2];
      s3 = s[count + 3];
      if (table[s0])
        goto ret0;
      if (table[s1])
        goto ret1;
      if (table[s2])
        goto ret2;
      if (table[s3])
        goto ret3;
      count += 4;
    }
  ret3:
  count++;
  ret2:
  count++;
  ret1:
  count++;
  ret0:
  RETURN(count);
}

libc_hidden_builtin_def (STRPBRK)
