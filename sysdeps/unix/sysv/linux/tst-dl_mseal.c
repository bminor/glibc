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

#include <support/xdlfcn.h>
#include <support/xthread.h>
#include <gnu/lib-names.h>
#include "tst-dl_mseal-common.h"

/* Check if memory sealing works as expected on multiples places:
   - On the binary itself.
   - On a LD_PRELOAD library.
   - On a dependency modules (tst-dl_mseal-mod-{1,2}.so).
   - On a audit modules (tst-dl_mseal-auditmod.so).
   - On a dlopen dependency opened with RTLD_NODELETE
     (tst-dl_mseal-dlopen-{2,2-1}.so).
   - On the libgcc_s opened by thread unwind.  */

/* Expected libraries that loader will seal.  */
static const char *expected_sealed_vmas[] =
{
#ifdef DEFAULT_MEMORY_SEAL
  "libc.so",
  "ld.so",
#endif
  "tst-dl_mseal",
  LIB_MODULE1,
  LIB_MODULE1_DEP,
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
  /* Auxiliary pages mapped by the kernel.  */
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

static void *
tf (void *closure)
{
  pthread_exit (NULL);
  return NULL;
}

static void
run_extra_steps (void)
{
  xdlopen (LIB_DLOPEN_NODELETE, RTLD_NOW | RTLD_NODELETE);
  xdlopen (LIB_DLOPEN_DEFAULT, RTLD_NOW);

  /* pthread_exit will load LIBGCC_S_SO.  */
  xpthread_join (xpthread_create (NULL, tf, NULL));
}

#define LD_PRELOAD LIB_PRELOAD
#define LD_AUDIT   LIB_AUDIT

#include "tst-dl_mseal-skeleton.c"
