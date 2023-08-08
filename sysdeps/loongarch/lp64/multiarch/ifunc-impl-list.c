/* Enumerate available IFUNC implementations of a function LoongArch64 version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <ldsodefs.h>
#include <ifunc-impl-list.h>
#include <stdio.h>

size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{

  size_t i = max;

  IFUNC_IMPL (i, name, strlen,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strlen, SUPPORT_LASX, __strlen_lasx)
	      IFUNC_IMPL_ADD (array, i, strlen, SUPPORT_LSX, __strlen_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strlen, 1, __strlen_aligned)
	      )
  return i;
}
