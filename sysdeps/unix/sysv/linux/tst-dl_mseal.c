/* Basic tests for sealing.
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

#include <gnu/lib-names.h>

/* Check if memory sealing works as expected on multiples places:
   - On the binary itself.
   - On a LD_PRELOAD library.
   - On a depedency modules (tst-dl_mseal-mod-{1,2}.so).
   - On a audit modules (tst-dl_mseal-auditmod.so).
   - On a dlopen dependency opened with RTLD_NODELET
     (tst-dl_mseal-dlopen-{2,2-1}.so).
   - On the libgcc_s opened by thread unwind.
*/

#define LIB_PRELOAD              "tst-dl_mseal-preload.so"
#define LIB_AUDIT                "tst-dl_mseal-auditmod.so"

#define LIB_DLOPEN_DEFAULT       "tst-dl_mseal-dlopen-1.so"
#define LIB_DLOPEN_DEFAULT_DEP   "tst-dl_mseal-dlopen-1-1.so"
#define LIB_DLOPEN_NODELETE      "tst-dl_mseal-dlopen-2.so"
#define LIB_DLOPEN_NODELETE_DEP  "tst-dl_mseal-dlopen-2-1.so"

/* Expected libraries that loader will seal.  */
static const char *expected_sealed_vmas[] =
{
#ifdef DEFAULT_MEMORY_SEAL
  "libc.so",
  "ld.so",
#endif
  "tst-dl_mseal",
  "tst-dl_mseal-mod-1.so",
  "tst-dl_mseal-mod-2.so",
  LIB_PRELOAD,
  LIB_AUDIT,
  LIB_DLOPEN_NODELETE,
};

/* Expected non sealed libraries.  */
static const char *expected_non_sealed_vmas[] =
{
#ifndef DEFAULT_MEMORY_SEAL
  "libc.so",
  "ld.so",
#endif
  LIB_DLOPEN_DEFAULT,
  LIB_DLOPEN_DEFAULT_DEP,
  LIB_DLOPEN_NODELETE_DEP,
  /* Auxiary pages mapped by the kernel.  */
  "[vdso]",
  "[sigpage]",
};

/* Special pages, either Auxiliary kernel pages where permission can not be
   changed or auxiliary libs that we can know prior hand that sealing is
   enabled.  */
static const char *expected_non_sealed_special[] =
{
   LIBGCC_S_SO,
  "[vectors]",
};

#include "tst-dl_mseal-skeleton.c"
