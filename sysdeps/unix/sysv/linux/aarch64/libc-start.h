/* AArch64 definitions for libc main startup.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.
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

#ifndef _LIBC_START_H
#define _LIBC_START_H

#ifndef SHARED

# include <dl-prop.h>

# ifndef PR_SET_SHADOW_STACK_STATUS
#  define PR_SET_SHADOW_STACK_STATUS	75
#  define PR_SHADOW_STACK_ENABLE	(1UL << 0)
# endif

/* Must be on a top-level stack frame that does not return.  */
static inline void __attribute__((always_inline))
aarch64_libc_setup_tls (void)
{
  __libc_setup_tls ();

  struct link_map *main_map = _dl_get_dl_main_map ();
  const ElfW(Phdr) *phdr = GL(dl_phdr);
  const ElfW(Phdr) *ph;
  for (ph = phdr; ph < phdr + GL(dl_phnum); ph++)
    if (ph->p_type == PT_GNU_PROPERTY)
      {
	_dl_process_pt_gnu_property (main_map, -1, ph);
	_rtld_main_check (main_map, _dl_argv[0]);
	break;
      }

  if (GL(dl_aarch64_gcs) != 0)
    {
      int ret = INLINE_SYSCALL_CALL (prctl, PR_SET_SHADOW_STACK_STATUS,
				     PR_SHADOW_STACK_ENABLE, 0, 0, 0);
      if (ret)
        _dl_fatal_printf ("failed to enable GCS: %d\n", -ret);
    }
}

# define ARCH_SETUP_IREL() apply_irel ()
# define ARCH_SETUP_TLS() aarch64_libc_setup_tls ()
# define ARCH_APPLY_IREL()
#endif /* ! SHARED  */

#endif /* _LIBC_START_H  */
