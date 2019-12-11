/* Syscall definitions, Linux PowerPC generic version.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#define VDSO_NAME  "LINUX_2.6.15"
#define VDSO_HASH  123718565

/* List of system calls which are supported as vsyscalls.  */
#if defined(__PPC64__) || defined(__powerpc64__)
#define HAVE_CLOCK_GETRES64_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME64_VSYSCALL	"__kernel_clock_gettime"
#else
#define HAVE_CLOCK_GETRES_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME_VSYSCALL	"__kernel_clock_gettime"
#endif
#define HAVE_GETCPU_VSYSCALL		"__kernel_getcpu"
#define HAVE_TIME_VSYSCALL		"__kernel_time"
#define HAVE_GETTIMEOFDAY_VSYSCALL      "__kernel_gettimeofday"
#define HAVE_GET_TBFREQ                 "__kernel_get_tbfreq"

#if defined(__PPC64__) || defined(__powerpc64__)
# define HAVE_SIGTRAMP_RT64		"__kernel_sigtramp_rt64"
#else
# define HAVE_SIGTRAMP_32		"__kernel_sigtramp32"
# define HAVE_SIGTRAMP_RT32		"__kernel_sigtramp_rt32"
#endif
