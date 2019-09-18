/* vDSO internal symbols.  Linux generic version.
   Copyright (C) 2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <dl-vdso.h>
#include <libc-vdso.h>

/* vDSO symbol used on clock_gettime implementation.  */
#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *)
  attribute_hidden;
#endif
/* vDSO symbol used on clock_gettime64 implementation.  */
#ifdef HAVE_CLOCK_GETTIME64_VSYSCALL
int (*VDSO_SYMBOL(clock_gettime64)) (clockid_t, struct __timespec64 *)
  attribute_hidden;
#endif
/* vDSO symbol used on clock_getres implementation.  */
#ifdef HAVE_CLOCK_GETRES_VSYSCALL
int (*VDSO_SYMBOL(clock_getres)) (clockid_t, struct timespec *)
  attribute_hidden;
#endif
/* vDSO symbol used on gettimeofday implementation.  */
#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
int (*VDSO_SYMBOL (gettimeofday)) (struct timeval *, void *)
  attribute_hidden;
#endif
/* vDSO symbol used on GNU extension getcpu implementation.  */
#ifdef HAVE_GETCPU_VSYSCALL
long int (*VDSO_SYMBOL(getcpu)) (unsigned *, unsigned *, void *)
   attribute_hidden;
#endif
/* vDSO symbol used on time implementation.  */
#ifdef HAVE_TIME_VSYSCALL
time_t (*VDSO_SYMBOL(time)) (time_t *) attribute_hidden;
#endif

static inline void
__libc_vdso_platform_setup (void)
{
#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
  VDSO_SYMBOL(clock_gettime)
    = get_vdso_mangle_symbol (HAVE_CLOCK_GETTIME_VSYSCALL);
#endif

#ifdef HAVE_CLOCK_GETTIME64_VSYSCALL
  VDSO_SYMBOL(clock_gettime64)
    = get_vdso_mangle_symbol (HAVE_CLOCK_GETTIME64_VSYSCALL);
#endif

#ifdef HAVE_CLOCK_GETRES_VSYSCALL
  VDSO_SYMBOL(clock_getres)
    = get_vdso_mangle_symbol (HAVE_CLOCK_GETRES_VSYSCALL);
#endif

#ifdef HAVE_GETTIMEOFDAY_VSYSCALL
  VDSO_SYMBOL(gettimeofday)
    = get_vdso_mangle_symbol (HAVE_GETTIMEOFDAY_VSYSCALL);
#endif

#ifdef HAVE_GETCPU_VSYSCALL
  VDSO_SYMBOL(getcpu) = get_vdso_mangle_symbol (HAVE_GETCPU_VSYSCALL);
#endif

#ifdef HAVE_TIME_VSYSCALL
  VDSO_SYMBOL(time) = get_vdso_mangle_symbol (HAVE_TIME_VSYSCALL);
#endif

#ifdef VDSO_SETUP_ARCH
  VDSO_SETUP_ARCH ();
#endif
}

#define VDSO_SETUP __libc_vdso_platform_setup

#include <csu/init-first.c>
