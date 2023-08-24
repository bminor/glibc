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

  IFUNC_IMPL (i, name, strnlen,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strnlen, SUPPORT_LASX, __strnlen_lasx)
	      IFUNC_IMPL_ADD (array, i, strnlen, SUPPORT_LSX, __strnlen_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strnlen, 1, __strnlen_aligned)
	      )

  IFUNC_IMPL (i, name, strchr,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strchr, SUPPORT_LASX, __strchr_lasx)
	      IFUNC_IMPL_ADD (array, i, strchr, SUPPORT_LSX, __strchr_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strchr, 1, __strchr_aligned)
	      )

  IFUNC_IMPL (i, name, strchrnul,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strchrnul, SUPPORT_LASX, __strchrnul_lasx)
	      IFUNC_IMPL_ADD (array, i, strchrnul, SUPPORT_LSX, __strchrnul_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strchrnul, 1, __strchrnul_aligned)
	      )

  IFUNC_IMPL (i, name, strcmp,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strcmp, SUPPORT_LSX, __strcmp_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strcmp, 1, __strcmp_aligned)
	      )

  IFUNC_IMPL (i, name, strncmp,
#if !defined __loongarch_soft_float
	      IFUNC_IMPL_ADD (array, i, strncmp, SUPPORT_LSX, __strncmp_lsx)
#endif
	      IFUNC_IMPL_ADD (array, i, strncmp, 1, __strncmp_aligned)
	      )

  IFUNC_IMPL (i, name, memcpy,
#if !defined __loongarch_soft_float
              IFUNC_IMPL_ADD (array, i, memcpy, SUPPORT_LASX, __memcpy_lasx)
              IFUNC_IMPL_ADD (array, i, memcpy, SUPPORT_LSX, __memcpy_lsx)
#endif
              IFUNC_IMPL_ADD (array, i, memcpy, SUPPORT_UAL, __memcpy_unaligned)
              IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_aligned)
              )

  IFUNC_IMPL (i, name, memmove,
#if !defined __loongarch_soft_float
              IFUNC_IMPL_ADD (array, i, memmove, SUPPORT_LASX, __memmove_lasx)
              IFUNC_IMPL_ADD (array, i, memmove, SUPPORT_LSX, __memmove_lsx)
#endif
              IFUNC_IMPL_ADD (array, i, memmove, SUPPORT_UAL, __memmove_unaligned)
              IFUNC_IMPL_ADD (array, i, memmove, 1, __memmove_aligned)
              )

  return i;
}
