/* Enumerate available IFUNC implementations of a function. s390/s390x version.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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
#include <ifunc-impl-list.h>
#include <ifunc-resolve.h>
#include <ifunc-memset.h>
#include <ifunc-memcmp.h>
#include <ifunc-memcpy.h>
#include <ifunc-strstr.h>
#include <ifunc-memmem.h>
#include <ifunc-strlen.h>
#include <ifunc-strnlen.h>
#include <ifunc-strcpy.h>
#include <ifunc-stpcpy.h>
#include <ifunc-strncpy.h>
#include <ifunc-stpncpy.h>

/* Maximum number of IFUNC implementations.  */
#define MAX_IFUNC	3

/* Fill ARRAY of MAX elements with IFUNC implementations for function
   NAME supported on target machine and return the number of valid
   entries.  */
size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{
  assert (max >= MAX_IFUNC);

  size_t i = 0;

  /* Get hardware information.  */
  unsigned long int dl_hwcap = GLRO (dl_hwcap);
  unsigned long long stfle_bits = 0ULL;
  if ((dl_hwcap & HWCAP_S390_STFLE)
	&& (dl_hwcap & HWCAP_S390_ZARCH)
	&& (dl_hwcap & HWCAP_S390_HIGH_GPRS))
    {
      S390_STORE_STFLE (stfle_bits);
    }

#if HAVE_MEMSET_IFUNC
  IFUNC_IMPL (i, name, memset,
# if HAVE_MEMSET_Z196
	      IFUNC_IMPL_ADD (array, i, memset,
			      S390_IS_Z196 (stfle_bits), MEMSET_Z196)
# endif
# if HAVE_MEMSET_Z10
	      IFUNC_IMPL_ADD (array, i, memset,
			      S390_IS_Z10 (stfle_bits), MEMSET_Z10)
# endif
# if HAVE_MEMSET_Z900_G5
	      IFUNC_IMPL_ADD (array, i, memset, 1, MEMSET_Z900_G5)
# endif
	      )

  /* Note: bzero is implemented in memset.  */
  IFUNC_IMPL (i, name, bzero,
# if HAVE_MEMSET_Z196
	      IFUNC_IMPL_ADD (array, i, bzero,
			      S390_IS_Z196 (stfle_bits), BZERO_Z196)
# endif
# if HAVE_MEMSET_Z10
	      IFUNC_IMPL_ADD (array, i, bzero,
			      S390_IS_Z10 (stfle_bits), BZERO_Z10)
# endif
# if HAVE_MEMSET_Z900_G5
	      IFUNC_IMPL_ADD (array, i, bzero, 1, BZERO_Z900_G5)
# endif
	      )
#endif /* HAVE_MEMSET_IFUNC */

#if HAVE_MEMCMP_IFUNC
  IFUNC_IMPL (i, name, memcmp,
# if HAVE_MEMCMP_Z196
	      IFUNC_IMPL_ADD (array, i, memcmp,
			      S390_IS_Z196 (stfle_bits), MEMCMP_Z196)
# endif
# if HAVE_MEMCMP_Z10
	      IFUNC_IMPL_ADD (array, i, memcmp,
			      S390_IS_Z10 (stfle_bits), MEMCMP_Z10)
# endif
# if HAVE_MEMCMP_Z900_G5
	      IFUNC_IMPL_ADD (array, i, memcmp, 1, MEMCMP_Z900_G5)
# endif
	      )
#endif /* HAVE_MEMCMP_IFUNC */

#if HAVE_MEMCPY_IFUNC
  IFUNC_IMPL (i, name, memcpy,
# if HAVE_MEMCPY_Z196
	      IFUNC_IMPL_ADD (array, i, memcpy,
			      S390_IS_Z196 (stfle_bits), MEMCPY_Z196)
# endif
# if HAVE_MEMCPY_Z10
	      IFUNC_IMPL_ADD (array, i, memcpy,
			      S390_IS_Z10 (stfle_bits), MEMCPY_Z10)
# endif
# if HAVE_MEMCPY_Z900_G5
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, MEMCPY_Z900_G5)
# endif
	      )

  IFUNC_IMPL (i, name, mempcpy,
# if HAVE_MEMCPY_Z196
	      IFUNC_IMPL_ADD (array, i, mempcpy,
			      S390_IS_Z196 (stfle_bits), MEMPCPY_Z196)
# endif
# if HAVE_MEMCPY_Z10
	      IFUNC_IMPL_ADD (array, i, mempcpy,
			      S390_IS_Z10 (stfle_bits), MEMPCPY_Z10)
# endif
# if HAVE_MEMCPY_Z900_G5
	      IFUNC_IMPL_ADD (array, i, mempcpy, 1, MEMPCPY_Z900_G5)
# endif
	      )
#endif /* HAVE_MEMCPY_IFUNC  */

#if HAVE_MEMMOVE_IFUNC
    IFUNC_IMPL (i, name, memmove,
# if HAVE_MEMMOVE_Z13
		IFUNC_IMPL_ADD (array, i, memmove,
				dl_hwcap & HWCAP_S390_VX, MEMMOVE_Z13)
# endif
# if HAVE_MEMMOVE_C
		IFUNC_IMPL_ADD (array, i, memmove, 1, MEMMOVE_C)
# endif
		)
#endif /* HAVE_MEMMOVE_IFUNC  */

#if HAVE_STRSTR_IFUNC
    IFUNC_IMPL (i, name, strstr,
# if HAVE_STRSTR_Z13
		IFUNC_IMPL_ADD (array, i, strstr,
				dl_hwcap & HWCAP_S390_VX, STRSTR_Z13)
# endif
# if HAVE_STRSTR_C
		IFUNC_IMPL_ADD (array, i, strstr, 1, STRSTR_C)
# endif
		)
#endif /* HAVE_STRSTR_IFUNC  */

#if HAVE_MEMMEM_IFUNC
    IFUNC_IMPL (i, name, memmem,
# if HAVE_MEMMEM_Z13
		IFUNC_IMPL_ADD (array, i, memmem,
				dl_hwcap & HWCAP_S390_VX, MEMMEM_Z13)
# endif
# if HAVE_MEMMEM_C
		IFUNC_IMPL_ADD (array, i, memmem, 1, MEMMEM_C)
# endif
		)
#endif /* HAVE_MEMMEM_IFUNC  */

#if HAVE_STRLEN_IFUNC
    IFUNC_IMPL (i, name, strlen,
# if HAVE_STRLEN_Z13
		IFUNC_IMPL_ADD (array, i, strlen,
				dl_hwcap & HWCAP_S390_VX, STRLEN_Z13)
# endif
# if HAVE_STRLEN_C
		IFUNC_IMPL_ADD (array, i, strlen, 1, STRLEN_C)
# endif
		)
#endif /* HAVE_STRLEN_IFUNC  */

#if HAVE_STRNLEN_IFUNC
    IFUNC_IMPL (i, name, strnlen,
# if HAVE_STRNLEN_Z13
		IFUNC_IMPL_ADD (array, i, strnlen,
				dl_hwcap & HWCAP_S390_VX, STRNLEN_Z13)
# endif
# if HAVE_STRNLEN_C
		IFUNC_IMPL_ADD (array, i, strnlen, 1, STRNLEN_C)
# endif
		)
#endif /* HAVE_STRNLEN_IFUNC  */

#if HAVE_STRCPY_IFUNC
    IFUNC_IMPL (i, name, strcpy,
# if HAVE_STRCPY_Z13
		IFUNC_IMPL_ADD (array, i, strcpy,
				dl_hwcap & HWCAP_S390_VX, STRCPY_Z13)
# endif
# if HAVE_STRCPY_Z900_G5
		IFUNC_IMPL_ADD (array, i, strcpy, 1, STRCPY_Z900_G5)
# endif
		)
#endif /* HAVE_STRCPY_IFUNC  */

#if HAVE_STPCPY_IFUNC
    IFUNC_IMPL (i, name, stpcpy,
# if HAVE_STPCPY_Z13
		IFUNC_IMPL_ADD (array, i, stpcpy,
				dl_hwcap & HWCAP_S390_VX, STPCPY_Z13)
# endif
# if HAVE_STPCPY_C
		IFUNC_IMPL_ADD (array, i, stpcpy, 1, STPCPY_C)
# endif
		)
#endif /* HAVE_STPCPY_IFUNC  */

#if HAVE_STRNCPY_IFUNC
    IFUNC_IMPL (i, name, strncpy,
# if HAVE_STRNCPY_Z13
		IFUNC_IMPL_ADD (array, i, strncpy,
				dl_hwcap & HWCAP_S390_VX, STRNCPY_Z13)
# endif
# if HAVE_STRNCPY_Z900_G5
		IFUNC_IMPL_ADD (array, i, strncpy, 1, STRNCPY_Z900_G5)
# endif
		)
#endif /* HAVE_STRNCPY_IFUNC  */

#if HAVE_STPNCPY_IFUNC
    IFUNC_IMPL (i, name, stpncpy,
# if HAVE_STPNCPY_Z13
		IFUNC_IMPL_ADD (array, i, stpncpy,
				dl_hwcap & HWCAP_S390_VX, STPNCPY_Z13)
# endif
# if HAVE_STPNCPY_C
		IFUNC_IMPL_ADD (array, i, stpncpy, 1, STPNCPY_C)
# endif
		)
#endif /* HAVE_STPNCPY_IFUNC  */


#ifdef HAVE_S390_VX_ASM_SUPPORT

# define IFUNC_VX_IMPL(FUNC)						\
  IFUNC_IMPL (i, name, FUNC,						\
	      IFUNC_IMPL_ADD (array, i, FUNC, dl_hwcap & HWCAP_S390_VX, \
			      __##FUNC##_vx)				\
	      IFUNC_IMPL_ADD (array, i, FUNC, 1, __##FUNC##_c))

  IFUNC_VX_IMPL (wcslen);

  IFUNC_VX_IMPL (wcsnlen);

  IFUNC_VX_IMPL (wcscpy);

  IFUNC_VX_IMPL (wcpcpy);

  IFUNC_VX_IMPL (wcsncpy);

  IFUNC_VX_IMPL (wcpncpy);

  IFUNC_VX_IMPL (strcat);
  IFUNC_VX_IMPL (wcscat);

  IFUNC_VX_IMPL (strncat);
  IFUNC_VX_IMPL (wcsncat);

  IFUNC_VX_IMPL (strcmp);
  IFUNC_VX_IMPL (wcscmp);

  IFUNC_VX_IMPL (strncmp);
  IFUNC_VX_IMPL (wcsncmp);

  IFUNC_VX_IMPL (strchr);
  IFUNC_VX_IMPL (wcschr);

  IFUNC_VX_IMPL (strchrnul);
  IFUNC_VX_IMPL (wcschrnul);

  IFUNC_VX_IMPL (strrchr);
  IFUNC_VX_IMPL (wcsrchr);

  IFUNC_VX_IMPL (strspn);
  IFUNC_VX_IMPL (wcsspn);

  IFUNC_VX_IMPL (strpbrk);
  IFUNC_VX_IMPL (wcspbrk);

  IFUNC_VX_IMPL (strcspn);
  IFUNC_VX_IMPL (wcscspn);

  IFUNC_VX_IMPL (memchr);
  IFUNC_VX_IMPL (wmemchr);
  IFUNC_VX_IMPL (rawmemchr);

  IFUNC_VX_IMPL (memccpy);

  IFUNC_VX_IMPL (wmemset);

  IFUNC_VX_IMPL (wmemcmp);

  IFUNC_VX_IMPL (memrchr);

#endif /* HAVE_S390_VX_ASM_SUPPORT */

  return i;
}
