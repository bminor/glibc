/* Definitions used by AArch64 indirect function resolvers.
   Copyright (C) 2019-2025 Free Software Foundation, Inc.
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

#ifndef _SYS_IFUNC_H
#define _SYS_IFUNC_H

/* A second argument is passed to the ifunc resolver.  */
#define _IFUNC_ARG_HWCAP	(1ULL << 62)

/* Maximum number of HWCAP elements that are currently supported.  */
#define _IFUNC_HWCAP_MAX	4

/* The prototype of a GNU indirect function resolver on AArch64 is

     ElfW(Addr) ifunc_resolver (uint64_t, const uint64_t *);

   The following prototype is also compatible:

     ElfW(Addr) ifunc_resolver (uint64_t, const __ifunc_arg_t *);

   The first argument might have the _IFUNC_ARG_HWCAP bit set and
   the remaining bits should match the AT_HWCAP settings.

   If the _IFUNC_ARG_HWCAP bit is set in the first argument, then
   the second argument is passed to the resolver function.  In
   this case, the second argument is a const pointer to a buffer
   that allows to access all available HWCAP elements.

   This buffer has its size in bytes at offset 0.  The HWCAP elements
   are available at offsets 8, 16, 24, 32... respectively for AT_HWCAP,
   AT_HWCAP2, AT_HWCAP3, AT_HWCAP4...  (these offsets are multiples of
   sizeof (unsigned long)).

   Indirect function resolvers must check availability of HWCAP
   elements at runtime before accessing them using the size of the
   buffer.  */

struct __ifunc_arg_t
{
  unsigned long _size;    /* Size of the struct, so it can grow.  */
  unsigned long _hwcap;
  unsigned long _hwcap2;  /* End of 1st published struct.  */
  unsigned long _hwcap3;
  unsigned long _hwcap4;  /* End of 2nd published struct.  */
};

typedef struct __ifunc_arg_t __ifunc_arg_t;

#endif
