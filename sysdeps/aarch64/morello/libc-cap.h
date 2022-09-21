/* Support for allocations with narrow capability bounds.  Morello version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#ifndef _AARCH64_MORELLO_LIBC_CAP_H
#define _AARCH64_MORELLO_LIBC_CAP_H 1

/* No special alignment is needed for n <= __CAP_ALIGN_THRESHOLD
   allocations, i.e. __libc_cap_align (n) <= MALLOC_ALIGNMENT.  */
#define __CAP_ALIGN_THRESHOLD 32759

/* Round up the allocation size so the allocated pointer bounds
   can be represented.  Note: this may be called before any
   checks on n, so it should work with all possible n values.  */
static __always_inline size_t
__libc_cap_roundup (size_t n)
{
  if (__glibc_unlikely (n > PTRDIFF_MAX))
    return n;
  return __builtin_cheri_round_representable_length (n);
}

/* Returns the alignment requirement for an allocation size n such
   that the allocated pointer bounds can be represented.  Note:
   same n should be used in __libc_cap_roundup and __libc_cap_align
   and the results used together for __libc_cap_narrow.  */
static __always_inline size_t
__libc_cap_align (size_t n)
{
  return -__builtin_cheri_representable_alignment_mask (n);
}

/* Narrow the bounds of p to be [p, p+n).  Note: the original bounds
   of p includes the entire mapping where p points to and that bound
   should be recoverable by __libc_cap_widen.  Called with p aligned
   and n sized according to __libc_cap_align and __libc_cap_roundup.  */
static __always_inline void *
__libc_cap_narrow (void *p, size_t n)
{
  return __builtin_cheri_bounds_set_exact (p, n);
}

/* Given a p with narrowed bound (output of __libc_cap_narrow) return
   the same pointer with the internal wide bound.  */
static __always_inline void *
__libc_cap_widen (void *p)
{
  void *cap = __builtin_cheri_global_data_get ();
  return __builtin_cheri_address_set (cap, p);
}

#endif
