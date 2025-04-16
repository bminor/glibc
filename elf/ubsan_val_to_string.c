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

#include <intprops.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ubsan.h"

static const char lower_digits[] = "0123456789";

enum { BASE = 10 };

static char *
utoa (unsigned long long int value, char *buf, size_t len)
{
  if (len == 0)
    return buf;

  char *ptr = buf;
  do
    {
      if (len-- == 0)
	break;
      *ptr++ = lower_digits[value % BASE];
      value /= BASE;
    }
  while (value != 0);
  char *r = ptr;
  *ptr-- = '\0';

  while (buf < ptr)
    {
      char t = *ptr;
      *ptr-- = *buf;
      *buf++ = t;
    }

  return r;
}

static char *
itoa (long long int value, char *buf, size_t len)
{
  if (len == 0)
    return buf;

  bool isneg = value < 0;
  char *ptr = buf;
  do
    {
      if (len-- == 0)
	break;
      *ptr++ = lower_digits[abs (value % BASE)];
      value /= BASE;
    }
  while (value != 0);
  if (isneg)
    *ptr++ = '-';
  char *r = ptr;
  *ptr-- = '\0';

  while (buf < ptr)
    {
      char t = *ptr;
      *ptr-- = *buf;
      *buf++ = t;
    }

  return r;
}

static long long int
ubsan_val_to_ll (int width, void *value, long long int def)
{
  switch (width)
    {
    case 8:
      return (int8_t) (intptr_t) value;
    case 16:
      return (int16_t) (intptr_t) value;
    case 32:
      if (sizeof (value) >= sizeof (int32_t))
	return (int32_t) (intptr_t) value;
      else
	return *(int32_t *) value;
    case 64:
      if (sizeof (value) >= sizeof (int64_t))
	return (int64_t) (intptr_t) value;
      else
	return *(int64_t *) value;
    default:
      return def;
    }
}

static unsigned long long int
ubsan_val_to_ull (int width, void *value, unsigned long long int def)
{
  switch (width)
    {
    case 8:
      return (uint8_t) (uintptr_t) value;
    case 16:
      return (uint16_t) (uintptr_t) value;
    case 32:
      if (sizeof (value) >= sizeof (uint32_t))
	return (uint32_t) (uintptr_t) value;
      else
	return *(uint32_t *) value;
    case 64:
      if (sizeof (value) >= sizeof (uint64_t))
	return (uint64_t) (uintptr_t) value;
      else
	return *(uint64_t *) value;
    default:
      return def;
    }
}

static inline char *
add_string (char *str, const char *s, size_t *len)
{
  char *endp = __stpncpy (str, s, *len);
  *len -= endp - str;
  return endp;
}

static inline char *
add_uint (char *str, unsigned long long int value, size_t *len)
{
  char *endp = utoa (value, str, *len);
  *len -= endp - str;
  return endp;
}

void
__ubsan_val_to_string (char str[static UBSAN_VAL_STR_LEN],
		       struct type_descriptor *type, void *value)
{
  int width = ubsan_type_bit_width (type);
  switch (type->type_kind)
    {
    case ubsan_type_kind_int:
      if (ubsan_type_is_signed (type))
	{
	  long long int v = ubsan_val_to_ll (width, value, 0);
	  itoa (v, str, UBSAN_VAL_STR_LEN);
	}
      else
	{
	  unsigned long long int v = ubsan_val_to_ull (width, value, 0);
	  utoa (v, str, UBSAN_VAL_STR_LEN);
	}
      break;
    case ubsan_type_kind_float:
      {
	char *endp = __stpcpy (str, "float size ");
	utoa (width, endp, UBSAN_VAL_STR_LEN);
      }
      break;
    default:
      {
	size_t size = UBSAN_VAL_STR_LEN;
	char *endp = add_string (str, "kind ", &size);
	endp = add_uint (endp, type->type_kind, &size);
	endp = add_string (endp, " (width ", &size);
	endp = add_uint (endp, width, &size);
	add_string (endp, " )", &size);
      }
      break;
    }
}
