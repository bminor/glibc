/* Copyright (C) 1991-2023 Free Software Foundation, Inc.
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

/* If you consider tuning this algorithm, you should consult first:
   Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
   Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993.  */

#include <limits.h>
#include <memswap.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Swap SIZE bytes between addresses A and B.  These helpers are provided
   along the generic one as an optimization.  */

enum swap_type_t
  {
    SWAP_WORDS_64,
    SWAP_WORDS_32,
    SWAP_BYTES
  };

/* If this function returns true, elements can be safely copied using word
   loads and stores.  Otherwise, it might not be safe.  BASE (as an integer)
   must be a multiple of the word alignment.  SIZE must be a multiple of
   WORDSIZE.  Since WORDSIZE must be a multiple of the word alignment, and
   WORDSIZE is a power of two on all supported platforms, this function for
   speed merely checks that BASE and SIZE are both multiples of the word
   size.  */
static inline bool
is_aligned (const void *base, size_t size, size_t wordsize)
{
  return (((uintptr_t) base | size) & (wordsize - 1)) == 0;
}

static inline void
swap_words_64 (void * restrict a, void * restrict b, size_t n)
{
  typedef uint64_t __attribute__ ((__may_alias__)) u64_alias_t;
  do
   {
     n -= 8;
     u64_alias_t t = *(u64_alias_t *)(a + n);
     *(u64_alias_t *)(a + n) = *(u64_alias_t *)(b + n);
     *(u64_alias_t *)(b + n) = t;
   } while (n);
}

static inline void
swap_words_32 (void * restrict a, void * restrict b, size_t n)
{
  typedef uint32_t __attribute__ ((__may_alias__)) u32_alias_t;
  do
   {
     n -= 4;
     u32_alias_t t = *(u32_alias_t *)(a + n);
     *(u32_alias_t *)(a + n) = *(u32_alias_t *)(b + n);
     *(u32_alias_t *)(b + n) = t;
   } while (n);
}

/* Replace the indirect call with a serie of if statements.  It should help
   the branch predictor.  */
static void
do_swap (void * restrict a, void * restrict b, size_t size,
	 enum swap_type_t swap_type)
{
  if (swap_type == SWAP_WORDS_64)
    swap_words_64 (a, b, size);
  else if (swap_type == SWAP_WORDS_32)
    swap_words_32 (a, b, size);
  else
    __memswap (a, b, size);
}

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    char *lo;
    char *hi;
    size_t depth;
  } stack_node;

/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */
enum { STACK_SIZE = CHAR_BIT * sizeof (size_t) };

static inline stack_node *
push (stack_node *top, char *lo, char *hi, size_t depth)
{
  top->lo = lo;
  top->hi = hi;
  top->depth = depth;
  return ++top;
}

static inline stack_node *
pop (stack_node *top, char **lo, char **hi, size_t *depth)
{
  --top;
  *lo = top->lo;
  *hi = top->hi;
  *depth = top->depth;
  return top;
}

/* NB: N is inclusive bound for BASE.  */
static inline void
siftdown (void *base, size_t size, size_t k, size_t n,
	  enum swap_type_t swap_type, __compar_d_fn_t cmp, void *arg)
{
  while (k <= n / 2)
    {
      size_t j = 2 * k;
      if (j < n && cmp (base + (j * size), base + ((j + 1) * size), arg) < 0)
	j++;

      if (j == k || cmp (base + (k * size), base + (j * size), arg) >= 0)
	break;

      do_swap (base + (size * j), base + (k * size), size, swap_type);
      k = j;
    }
}

static inline void
heapify (void *base, size_t size, size_t n, enum swap_type_t swap_type,
	 __compar_d_fn_t cmp, void *arg)
{
  size_t k = n / 2;
  while (1)
    {
      siftdown (base, size, k, n, swap_type, cmp, arg);
      if (k-- == 0)
	break;
    }
}

/* A non-recursive heapsort, used on introsort implementation as a fallback
   routine with worst-case performance of O(nlog n) and worst-case space
   complexity of O(1).  It sorts the array starting at BASE and ending at
   END, with each element of SIZE bytes.  The SWAP_TYPE is the callback
   function used to swap elements, and CMP is the function used to compare
   elements.   */
static void
heapsort_r (void *base, void *end, size_t size, enum swap_type_t swap_type,
	    __compar_d_fn_t cmp, void *arg)
{
  const size_t count = ((uintptr_t) end - (uintptr_t) base) / size;

  if (count < 2)
    return;

  size_t n = count - 1;

  /* Build the binary heap, largest value at the base[0].  */
  heapify (base, size, n, swap_type, cmp, arg);

  /* On each iteration base[0:n] is the binary heap, while base[n:count]
     is sorted.  */
  while (n > 0)
    {
      do_swap (base, base + (n * size), size, swap_type);
      n--;
      siftdown (base, size, 0, n, swap_type, cmp, arg);
    }
}

static inline void
insertion_sort_qsort_partitions (void *const pbase, size_t total_elems,
				 size_t size, enum swap_type_t swap_type,
				 __compar_d_fn_t cmp, void *arg)
{
  char *base_ptr = (char *) pbase;
  char *const end_ptr = &base_ptr[size * (total_elems - 1)];
  char *tmp_ptr = base_ptr;
#define min(x, y) ((x) < (y) ? (x) : (y))
  const size_t max_thresh = MAX_THRESH * size;
  char *thresh = min(end_ptr, base_ptr + max_thresh);
  char *run_ptr;

  /* Find smallest element in first threshold and place it at the
     array's beginning.  This is the smallest array element,
     and the operation speeds up insertion sort's inner loop. */

  for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
    if (cmp (run_ptr, tmp_ptr, arg) < 0)
      tmp_ptr = run_ptr;

  if (tmp_ptr != base_ptr)
    do_swap (tmp_ptr, base_ptr, size, swap_type);

  /* Insertion sort, running from left-hand-side up to right-hand-side.  */

  run_ptr = base_ptr + size;
  while ((run_ptr += size) <= end_ptr)
    {
      tmp_ptr = run_ptr - size;
      while (cmp (run_ptr, tmp_ptr, arg) < 0)
        tmp_ptr -= size;

      tmp_ptr += size;
      if (tmp_ptr != run_ptr)
        {
          char *trav;

          trav = run_ptr + size;
          while (--trav >= run_ptr)
            {
              char c = *trav;
              char *hi, *lo;

              for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                *hi = *lo;
              *hi = c;
            }
        }
    }
}

/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
      Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (total_elems)
      stack size is needed (actually O(1) in this case)!  */

void
__qsort_r (void *const pbase, size_t total_elems, size_t size,
	   __compar_d_fn_t cmp, void *arg)
{
  char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

  if (total_elems <= 1)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;

  enum swap_type_t swap_type;
  if (is_aligned (pbase, size, 8))
    swap_type = SWAP_WORDS_64;
  else if (is_aligned (pbase, size, 4))
    swap_type = SWAP_WORDS_32;
  else
    swap_type = SWAP_BYTES;

  /* Maximum depth before quicksort switches to heapsort.  */
  size_t depth = 2 * (sizeof (size_t) * CHAR_BIT - 1
		      - __builtin_clzl (total_elems));

  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = push (stack, NULL, NULL, depth);

      while (stack < top)
        {
          if (depth == 0)
            {
              heapsort_r (lo, hi, size, swap_type, cmp, arg);
              top = pop (top, &lo, &hi, &depth);
              continue;
            }

          char *left_ptr;
          char *right_ptr;

	  /* Select median value from among LO, MID, and HI. Rearrange
	     LO and HI so the three values are sorted. This lowers the
	     probability of picking a pathological pivot value and
	     skips a comparison for both the LEFT_PTR and RIGHT_PTR in
	     the while loops. */

	  char *mid = lo + size * ((hi - lo) / size >> 1);

	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    do_swap (mid, lo, size, swap_type);
	  if ((*cmp) ((void *) hi, (void *) mid, arg) < 0)
	    do_swap (mid, hi, size, swap_type);
	  else
	    goto jump_over;
	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    do_swap (mid, lo, size, swap_type);
	jump_over:;

	  left_ptr  = lo + size;
	  right_ptr = hi - size;

	  /* Here's the famous ``collapse the walls'' section of quicksort.
	     Gotta like those tight inner loops!  They are the main reason
	     that this algorithm runs much faster than others. */
	  do
	    {
	      while (left_ptr != mid
		     && (*cmp) ((void *) left_ptr, (void *) mid, arg) < 0)
		left_ptr += size;

	      while (right_ptr != mid
		     && (*cmp) ((void *) mid, (void *) right_ptr, arg) < 0)
		right_ptr -= size;

	      if (left_ptr < right_ptr)
		{
		  do_swap (left_ptr, right_ptr, size, swap_type);
		  if (mid == left_ptr)
		    mid = right_ptr;
		  else if (mid == right_ptr)
		    mid = left_ptr;
		  left_ptr += size;
		  right_ptr -= size;
		}
	      else if (left_ptr == right_ptr)
		{
		  left_ptr += size;
		  right_ptr -= size;
		  break;
		}
	    }
	  while (left_ptr <= right_ptr);

          /* Set up pointers for next iteration.  First determine whether
             left and right partitions are below the threshold size.  If so,
             ignore one or both.  Otherwise, push the larger partition's
             bounds on the stack and continue sorting the smaller one. */

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
		/* Ignore both small partitions. */
                top = pop (top, &lo, &hi, &depth);
              else
		/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
	    /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
	      /* Push larger left partition indices. */
              top = push (top, lo, right_ptr, depth - 1);
              lo = left_ptr;
            }
          else
            {
	      /* Push larger right partition indices. */
              top = push (top, left_ptr, hi, depth - 1);
              hi = right_ptr;
            }
        }
    }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */
  insertion_sort_qsort_partitions (pbase, total_elems, size, swap_type, cmp,
				   arg);
}
libc_hidden_def (__qsort_r)
weak_alias (__qsort_r, qsort_r)

void
qsort (void *b, size_t n, size_t s, __compar_fn_t cmp)
{
  return __qsort_r (b, n, s, (__compar_d_fn_t) cmp, NULL);
}
libc_hidden_def (qsort)
