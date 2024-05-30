/* Architecture-specific thread initialization for NPTL.  x86-64 version.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

static inline __always_inline void
__nptl_arch_thread_init (void)
{
#if CET_ENABLED
  void **ssp;
  asm ("rdsspq %0"
       : "=r" (ssp)
       : "0" (0));
  THREAD_SETMEM (THREAD_SELF, header.ssp_base, ssp);
#endif
}
