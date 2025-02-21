/* Check if PT_OPENBSD_MUTABLE is correctly applied.
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

#define LIB_DLOPEN "tst-dl_mseal-mutable-dlopen.so"

struct array_t
{
  unsigned char *arr;
  size_t size;
};

typedef struct array_t (*get_array_t)(void);

struct array_t get_mutable_array1 (void);
struct array_t get_mutable_array2 (void);
struct array_t get_immutable_array (void);
