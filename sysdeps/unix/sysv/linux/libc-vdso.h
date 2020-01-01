/* Copyright (C) 2009-2020 Free Software Foundation, Inc.

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

#ifndef _LIBC_VDSO_H
#define _LIBC_VDSO_H

#define VDSO_SYMBOL(__name) __vdso_##__name

/* Adjust the return IFUNC value from a vDSO symbol accordingly required
   by the ELFv1 ABI.  It is used by the architecture to create an ODP
   entry since the kernel vDSO does not provide it.  */
#ifndef VDSO_IFUNC_RET
# define VDSO_IFUNC_RET(__value) (__value)
#endif

#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
extern int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *)
  attribute_hidden;
#endif
#ifdef HAVE_CLOCK_GETTIME64_VSYSCALL
extern int (*VDSO_SYMBOL(clock_gettime64)) (clockid_t, struct __timespec64 *)
  attribute_hidden;
#endif
#ifdef HAVE_CLOCK_GETRES_VSYSCALL
extern int (*VDSO_SYMBOL(clock_getres)) (clockid_t, struct timespec *)
  attribute_hidden;
#endif
#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
extern int (*VDSO_SYMBOL (gettimeofday)) (struct timeval *, void *)
  attribute_hidden;
#endif
#ifdef HAVE_GETCPU_VSYSCALL
extern long int (*VDSO_SYMBOL(getcpu)) (unsigned *, unsigned *, void *)
  attribute_hidden;
#endif
#ifdef HAVE_TIME_VSYSCALL
extern time_t (*VDSO_SYMBOL(time)) (time_t *) attribute_hidden;
#endif

#endif /* _LIBC_VDSO_H */
