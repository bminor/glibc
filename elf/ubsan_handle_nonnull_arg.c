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
__ubsan_handle_nonnull_arg (void *_data)
{
  struct nonnull_arg_data *data = _data;

  __ubsan_error (&data->location,
		 "null pointer passed as argument %u, nonnull attribute "
		 "declared at %s:%u:%u\n",
		 data->arg_index,
		 get_source_location_file_name (&data->attr_location),
		 get_source_location_line (&data->attr_location),
		 get_source_location_column (&data->attr_location));
}
rtld_hidden_def (__ubsan_handle_nonnull_arg)
