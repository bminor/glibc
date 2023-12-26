/* Largest integral value not greater than argument.  MIPS version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#if ((__mips_fpr == 64) \
      && (__mips_hard_float == 1) \
      && ((__mips == 32 && __mips_isa_rev > 1) || __mips == 64))

#define NO_MATH_REDIRECT
#include <math.h>
#include <round_to_integer.h>
#include <libm-alias-double.h>

double
__floor (double x)
{
  return round_to_integer_double (FLOOR, x);
}
libm_alias_double (__floor, floor)
#else
# include <sysdeps/ieee754/dbl-64/s_floor.c>
#endif
