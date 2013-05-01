/* Convert between the NaCl ABI's `struct stat' format, and libc's.
   Copyright (C) 2013 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <nacl-interfaces.h>

struct stat;

/* We use this header to define struct nacl_abi_stat.  But we must avoid
   its excess declarations, and defining these names away is (marginally)
   cleaner than #undef'ing __native_client__.  */
#undef  stat
#define stat    __avoid_nacl_stat
#undef  fstat
#define fstat   __avoid_nacl_fstat
#include <native_client/src/trusted/service_runtime/include/sys/stat.h>
#undef  stat
#undef  fstat

extern int __xstat_conv (int vers, const struct nacl_abi_stat *, void *)
  internal_function attribute_hidden;
