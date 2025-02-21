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

#include <elf.h>
#include "tst-dl_mseal-mutable-mod.h"

static unsigned char mutable_array1[128]
  __attribute__ ((section (GNU_MUTABLE_SECTION_NAME)))
     = { 0 };
static unsigned char mutable_array2[256]
  __attribute__ ((section (GNU_MUTABLE_SECTION_NAME)))
     = { 0 };

static unsigned char immutable_array[256];

struct array_t
get_mutable_array1 (void)
{
  return (struct array_t) { mutable_array1, sizeof (mutable_array1) };
}

struct array_t
get_mutable_array2 (void)
{
  return (struct array_t) { mutable_array2, sizeof (mutable_array2) };
}

struct array_t
get_immutable_array (void)
{
  return (struct array_t) { immutable_array, sizeof (immutable_array) };
}
