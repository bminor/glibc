/* Atomic operations.  OpenRISC version.
   Copyright (C) 2022-2025 Free Software Foundation, Inc.
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

#ifndef __OR1K_ATOMIC_H_
#define __OR1K_ATOMIC_H_

#define __HAVE_64B_ATOMICS 0
#define ATOMIC_EXCHANGE_USES_CAS 1

#define atomic_full_barrier() ({ asm volatile ("l.msync" ::: "memory"); })

#endif /* atomic-machine.h */
