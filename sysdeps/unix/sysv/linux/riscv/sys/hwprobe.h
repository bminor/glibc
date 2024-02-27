/* RISC-V architecture probe interface
   Copyright (C) 2024 Free Software Foundation, Inc.

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

#ifndef _SYS_HWPROBE_H
#define _SYS_HWPROBE_H 1

#include <features.h>
#include <stddef.h>
#ifdef __has_include
# if __has_include (<asm/hwprobe.h>)
#  include <asm/hwprobe.h>
# endif
#endif

/* Define a (probably stale) version of the interface if the Linux headers
   aren't present.  */
#ifndef RISCV_HWPROBE_KEY_MVENDORID
struct riscv_hwprobe {
	signed long long int key;
	unsigned long long int value;
};

#define RISCV_HWPROBE_KEY_MVENDORID 0
#define RISCV_HWPROBE_KEY_MARCHID 1
#define RISCV_HWPROBE_KEY_MIMPID 2
#define RISCV_HWPROBE_KEY_BASE_BEHAVIOR 3
#define  RISCV_HWPROBE_BASE_BEHAVIOR_IMA (1 << 0)
#define RISCV_HWPROBE_KEY_IMA_EXT_0 4
#define  RISCV_HWPROBE_IMA_FD (1 << 0)
#define  RISCV_HWPROBE_IMA_C (1 << 1)
#define  RISCV_HWPROBE_IMA_V (1 << 2)
#define  RISCV_HWPROBE_EXT_ZBA (1 << 3)
#define  RISCV_HWPROBE_EXT_ZBB (1 << 4)
#define  RISCV_HWPROBE_EXT_ZBS (1 << 5)
#define  RISCV_HWPROBE_EXT_ZICBOZ (1 << 6)
#define RISCV_HWPROBE_KEY_CPUPERF_0 5
#define  RISCV_HWPROBE_MISALIGNED_UNKNOWN (0 << 0)
#define  RISCV_HWPROBE_MISALIGNED_EMULATED (1 << 0)
#define  RISCV_HWPROBE_MISALIGNED_SLOW (2 << 0)
#define  RISCV_HWPROBE_MISALIGNED_FAST (3 << 0)
#define  RISCV_HWPROBE_MISALIGNED_UNSUPPORTED (4 << 0)
#define  RISCV_HWPROBE_MISALIGNED_MASK (7 << 0)
#define RISCV_HWPROBE_KEY_ZICBOZ_BLOCK_SIZE 6

#endif /* RISCV_HWPROBE_KEY_MVENDORID */

__BEGIN_DECLS

extern int __riscv_hwprobe (struct riscv_hwprobe *__pairs, size_t __pair_count,
			    size_t __cpu_count, unsigned long int *__cpus,
			    unsigned int __flags)
     __nonnull ((1)) __wur
     __fortified_attr_access (__read_write__, 1, 2)
     __fortified_attr_access (__read_only__, 4, 3);

/* A pointer to the __riscv_hwprobe vDSO function is passed as the second
   argument to ifunc selector routines. Include a function pointer type for
   convenience in calling the function in those settings. */
typedef int (*__riscv_hwprobe_t) (struct riscv_hwprobe *__pairs, size_t __pair_count,
				  size_t __cpu_count, unsigned long int *__cpus,
				  unsigned int __flags)
     __nonnull ((1)) __wur
     __fortified_attr_access (__read_write__, 1, 2)
     __fortified_attr_access (__read_only__, 4, 3);

__END_DECLS

#endif /* sys/hwprobe.h */
