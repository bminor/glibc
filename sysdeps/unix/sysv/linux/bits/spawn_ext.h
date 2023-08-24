/* POSIX spawn extensions.   Linux version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#ifndef _SPAWN_H
# error "Never include <bits/spawn-ext.h> directly; use <spawn.h> instead."
#endif

__BEGIN_DECLS

#ifdef __USE_MISC

/* Get the cgroupsv2 the attribute structure.  */
extern int posix_spawnattr_getcgroup_np (const posix_spawnattr_t *
					 __restrict __attr,
					 int *__restrict __cgroup)
     __THROW __nonnull ((1, 2));

/* Sore the cgroupsv2 the attribute structure.  */
extern int posix_spawnattr_setcgroup_np (posix_spawnattr_t *__attr,
					 int __cgroup)
     __THROW __nonnull ((1));

#endif /* __USE_MISC */

__END_DECLS
