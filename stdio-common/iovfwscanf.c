/* Implementation and symbols for _IO_vfwscanf.
   Copyright (C) 1991-2019 Free Software Foundation, Inc.
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

#include <libioP.h>
#include <shlib-compat.h>

/* This function is provided for ports older than GLIBC 2.29 because
   external callers could theoretically exist.  Newer ports do not need,
   since it is not part of the API.  */
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_29)

int
attribute_compat_text_section
__IO_vfwscanf (FILE *fp, const wchar_t *format, va_list ap, int *errp)
{
  int rv = __vfwscanf_internal (fp, format, ap, 0);
  if (__glibc_unlikely (errp != 0))
    *errp = (rv == -1);
  return rv;
}
compat_symbol (libc, __IO_vfwscanf, _IO_vfwscanf, GLIBC_2_0);

#endif
