/* Change data segment.  Linux SPARC version.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <sysdep.h>

/* This must be initialized data because commons can't have aliases.  */
void *__curbrk = 0;

#if HAVE_INTERNAL_BRK_ADDR_SYMBOL
/* Old braindamage in GCC's crtstuff.c requires this symbol in an attempt
   to work around different old braindamage in the old Linux ELF dynamic
   linker.  */
weak_alias (__curbrk, ___brk_addr)
#endif

#ifdef __arch64__
# define SYSCALL_NUM "0x6d"
#else
# define SYSCALL_NUM "0x10"
#endif

int
__brk (void *addr)
{
  register long int g1 asm ("g1") = __NR_brk;
  register long int o0 asm ("o0") = (long int) addr;
  asm volatile ("ta " SYSCALL_NUM
		: "=r"(o0)
		: "r"(g1), "0"(o0)
		: "cc");
  __curbrk = (void *) o0;

  if (__curbrk < addr)
    {
      __set_errno (ENOMEM);
      return -1;
    }

  return 0;
}
weak_alias (__brk, brk)
