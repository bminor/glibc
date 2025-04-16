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
__ubsan_handle_negate_overflow (void *_data, void *val)
{
  struct overflow_data *data = _data;
  char val_str[UBSAN_VAL_STR_LEN];

  __ubsan_val_to_string(val_str, data->type, val);

  __ubsan_error (&data->location,
		 "negation of %s cannot be represented in type %s\n",
		 val_str,
		 data->type->type_name);
}
rtld_hidden_def (__ubsan_handle_negate_overflow)
