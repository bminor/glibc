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

/*
 * Never include sysdeps/powerpc/atomic-machine.h directly.
 * Always use include/atomic.h which will include either
 * sysdeps/powerpc/powerpc32/atomic-machine.h
 * or
 * sysdeps/powerpc/powerpc64/atomic-machine.h
 * as appropriate and which in turn include this file.
 */

#define __ARCH_ACQ_INSTR	"isync"
#ifndef __ARCH_REL_INSTR
# define __ARCH_REL_INSTR	"sync"
#endif

#ifndef MUTEX_HINT_ACQ
# define MUTEX_HINT_ACQ
#endif
#ifndef MUTEX_HINT_REL
# define MUTEX_HINT_REL
#endif

#define atomic_full_barrier()	__asm ("sync" ::: "memory")
