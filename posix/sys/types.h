/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
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

/*
 *	POSIX Standard: 2.6 Primitive System Data Types	<sys/types.h>
 */

#ifndef	_SYS_TYPES_H
#define	_SYS_TYPES_H	1

#include <features.h>

__BEGIN_DECLS

#include <bits/types.h>

#define __need_size_t
#include <stddef.h>

#include <bits/types/blkcnt_t.h>
#include <bits/types/clockid_t.h>
#include <bits/types/dev_t.h>
#include <bits/types/fsblkcnt_t.h>
#include <bits/types/fsfilcnt_t.h>
#include <bits/types/gid_t.h>
#include <bits/types/ino_t.h>
#include <bits/types/mode_t.h>
#include <bits/types/nlink_t.h>
#include <bits/types/off_t.h>
#include <bits/types/pid_t.h>
#include <bits/types/ssize_t.h>
#include <bits/types/time_t.h>
#include <bits/types/timer_t.h>
#include <bits/types/uid_t.h>

#ifdef __USE_LARGEFILE64
# include <bits/types/blkcnt64_t.h>
# include <bits/types/fsblkcnt64_t.h>
# include <bits/types/fsfilcnt64_t.h>
# include <bits/types/ino64_t.h>
# include <bits/types/off64_t.h>
#endif

#ifdef __USE_XOPEN
# include <bits/types/useconds_t.h>
# include <bits/types/suseconds_t.h>
#endif

#if defined __USE_XOPEN || defined __USE_XOPEN2K8
# include <bits/types/clock_t.h>
# include <bits/types/id_t.h>
# include <bits/types/key_t.h>
#endif

#if defined __USE_UNIX98 || defined __USE_XOPEN2K8
#include <bits/types/blksize_t.h>
#endif

#if defined __USE_POSIX199506 || defined __USE_UNIX98
# include <bits/pthreadtypes.h>
#endif

/* POSIX does not require intN_t to be defined in this header, so
   technically this ought to be under __USE_MISC, but it doesn't
   forbid them to be defined here either, and much existing code
   expects them to be defined here.  */
#include <bits/stdint-intn.h>

#ifdef __USE_MISC
/* Additional typedefs from BSD, mostly obsolete.  */
# include <bits/types/caddr_t.h>
# include <bits/types/daddr_t.h>
# include <bits/types/fsid_t.h>
# include <bits/types/loff_t.h>
# include <bits/types/register_t.h>

/* Shorthand unsigned types, mostly superseded by stdint.h.  */
# include <bits/types/uint.h>
# include <bits/types/u_int.h>
# include <bits/types/u_intN_t.h>

/* In BSD <sys/types.h> is expected to define BYTE_ORDER.  */
# include <endian.h>

/* It also defines `fd_set' and the FD_* macros for `select'.  */
# include <sys/select.h>
#endif /* Use misc.  */

__END_DECLS

#endif /* sys/types.h */
