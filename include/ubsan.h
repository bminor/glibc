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

#ifndef __UBSAN_H__
#define __UBSAN_H__

#include <stdint.h>
#include <stdbool.h>
#include <endian.h>

#ifdef __SIZEOF_INT128__
typedef __int128 ubsan_s_max;
typedef unsigned __int128 ubsan_u_max;
#else
typedef int64_t ubsan_u_max;
typedef uint64_t ubsan_s_max;
#endif

#define REPORTED_BIT 31
#if (__WORDSIZE == 64 && BYTE_ORDER == BIG_ENDIAN)
# define COLUMN_MASK  (~(1U << REPORTED_BIT))
# define LINE_MASK    (~0U)
#else
# define COLUMN_MASK  (~0U)
# define LINE_MASK    (~(1U << REPORTED_BIT))
#endif

struct source_location
{
  const char *file_name;
  unsigned int line;
  unsigned int column;
};

static inline const char *
get_source_location_file_name (const struct source_location *location)
{
  return location->file_name ? location->file_name : "unknown";
}

static inline unsigned int
get_source_location_line (const struct source_location *location)
{
  return location->line & LINE_MASK;
}

static inline unsigned int
get_source_location_column (const struct source_location *location)
{
  return location->column & COLUMN_MASK;
}

struct nonnull_arg_data
{
  struct source_location location;
  struct source_location attr_location;
  int arg_index;
};

struct type_descriptor
{
  uint16_t type_kind;
  uint16_t type_info;
  char type_name[];
};

static inline bool
ubsan_type_is_signed (const struct type_descriptor *type)
{
  return type->type_kind & 1;
}

static inline unsigned int
ubsan_type_bit_width (const struct type_descriptor *type)
{
  return 1 << (type->type_info >> 1);
}

static inline bool
ubsan_is_inline_int (const struct type_descriptor *type)
{
  unsigned int inline_bits = sizeof (unsigned long) * 8;
  unsigned int bits = ubsan_type_bit_width (type);

  return bits <= inline_bits;
}

static inline ubsan_s_max
ubsan_get_signed_val (const struct type_descriptor *type, void *val)
{
  if (ubsan_is_inline_int (type))
    {
      unsigned int extra_bits = sizeof (ubsan_s_max) * 8
	- ubsan_type_bit_width (type);
      unsigned long ul_val = (unsigned long) val;
      return ((ubsan_s_max) ul_val) << extra_bits >> extra_bits;
    }

  if (ubsan_type_bit_width (type) == 64)
    return *(int64_t*) val;

  return *(ubsan_s_max *) val;
}

static inline ubsan_u_max
ubsan_get_unsigned_val (const struct type_descriptor *type, void *val)
{
  if (ubsan_is_inline_int (type))
    return (unsigned long) val;

  if (ubsan_type_bit_width (type) == 64)
    return *(uint64_t*) val;

  return *(ubsan_u_max *)val;
}

static inline bool
ubsan_val_is_negative (const struct type_descriptor *type, void *val)
{
  return ubsan_type_is_signed (type) && ubsan_get_signed_val (type, val) < 0;
}

struct invalid_value_data
{
  struct source_location location;
  struct type_descriptor *type;
};

/* The type_mismatch_data_v1::type_check_kind */
enum
{
  ubsan_type_check_load,
  ubsan_type_check_store,
  ubsan_type_check_reference_binding,
  ubsan_type_check_member_access,
  ubsan_type_check_member_call,
  ubsan_type_check_constructor_call,
  ubsan_type_check_downcast_pointer,
  ubsan_type_check_downcast_reference,
  ubsan_type_check_upcast,
  ubsan_type_check_upcast_to_virtual_base,
  ubsan_type_check_nonnull_assign,
  ubsan_type_check_dynamic_operation
};

struct type_mismatch_data_v1
{
  struct source_location location;
  struct type_descriptor *type;
  unsigned char log_alignment;
  unsigned char type_check_kind;
};

struct pointer_overflow_data
{
  struct source_location location;
};

struct overflow_data
{
  struct source_location location;
  struct type_descriptor *type;
};

struct out_of_bounds_data
{
  struct source_location location;
  struct type_descriptor *array_type;
  struct type_descriptor *index_type;
};

struct shift_out_of_bounds_data
{
  struct source_location location;
  struct type_descriptor *lhs_type;
  struct type_descriptor *rhs_type;
};

struct vla_bound_not_positive_data
{
  struct source_location location;
  struct type_descriptor *type;
};

struct unreachable_data
{
  struct source_location location;
};

struct invalid_builtin_data
{
  struct source_location location;
  unsigned char kind;
};

struct nonnull_return_data
{
  struct source_location location;
};

struct dynamic_type_cache_miss_data
{
  struct source_location location;
  struct type_descriptor *type;
  void *info;
  unsigned char kind;
};

enum
{
  ubsan_type_kind_int = 0,
  ubsan_type_kind_float = 1,
  ubsan_type_unknown = 0xffff
};

enum
{
  ubsan_builtin_check_kind_ctz_passed_zero,
  ubsan_builtin_check_kind_clz_passed_zero,
  ubsan_builtin_check_kind_assume_passed_false,
};

#define UBSAN_VAL_STR_LEN     32

void
__ubsan_val_to_string (char str[static UBSAN_VAL_STR_LEN],
		       struct type_descriptor *type, void *value)
  attribute_hidden;

#define UBSAN_VPTR_TYPE_CACHE_SIZE 128

extern unsigned int __ubsan_vptr_type_cache[UBSAN_VPTR_TYPE_CACHE_SIZE];

#if IS_IN(rtld)
# define ubsan_hidden attribute_hidden
#else
# define ubsan_hidden
#endif

void __ubsan_error (const struct source_location *source,
		    const char *fmt, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)))
  attribute_hidden;

void __ubsan_handle_overflow (const struct overflow_data *, void *,
			      void *, const char *op)
  attribute_hidden;

void __ubsan_handle_load_invalid_value (void *data, void *value)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_load_invalid_value)

void __ubsan_handle_type_mismatch_v1 (void *data, void *ptr)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_type_mismatch_v1)

void __ubsan_handle_pointer_overflow (void *data, void *val, void *result)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_pointer_overflow)

void __ubsan_handle_add_overflow (void *data, void *lhs, void *rhs)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_add_overflow)

void __ubsan_handle_sub_overflow (void *data, void *lhs, void *rhs)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_sub_overflow)

void __ubsan_handle_mul_overflow (void *data, void *lhs, void *rhs)
  attribute_hidden;
rtld_hidden_proto (__ubsan_handle_mul_overflow)

void __ubsan_handle_out_of_bounds (void *data, void *index)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_out_of_bounds)

void __ubsan_handle_negate_overflow (void *data, void *val)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_negate_overflow)

void __ubsan_handle_shift_out_of_bounds (void *_data, void *lhs, void *rhs)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_shift_out_of_bounds)

void __ubsan_handle_divrem_overflow (void *_data, void *lhs, void *rhs)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_divrem_overflow)

void __ubsan_handle_vla_bound_not_positive (void *data, void *bound)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_vla_bound_not_positive)

void __ubsan_handle_builtin_unreachable (void *data)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_builtin_unreachable)

void __ubsan_handle_invalid_builtin (void *data)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_invalid_builtin)

void __ubsan_handle_nonnull_arg (void *data)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_nonnull_arg);

void __ubsan_handle_nonnull_return_v1 (void *data, void *location)
  ubsan_hidden;
rtld_hidden_proto (__ubsan_handle_nonnull_return_v1)

void __ubsan_handle_dynamic_type_cache_miss (void *, void *, void *);
rtld_hidden_proto (__ubsan_handle_dynamic_type_cache_miss)

#endif /* __UBSAN_H__ */
