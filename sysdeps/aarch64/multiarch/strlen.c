/* Multiple versions of strlen. AARCH64 version.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.
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

/* Define multiple versions only for the definition in libc.  */

#if IS_IN (libc)
/* Redefine strlen so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# undef strlen
# define strlen __redirect_strlen
# include <string.h>
# include <init-arch.h>

/* This should check HWCAP2_MTE when it is available: current
   linux kernel does not expose it, but its value is reserved.
   This is needed to make glibc MTE-safe on future systems in
   case user code enables MTE. The ABI contract for enabling
   MTE is not yet specified, but it can be useful for at least
   debugging which does not need a contract.  */
#define FUTURE_HWCAP2_MTE (1 << 18)
#define MTE_ENABLED() (GLRO(dl_hwcap2) & FUTURE_HWCAP2_MTE)

extern __typeof (__redirect_strlen) __strlen;

extern __typeof (__redirect_strlen) __strlen_mte attribute_hidden;
extern __typeof (__redirect_strlen) __strlen_asimd attribute_hidden;

libc_ifunc (__strlen, (MTE_ENABLED () ? __strlen_mte : __strlen_asimd));

# undef strlen
strong_alias (__strlen, strlen);
#endif
