/* Measure memset function throughput with large data sizes.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
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

#define TEST_MAIN
#ifdef DO_MEMSET
# define TEST_NAME "memset"
#else
# define TEST_NAME "bzero"
#endif
#define START_SIZE 128
#define MIN_PAGE_SIZE (getpagesize () + 32 * 1024 * 1024)
#define TIMEOUT (20 * 60)
#include "bench-string.h"

#include "json-lib.h"

#ifdef DO_MEMSET
void *generic_memset (void *, int, size_t);

typedef void *(*proto_t) (void *, int, size_t);

IMPL (memset, 1)
IMPL (generic_memset, 0)

#else
static void
memset_zero (void * s, size_t len)
{
  memset (s, '\0', len);
}

typedef void (*proto_t) (void *, size_t);

IMPL (bzero, 1)
IMPL (memset_zero, 0)
#endif

static void
do_one_test (json_ctx_t *json_ctx, impl_t *impl, CHAR *s, CHAR *s_end,
	     size_t n)
{
  size_t i, iters = MIN_PAGE_SIZE / n;
  timing_t start, stop, cur;

  TIMING_NOW (start);
  for (i = 0; i < iters && s <= s_end; s_end -= n, i++)
#ifdef DO_MEMSET
    CALL (impl, s, 0, n);
#else
    CALL (impl, s, n);
#endif
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  /* Get time taken per function call.  */
  json_element_double (json_ctx, (double) cur / i);
}

static void
do_test (json_ctx_t *json_ctx, size_t len)
{
  json_element_object_begin (json_ctx);
  json_attr_uint (json_ctx, "length", len);
  json_array_begin (json_ctx, "timings");

  FOR_EACH_IMPL (impl, 0)
    {
      do_one_test (json_ctx, impl, (CHAR *) buf1,
		   (CHAR *) buf1 + MIN_PAGE_SIZE - len, len);
      alloc_bufs ();
    }

  json_array_end (json_ctx);
  json_element_object_end (json_ctx);
}

int
test_main (void)
{
  json_ctx_t json_ctx;
  size_t i;

  test_init ();

  json_init (&json_ctx, 0, stdout);

  json_document_begin (&json_ctx);
  json_attr_string (&json_ctx, "timing_type", TIMING_TYPE);

  json_attr_object_begin (&json_ctx, "functions");
  json_attr_object_begin (&json_ctx, TEST_NAME);
  json_attr_string (&json_ctx, "bench-variant", "walk");

  json_array_begin (&json_ctx, "ifuncs");
  FOR_EACH_IMPL (impl, 0)
    json_element_string (&json_ctx, impl->name);
  json_array_end (&json_ctx);

  json_array_begin (&json_ctx, "results");
  for (i = 1; i <= 64; i++)
    do_test (&json_ctx, i);

  for (i = START_SIZE; i <= MIN_PAGE_SIZE; i <<= 1)
    {
      do_test (&json_ctx, i);
      do_test (&json_ctx, i + 1);
    }

  json_array_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return ret;
}

#include <support/test-driver.c>

#ifdef DO_MEMSET
# define libc_hidden_builtin_def(X)
# define libc_hidden_def(X)
# define libc_hidden_weak(X)
# define weak_alias(X,Y)
# undef MEMSET
# define MEMSET generic_memset
# include <string/memset.c>
#endif
