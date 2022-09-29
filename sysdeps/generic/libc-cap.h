/* Support for allocations with narrow capability bounds.  Generic version.
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

#ifndef _LIBC_CAP_H
#define _LIBC_CAP_H 1

/* Larger allocations may need special alignment via cap_align.  */
#define __CAP_ALIGN_THRESHOLD SIZE_MAX

/* Error if the identifiers bellow are used.  */
void __libc_cap_link_error (void);

#define __libc_cap_fail(rtype) (__libc_cap_link_error (), (rtype) 0)
#define __libc_cap_init() __libc_cap_fail (bool)
#define __libc_cap_fork_lock() __libc_cap_fail (void)
#define __libc_cap_fork_unlock_parent() __libc_cap_fail (void)
#define __libc_cap_fork_unlock_child() __libc_cap_fail (void)
#define __libc_cap_map_add(p) __libc_cap_fail (bool)
#define __libc_cap_map_del(p) __libc_cap_fail (void)
#define __libc_cap_roundup(n) __libc_cap_fail (size_t)
#define __libc_cap_align(n) __libc_cap_fail (size_t)
#define __libc_cap_narrow(p, n) __libc_cap_fail (void *)
#define __libc_cap_widen(p) __libc_cap_fail (void *)
#define __libc_cap_reserve(p) __libc_cap_fail (bool)
#define __libc_cap_unreserve(p) __libc_cap_fail (void)
#define __libc_cap_drop(p) __libc_cap_fail (void)

#endif
