/* Install given floating-point control modes.  PowerPC soft-float version.
   Copyright (C) 2016-2025 Free Software Foundation, Inc.
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

#include "soft-fp.h"
#include "soft-supp.h"

int
fesetmode (const femode_t *modep)
{
  fenv_union_t u;

  u.fenv = *modep;
  __sim_round_mode_thread = u.l[0];
  SIM_SET_GLOBAL (__sim_round_mode_global, __sim_round_mode_thread);
  __sim_disabled_exceptions_thread = u.l[1];
  SIM_SET_GLOBAL (__sim_disabled_exceptions_global,
		  __sim_disabled_exceptions_thread);
  return 0;
}
