/* C-version of memcpy for using when Intel MPX is on
   in order to prosess with a buffer of pointers correctly.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <stddef.h>

void *
__memcpy (void *dst, const void *src, size_t n)
{
  if (!n) return dst;

  __bnd_chk_ptr_lbounds(dst);
  __bnd_chk_ptr_ubounds(dst+n-1);
#ifndef __CHKWR__
  __bnd_chk_ptr_lbounds(src);
  __bnd_chk_ptr_ubounds(src+n-1);
#endif

  return chkp_memcpy_nochk(dst, src, n);
}

void *
chkp_memcpy_nochk (void *dst, const void *src, size_t n)
{
  const char *s = src;
  char *d = dst;
  void *ret = dst;
  size_t offset_src = ((size_t) s) & (sizeof(size_t) - 1);
  size_t offset_dst = ((size_t) d) & (sizeof(size_t) - 1);

  if (offset_src != offset_dst)
  {
    while (n--)
      *d++ = *s++;
  }
  else
  {
    if (offset_src) offset_src = sizeof(size_t) - offset_src;
    while (n-- && offset_src--)
      *d++ = *s++;
    n++;
    if (!n) return ret;
    void **d1 = (void **)d;
    void **s1 = (void **)s;
    while (n >= sizeof(void *))
    {
      n -= sizeof(void *);
      *d1++ = *s1++;
    }
    s = (char *)s1;
    d = (char *)d1;
    while (n--)
      *d++ = *s++;
  }
  return ret;
}

weak_alias (__memcpy, __GI_memcpy)

# if defined SHARED && !defined NOT_IN_libc && !defined IA32
#  include <shlib-compat.h>
versioned_symbol (libc, __memcpy, memcpy, GLIBC_2_14);
# else
weak_alias (__memcpy, memcpy)
# endif
