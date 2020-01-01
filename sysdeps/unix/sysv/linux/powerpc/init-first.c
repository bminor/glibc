/* Initialization code run first thing by the ELF startup code.  Linux/PowerPC.
   Copyright (C) 2007-2020 Free Software Foundation, Inc.
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

unsigned long long (*VDSO_SYMBOL(get_tbfreq)) (void) attribute_hidden;
#if defined(__PPC64__) || defined(__powerpc64__)
void *VDSO_SYMBOL(sigtramp_rt64) attribute_hidden;
#else
void *VDSO_SYMBOL(sigtramp32) attribute_hidden;
void *VDSO_SYMBOL(sigtramp_rt32) attribute_hidden;
#endif

static inline void
__libc_vdso_platform_setup_arch (void)
{
  VDSO_SYMBOL (get_tbfreq) = get_vdso_mangle_symbol (HAVE_GET_TBFREQ);

  /* PPC64 uses only one signal trampoline symbol, while PPC32 will use
     two depending if SA_SIGINFO is used (__kernel_sigtramp_rt32) or not
     (__kernel_sigtramp32).
     There is no need to pointer mangle these symbol because they will
     used only for pointer comparison.  */
#if defined(__PPC64__) || defined(__powerpc64__)
  VDSO_SYMBOL(sigtramp_rt64) =  get_vdso_symbol (HAVE_SIGTRAMP_RT64);
#else
  VDSO_SYMBOL(sigtramp32) = get_vdso_symbol (HAVE_SIGTRAMP_32);
  VDSO_SYMBOL(sigtramp_rt32) = get_vdso_symbol (HAVE_SIGTRAMP_RT32);
#endif
}

#define VDSO_SETUP_ARCH __libc_vdso_platform_setup_arch

#include <sysdeps/unix/sysv/linux/init-first.c>
