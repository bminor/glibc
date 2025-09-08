/* Copyright (C) 2010-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _ATOMIC_MACHINE_H
#define _ATOMIC_MACHINE_H	1

#include <sysdep.h>

/* Coldfire has no atomic compare-and-exchange operation, but the
   kernel provides userspace atomicity operations.  Use them.  */

#define __HAVE_64B_ATOMICS 0
#define USE_ATOMIC_COMPILER_BUILTINS 1

/* XXX Is this actually correct?  */
#define ATOMIC_EXCHANGE_USES_CAS 1

# define atomic_full_barrier()				\
  (INTERNAL_SYSCALL_CALL (atomic_barrier), (void) 0)

#endif
