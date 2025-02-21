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

#include <dlfcn.h>
#include <errno.h>
#include <link.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include <libc-pointer-arith.h>
#include <support/check.h>
#include <support/test-driver.h>
#include <support/xdlfcn.h>
#include <support/xsignal.h>
#include <support/xunistd.h>

#include "tst-dl_mseal-mutable-mod.h"

static long int pagesize;

/* To check if the protection flags are correctly set, the thread tries
   read/writes on it and checks if a SIGSEGV is generated.  */

static volatile sig_atomic_t signal_jump_set;
static sigjmp_buf signal_jmp_buf;

static void
sigsegv_handler (int sig)
{
  if (signal_jump_set == 0)
    return;

  siglongjmp (signal_jmp_buf, sig);
}

static bool
try_access_buf (unsigned char *ptr, bool write)
{
  signal_jump_set = true;

  bool failed = sigsetjmp (signal_jmp_buf, 0) != 0;
  if (!failed)
    {
      if (write)
	*(volatile unsigned char *)(ptr) = 'x';
      else
	*(volatile unsigned char *)(ptr);
    }

  signal_jump_set = false;
  return !failed;
}

struct range_t
{
  const char *name;
  unsigned char *start;
  size_t size;
  bool found;
};

static int
callback (struct dl_phdr_info *info, size_t size, void *data)
{
  struct range_t *range = data;
  if (strcmp (info->dlpi_name, range->name) != 0)
    return 0;

  for (size_t i = 0; i < info->dlpi_phnum; i++)
    if (info->dlpi_phdr[i].p_type == PT_GNU_MUTABLE)
      {
	range->start = (unsigned char *) info->dlpi_phdr[i].p_vaddr;
	range->size = info->dlpi_phdr[i].p_memsz;
	range->found = true;
	break;
      }

  return 0;
}

static bool
find_mutable_range (void *addr, struct range_t *range)
{
  struct dl_find_object dlfo;
  if (_dl_find_object (addr, &dlfo) != 0)
    return false;

  range->name = dlfo.dlfo_link_map->l_name;
  range->found = false;
  dl_iterate_phdr (callback, range);
  if (range->found)
    range->start = dlfo.dlfo_link_map->l_addr + range->start;

  return range->found;
}

static bool
__attribute_used__
try_read_buf (unsigned char *ptr)
{
  return try_access_buf (ptr, false);
}

static bool
__attribute_used__
try_write_buf (unsigned char *ptr)
{
  return try_access_buf (ptr, true);
}

/* The GNU_MUTABLE_SECTION_NAME section is page-aligned and with a size
   multiple of page size.  */

unsigned char mutable_array1[64]
  __attribute__ ((section (GNU_MUTABLE_SECTION_NAME)))
     = { 0 };
unsigned char mutable_array2[32]
  __attribute__ ((section (GNU_MUTABLE_SECTION_NAME)))
     = { 0 };

unsigned char immutable_array[128];

static void
check_array (struct array_t *arr)
{
  TEST_COMPARE (try_write_buf (arr->arr), false);
  TEST_COMPARE (try_write_buf (&arr->arr[arr->size/2]), false);
  TEST_COMPARE (try_write_buf (&arr->arr[arr->size-1]), false);
}

static void
check_mutable (struct array_t *mut1,
	       struct array_t *mut2,
	       struct array_t *imut)
{
  struct range_t range1;
  struct range_t range2;

  TEST_VERIFY_EXIT (find_mutable_range (mut1->arr, &range1));
  TEST_VERIFY (mut1->arr >= range1.start);
  TEST_VERIFY (mut1->arr + mut1->size <= range1.start + range1.size);

  TEST_VERIFY_EXIT (find_mutable_range (mut2->arr, &range2));
  TEST_VERIFY (mut2->arr >= range2.start);
  TEST_VERIFY (mut2->arr + mut2->size <= range2.start + range2.size);

  /* Assume that both array will be placed in the same page since their
     combined size is less than pagesize.  */
  TEST_VERIFY (range1.start == range2.start);
  TEST_VERIFY (range2.size == range2.size);

  if (test_verbose > 0)
    printf ("mutable region: %-30s - %p-%p\n",
	    range1.name[0] == '\0' ? "main program" : basename (range1.name),
	    range1.start,
	    range1.start + range1.size);

  memset (mut1->arr, 0xaa, mut1->size);
  memset (mut2->arr, 0xbb, mut1->size);
  memset (imut->arr, 0xcc, imut->size);

  /* Sanity check, imut should be immutable.  */
  {
    void *start = PTR_ALIGN_DOWN (imut->arr, pagesize);
    TEST_COMPARE (mprotect (start, pagesize, PROT_READ), -1);
    TEST_COMPARE (errno, EPERM);
  }

  /* Change permission of mutable region to just allow read.  */
  xmprotect ((void *)range1.start, range1.size, PROT_READ);

  check_array (mut1);
  check_array (mut2);
}

static int
do_test (void)
{
  pagesize = xsysconf (_SC_PAGESIZE);

  {
    struct sigaction sa = {
      .sa_handler = sigsegv_handler,
      .sa_flags = SA_NODEFER,
    };
    sigemptyset (&sa.sa_mask);
    xsigaction (SIGSEGV, &sa, NULL);
  }

#define ARR_TO_RANGE(__arr) \
  &((struct array_t) { __arr, sizeof (__arr) })

  check_mutable (ARR_TO_RANGE (mutable_array1),
		 ARR_TO_RANGE (mutable_array2),
		 ARR_TO_RANGE (immutable_array));

#ifndef TEST_STATIC
  {
    struct array_t mut1 = get_mutable_array1 ();
    struct array_t mut2 = get_mutable_array2 ();
    struct array_t imut = get_immutable_array ();
    check_mutable (&mut1, &mut2, &imut);
  }

  {
    void *h = xdlopen (LIB_DLOPEN, RTLD_NOW | RTLD_NODELETE);

#define GET_ARRAY_DLOPEN(__name) \
    ({ \
       get_array_t f = xdlsym (h, __name); \
       f(); \
    })

    struct array_t mut1 = GET_ARRAY_DLOPEN ("get_mutable_array1");
    struct array_t mut2 = GET_ARRAY_DLOPEN ("get_mutable_array2");
    struct array_t imut = GET_ARRAY_DLOPEN ("get_immutable_array");
    check_mutable (&mut1, &mut2, &imut);
  }
#endif

  return 0;
}

#include <support/test-driver.c>
