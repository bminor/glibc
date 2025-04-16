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

#include <stddef.h>
#include <ubsan.h>

void
__ubsan_handle_pointer_overflow (void *_data, void *val, void *result)
{
  struct pointer_overflow_data *data = _data;

  if (val == NULL && result == NULL)
    __ubsan_error (&data->location,
		   "applying zero offset to a NULL pointer\n");
  else if (val == NULL && result != NULL)
    __ubsan_error (&data->location,
		   "applying non-zero offset to a NULL pointer\n");
  else if (val != NULL && result == NULL)
    __ubsan_error (&data->location,
		   "applying non-zero offset to non-NULL pointer 0x%0*lx "
		   "produced NULL pointer\n",
		   (int) sizeof (void *) * 2,
		   (unsigned long int) val);
  else if (((intptr_t)val >= 0) == ((intptr_t)result >= 0))
    {
      const char *operation = ((uintptr_t)val > (uintptr_t)result)
	? "addition" : "subtraction";

      __ubsan_error (&data->location,
		     "%s of unsigned offset to 0x%0*lx overflowed "
		     "to 0x%0*lx\n",
		     operation,
		     (int) sizeof (void *) * 2,
		     (unsigned long int) val,
		     (int) sizeof (void *) * 2,
		     (unsigned long int) result);
    }
  else
    __ubsan_error (&data->location,
		   "pointer index expression with base 0x%0*lx overflowed "
		   "to 0x%0*lx\n",
		   (int) sizeof (void *) * 2,
		   (unsigned long int) val,
		   (int) sizeof (void *) * 2,
		   (unsigned long int) result);
}
rtld_hidden_def (__ubsan_handle_pointer_overflow)
