/* Undefined Behavior Sanitizer support.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#include "ubsan.h"

void
__ubsan_handle_overflow (const struct overflow_data * data, void *lhs,
			 void *rhs, const char *op)
{
  char lhs_str[UBSAN_VAL_STR_LEN];
  char rhs_str[UBSAN_VAL_STR_LEN];

  __ubsan_val_to_string (lhs_str, data->type, lhs);
  __ubsan_val_to_string (rhs_str, data->type, rhs);

  __ubsan_error (&data->location,
		 "%s integer overflow: %s %s %s cannot be represened in "
		 "type %s\n",
		 ubsan_type_is_signed (data->type) ? "signed" : "unsigned",
		 lhs_str,
		 op,
		 rhs_str,
		 data->type->type_name);
}
