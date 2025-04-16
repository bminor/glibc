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

static const char *
type_check_to_string (unsigned char type_check_kind)
{
  switch (type_check_kind)
    {
    case ubsan_type_check_load: return "load of";
    case ubsan_type_check_store: return "store to";
    case ubsan_type_check_reference_binding: return "reference binding to";
    case ubsan_type_check_member_access: return "member access within";
    case ubsan_type_check_member_call: return "member call on";
    case ubsan_type_check_constructor_call: return "constructor call on";
    case ubsan_type_check_downcast_pointer:
    case ubsan_type_check_downcast_reference: return "downcast of";
    case ubsan_type_check_upcast: return "upcast of";
    case ubsan_type_check_upcast_to_virtual_base: return "cast to virtual base of";
    case ubsan_type_check_nonnull_assign: return "_Nonnull binding to";
    case ubsan_type_check_dynamic_operation: return "dynamic operation on";
    default: return "unknown";
    }
}

static inline bool
is_misaligned_pointer (const struct type_mismatch_data_v1 *data, void *ptr)
{
  uintptr_t alignment = 1UL << data->log_alignment;
  return (uintptr_t) ptr & (alignment - 1);
}

void
__ubsan_handle_type_mismatch_v1 (void *_data, void *ptr)
{
  struct type_mismatch_data_v1 *data = _data;

  if (data->type_check_kind == ubsan_type_check_nonnull_assign)
    __ubsan_error (&data->location,
		   "%s null pointer of type %s\n",
		   type_check_to_string (data->type_check_kind),
		   data->type->type_name);
  else if (is_misaligned_pointer (data, ptr))
    __ubsan_error (&data->location,
		   "%s misaligned address 0x%0*lx for type %s\n",
		   type_check_to_string (data->type_check_kind),
		   (int) sizeof (void *) * 2,
		   (unsigned long int) ptr,
		   data->type->type_name);
  else
    __ubsan_error (&data->location,
		   "%s address 0x%0*lx with insufficient space for an "
		   "object of type %s\n",
		   type_check_to_string (data->type_check_kind),
		   (int) sizeof (void *) * 2,
		   (unsigned long int) ptr,
		   data->type->type_name);
}
rtld_hidden_def (__ubsan_handle_type_mismatch_v1)
