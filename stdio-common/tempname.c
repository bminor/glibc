/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
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

#define __need_size_t
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

/* Perform the "SVID path search malarkey" on DIR and PFX.  Write a
   template suitable for use in __gen_tempname into TMPL, bounded
   by TMPL_LEN. */
int
__path_search (tmpl, tmpl_len, dir, pfx, try_tmpdir)
     char *tmpl;
     size_t tmpl_len;
     const char *dir;
     const char *pfx;
     int try_tmpdir;
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (__path_search)

/* Generate a temporary file name based on TMPL.  TMPL must match the
   rules for mk[s]temp (i.e. end in "XXXXXX", possibly with a suffix).
   The name constructed does not exist at the time of the call to
   __gen_tempname.  TMPL is overwritten with the result.

   The *TRY_NAME function is called repeatedly on candidate names until
   it returns >= 0.  If it returns -2, the next candidate name is tried.
   If it returns -1 (with errno set), __gen_tempname fails immediately.  */
int
__gen_tempname (char *tmpl, int suffixlen,
                int (*try_name) (const char *name, void *arg),
                void *try_name_arg)
{
  __set_errno (ENOSYS);
  return -1;
}

stub_warning (__gen_tempname)
