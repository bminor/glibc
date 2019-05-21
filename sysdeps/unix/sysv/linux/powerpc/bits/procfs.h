/* Types for registers for sys/procfs.h.  PowerPC version.
   Copyright (C) 1996-2020 Free Software Foundation, Inc.
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

#ifndef _BITS_PROCFS_H
#define _BITS_PROCFS_H 1

#ifndef _SYS_PROCFS_H
# error "Never include <bits/procfs.h> directly; use <sys/procfs.h> instead."
#endif

#include <bits/wordsize.h>
#include <asm/elf.h>

/* These definitions may have been provided by asm/elf.h.  Otherwise
   we define them here.  */
#ifndef ELF_NGREG

#define ELF_NGREG       48      /* includes nip, msr, lr, etc. */
#define ELF_NFPREG      33      /* includes fpscr */
#if __WORDSIZE == 32
# define ELF_NVRREG      33      /* includes vscr */
#else
# define ELF_NVRREG      34      /* includes vscr */
#endif

typedef unsigned long elf_greg_t;
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef double elf_fpreg_t;
typedef elf_fpreg_t elf_fpregset_t[ELF_NFPREG];

/* Altivec registers */
typedef struct {
  unsigned int u[4];
} __attribute__ ((__aligned__ (16))) elf_vrreg_t;
typedef elf_vrreg_t elf_vrregset_t[ELF_NVRREG];

#endif /* ifndef ELF_NGREG */

#endif /* bits/procfs.h */
