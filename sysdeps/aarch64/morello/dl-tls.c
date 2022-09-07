/* Thread-local storage handling in the ELF dynamic linker.  Morello version.
   Copyright (C) 2022 Free Software Foundation, Inc.
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

#ifdef SHARED
/* Add a wrapper around the generic __tls_get_addr code to set the
   bounds on the returned pointer.  */

# define __tls_get_addr __tls_get_addr_unbounded
# include <elf/dl-tls.c>
# undef __tls_get_addr

void *__tls_get_addr (tls_index *ti);
rtld_hidden_proto (__tls_get_addr)
rtld_hidden_def (__tls_get_addr)

void *
__tls_get_addr (tls_index *ti)
{
  void *p = __tls_get_addr_unbounded (ti);
  return __builtin_cheri_bounds_set_exact (p, ti->ti_size);
}
#else
# include <elf/dl-tls.c>
#endif
