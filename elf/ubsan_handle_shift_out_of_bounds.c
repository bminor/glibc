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

#include <ubsan.h>

void
__ubsan_handle_shift_out_of_bounds (void *_data, void *lhs, void *rhs)
{
  struct shift_out_of_bounds_data *data = _data;
  char lhs_str[UBSAN_VAL_STR_LEN];
  char rhs_str[UBSAN_VAL_STR_LEN];

  __ubsan_val_to_string (lhs_str, data->lhs_type, lhs);
  __ubsan_val_to_string (rhs_str, data->rhs_type, rhs);

  if (ubsan_val_is_negative (data->rhs_type, rhs))
    __ubsan_error (&data->location,
		   "shift expoenent %s is negative\n",
		   rhs_str);
  else if (ubsan_get_unsigned_val (data->rhs_type, rhs) >=
	   ubsan_type_bit_width (data->lhs_type))
    __ubsan_error (&data->location,
		   "shift exponent %s is too large for %u-bit type %s\n",
		   rhs_str,
		   ubsan_type_bit_width (data->lhs_type),
		   data->lhs_type->type_name);
  else if (ubsan_val_is_negative (data->lhs_type, lhs))
    __ubsan_error (&data->location,
		   "left shift of negative valor %s\n",
		   lhs_str);
  else
    __ubsan_error (&data->location,
		   "left shift of %s by %s cannot be represented in type %s\n",
		   lhs_str,
		   rhs_str,
		   data->lhs_type->type_name);
}
rtld_hidden_def (__ubsan_handle_shift_out_of_bounds)
