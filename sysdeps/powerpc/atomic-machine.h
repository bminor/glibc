/* Atomic operations.  PowerPC Common version.
   Copyright (C) 2003-2025 Free Software Foundation, Inc.
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

#ifndef _POWERPC_ATOMIC_MACHINE_H
#define _POWERPC_ATOMIC_MACHINE_H       1

#define ATOMIC_EXCHANGE_USES_CAS 1

/* Used on pthread_spin_{try}lock.  */
#define __ARCH_ACQ_INSTR        "isync"
#if defined _ARCH_PWR6 || defined _ARCH_PWR6X
# define MUTEX_HINT_ACQ	",1"
# define MUTEX_HINT_REL	",0"
#else
# define MUTEX_HINT_ACQ
# define MUTEX_HINT_REL
#endif

#endif
