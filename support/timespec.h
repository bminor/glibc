/* Useful functions for tests that use struct timespec.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#ifndef SUPPORT_TIMESPEC_H
#define SUPPORT_TIMESPEC_H

#include <stdio.h>
#include <time.h>
#include <support/check.h>
#include <support/xtime.h>

struct timespec timespec_add (struct timespec, struct timespec)
  __attribute__((const));
struct timespec timespec_sub (struct timespec, struct timespec)
  __attribute__((const));

static inline struct timespec
make_timespec (time_t s, long int ns)
{
  struct timespec r;
  r.tv_sec = s;
  r.tv_nsec = ns;
  return r;
}

enum { TIMESPEC_HZ = 1000000000 };

void test_timespec_before_impl (const char *file, int line,
                                const struct timespec left,
                                const struct timespec right);

void test_timespec_equal_or_after_impl (const char *file, int line,
                                        const struct timespec left,
                                        const struct timespec right);

/* Check that the timespec on the left represents a time before the
   time on the right. */
#define TEST_TIMESPEC_BEFORE(left, right)                               \
  test_timespec_before_impl (__FILE__, __LINE__, (left), (right))

#define TEST_TIMESPEC_BEFORE_NOW(left, clockid)                 \
  ({                                                            \
    struct timespec now;                                        \
    const int saved_errno = errno;                              \
    xclock_gettime ((clockid), &now);                           \
    TEST_TIMESPEC_BEFORE ((left), now);                         \
    errno = saved_errno;                                        \
  })

/* Check that the timespec on the left represents a time equal to or
   after the time on the right. */
#define TEST_TIMESPEC_EQUAL_OR_AFTER(left, right)                       \
  test_timespec_equal_or_after_impl (__FILE__, __LINE__, left, right)

#define TEST_TIMESPEC_NOW_OR_AFTER(clockid, right)              \
  ({                                                            \
    struct timespec now;                                        \
    const int saved_errno = errno;                              \
    xclock_gettime ((clockid), &now);                           \
    TEST_TIMESPEC_EQUAL_OR_AFTER (now, (right));                \
    errno = saved_errno;                                        \
  })

#endif /* SUPPORT_TIMESPEC_H */
