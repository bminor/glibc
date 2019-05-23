/* Initialization code run first thing by the ELF startup code.  Linux/PowerPC.
   Copyright (C) 2007-2019 Free Software Foundation, Inc.
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

#include <dl-vdso.h>
#include <libc-vdso.h>

int (*VDSO_SYMBOL(gettimeofday)) (struct timeval *, void *)
  attribute_hidden;
int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *);
int (*VDSO_SYMBOL(clock_getres)) (clockid_t, struct timespec *);
unsigned long long (*VDSO_SYMBOL(get_tbfreq)) (void);
int (*VDSO_SYMBOL(getcpu)) (unsigned *, unsigned *);
time_t (*VDSO_SYMBOL(time)) (time_t *);

#if defined(__PPC64__) || defined(__powerpc64__)
void *VDSO_SYMBOL(sigtramp_rt64);
#else
void *VDSO_SYMBOL(sigtramp32);
void *VDSO_SYMBOL(sigtramp_rt32);
#endif

static inline void
_libc_vdso_platform_setup (void)
{
  void *p = get_vdso_symbol ("__kernel_gettimeofday");
  PTR_MANGLE (p);
  VDSO_SYMBOL (gettimeofday) = p;

  p = get_vdso_symbol ("__kernel_clock_gettime");
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime) = p;

  p = get_vdso_symbol ("__kernel_clock_getres");
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_getres) = p;

  p = get_vdso_symbol ("__kernel_get_tbfreq");
  PTR_MANGLE (p);
  VDSO_SYMBOL (get_tbfreq) = p;

  p = get_vdso_symbol ("__kernel_getcpu");
  PTR_MANGLE (p);
  VDSO_SYMBOL (getcpu) = p;

  p = get_vdso_symbol ("__kernel_time");
  PTR_MANGLE (p);
  VDSO_SYMBOL (time) = p;

  /* PPC64 uses only one signal trampoline symbol, while PPC32 will use
     two depending if SA_SIGINFO is used (__kernel_sigtramp_rt32) or not
     (__kernel_sigtramp32).
     There is no need to pointer mangle these symbol because they will
     used only for pointer comparison.  */
#if defined(__PPC64__) || defined(__powerpc64__)
  VDSO_SYMBOL(sigtramp_rt64) =  get_vdso_symbol ("__kernel_sigtramp_rt64");
#else
  VDSO_SYMBOL(sigtramp32) = get_vdso_symbol ("__kernel_sigtramp32");
  VDSO_SYMBOL(sigtramp_rt32) = get_vdso_symbol ("__kernel_sigtramp_rt32");
#endif
}

#define VDSO_SETUP _libc_vdso_platform_setup

#include <csu/init-first.c>
