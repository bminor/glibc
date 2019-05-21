/* Copyright (C) 2007-2020 Free Software Foundation, Inc.
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

#ifndef	_SYS_SIGNALFD_H
#define	_SYS_SIGNALFD_H	1

#include <features.h>
#include <bits/types.h>
#include <bits/types/sigset_t.h>

/* Get the platform-dependent flags.  */
#include <bits/signalfd.h>

struct signalfd_siginfo
{
  __uint32_t ssi_signo;
  __int32_t ssi_errno;
  __int32_t ssi_code;
  __uint32_t ssi_pid;
  __uint32_t ssi_uid;
  __int32_t ssi_fd;
  __uint32_t ssi_tid;
  __uint32_t ssi_band;
  __uint32_t ssi_overrun;
  __uint32_t ssi_trapno;
  __int32_t ssi_status;
  __int32_t ssi_int;
  __uint64_t ssi_ptr;
  __uint64_t ssi_utime;
  __uint64_t ssi_stime;
  __uint64_t ssi_addr;
  __uint16_t ssi_addr_lsb;
  __uint16_t __pad2;
  __int32_t ssi_syscall;
  __uint64_t ssi_call_addr;
  __uint32_t ssi_arch;
  __uint8_t __pad[28];
};

__BEGIN_DECLS

/* Request notification for delivery of signals in MASK to be
   performed using descriptor FD.*/
extern int signalfd (int __fd, const sigset_t *__mask, int __flags)
  __THROW __nonnull ((2));

__END_DECLS

#endif /* sys/signalfd.h */
