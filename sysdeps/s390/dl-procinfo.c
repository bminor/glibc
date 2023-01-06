/* Data for s390 version of processor capability information.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

/* The hwcap and platform strings are now in
   sysdeps/s390/dl-procinfo-s390.c.  */

/* Needed by sysdeps/unix/sysv/linux/dl-vdso-setup.c (as included from
   sysdeps/generic/ldsodefs.h).  */
#undef PROCINFO_DECL
#undef PROCINFO_CLASS
