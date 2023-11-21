/* Adversarial test for qsort_r.
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

/* The approach follows Douglas McIlroy, A Killer Adversary for
   Quicksort.  Software—Practice and Experience 29 (1999) 341-344.
   Downloaded <http://www.cs.dartmouth.edu/~doug/mdmspe.pdf>
   (2023-11-17).  */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <support/check.h>
#include <support/support.h>

struct context
{
  /* Called the gas value in the paper.  This value is larger than all
     other values (length minus one will do), so comparison with any
     decided value has a known result.  */
  int undecided_value;

  /* If comparing undecided values, one of them as to be assigned a
     value to ensure consistency with future comparisons.  This is the
     value that will be used.  Starts out at zero.  */
  int next_decided;

  /* Used to trick pivot selection.  Deciding the value for the last
     seen undcided value in a decided/undecided comparison happens
     to trick the many qsort implementations.  */
  int last_undecided_index;

  /* This array contains the actually asigned values.  The call to
     qsort_r sorts a different array that contains indices into this
     array.  */
  int *decided_values;
};

static int
compare_opponent (const void *l1, const void *r1, void *ctx1)
{
  const int *l = l1;
  const int *r = r1;
  struct context *ctx = ctx1;
  int rvalue = ctx->decided_values[*r];
  int lvalue = ctx->decided_values[*l];

  if (lvalue == ctx->undecided_value)
    {
      if (rvalue == ctx->undecided_value)
        {
          /* Both values are undecided.  In this case, make a decision
             for the last-used undecided value.  This is tweak is very
             specific to quicksort.  */
          if (*l == ctx->last_undecided_index)
            {
              ctx->decided_values[*l] = ctx->next_decided;
              ++ctx->next_decided;
              /* The undecided value or *r is greater.  */
              return -1;
            }
          else
            {
              ctx->decided_values[*r] = ctx->next_decided;
              ++ctx->next_decided;
              /* The undecided value for *l is greater.  */
              return 1;
            }
        }
      else
        {
          ctx->last_undecided_index = *l;
          return 1;
        }
    }
  else
    {
      /* *l is a decided value.  */
      if (rvalue == ctx->undecided_value)
        {
          ctx->last_undecided_index = *r;
          /* The undecided value for *r is greater.  */
          return -1;
        }
      else
        return lvalue - rvalue;
    }
}

/* Return a pointer to the adversarial permutation of length N.  */
static int *
create_permutation (size_t n)
{
  struct context ctx =
    {
      .undecided_value = n - 1, /* Larger than all other values.  */
      .decided_values = xcalloc (n, sizeof (int)),
    };
  for (size_t i = 0; i < n; ++i)
    ctx.decided_values[i] = ctx.undecided_value;
  int *scratch = xcalloc (n, sizeof (int));
  for (size_t i = 0; i < n; ++i)
    scratch[i] = i;
  qsort_r (scratch, n, sizeof (*scratch), compare_opponent, &ctx);
  free (scratch);
  return ctx.decided_values;
}

/* Callback function for qsort which counts the number of invocations
   in *CLOSURE.  */
static int
compare_counter (const void *l1, const void *r1, void *closure)
{
  const int *l = l1;
  const int *r = r1;
  unsigned long long int *counter = closure;
  ++*counter;
  return *l - *r;
}

/* Count the comparisons required for an adversarial permutation of
   length N.  */
static unsigned long long int
count_comparisons (size_t n)
{
  int *array = create_permutation (n);
  unsigned long long int counter = 0;
  qsort_r (array, n, sizeof (*array), compare_counter, &counter);
  free (array);
  return counter;
}

/* Check the scaling factor for one adversarial permutation of length
   N, and report some statistics.  */
static void
check_one_n (size_t n)
{
  unsigned long long int count = count_comparisons (n);
  double factor = count / (n * log (count));
  printf ("info: length %zu: %llu comparisons ~ %f * n * log (n)\n",
          n, count, factor);
  /* This is an arbitrary factor which is true for the current
     implementation across a wide range of sizes.  */
  TEST_VERIFY (factor <= 4.5);
}

static int
do_test (void)
{
  check_one_n (100);
  check_one_n (1000);
  for (int i = 1; i <= 15; ++i)
    check_one_n (i * 10 * 1000);
  return 0;
}

#include <support/test-driver.c>
