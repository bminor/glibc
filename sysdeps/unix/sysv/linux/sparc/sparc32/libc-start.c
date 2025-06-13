/* Copyright (C) 1998-2025 Free Software Foundation, Inc.
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

#define LIBC_START_MAIN generic_start_main
#include <csu/libc-start.c>

STATIC int
__libc_start_main_impl (int (*main) (int, char **, char ** MAIN_AUXVEC_DECL),
			int argc, char **argv, void (*rtld_fini) (void),
			void *stack_end)
{
  return generic_start_main (main, argc, argv,
			     NULL, NULL, rtld_fini,
			     stack_end);
}
DEFINE_LIBC_START_MAIN_VERSION
