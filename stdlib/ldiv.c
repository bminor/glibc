/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stdlib.h>


/* Return the `ldiv_t' representation of NUMER over DENOM.  */
__CONSTVALUE
ldiv_t
DEFUN(ldiv, (numer, denom), long int numer AND long int denom)
{
  ldiv_t result;
  /* Behavior for negative numbers is guaranteed.  */
  register long int sign = (numer < 0L) == (denom < 0L) ? 1L : -1L;

  numer = numer < 0L ? -numer : numer;
  denom = denom < 0L ? -denom : denom;

  result.quot = (numer / denom) * sign;
  result.rem = (numer % denom) * sign;

  return result;
}
