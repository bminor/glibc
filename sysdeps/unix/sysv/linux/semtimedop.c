/* Copyright (C) 1995-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, August 1995.

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

#include <sys/sem.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <errno.h>

/* Perform user-defined atomical operation of array of semaphores.  */
int
__semtimedop64 (int semid, struct sembuf *sops, size_t nsops,
		const struct __timespec64 *timeout)
{
  int r;
#if defined __NR_semtimedop_time64
  r = INLINE_SYSCALL_CALL (semtimedop_time64, semid, sops, nsops, timeout);
#elif defined __ASSUME_DIRECT_SYSVIPC_SYSCALLS && defined __NR_semtimedop
  r = INLINE_SYSCALL_CALL (semtimedop, semid, sops, nsops, timeout);
#else
  r = INLINE_SYSCALL_CALL (ipc, IPCOP_semtimedop, semid,
			   SEMTIMEDOP_IPC_ARGS (nsops, sops, timeout));
#endif

#ifndef __ASSUME_TIME64_SYSCALLS
  if (r == 0 || errno != ENOSYS)
    return r;

  struct timespec ts32, *pts32 = NULL;
  if (timeout != NULL)
    {
      if (! in_time_t_range (timeout->tv_sec))
	{
	  __set_errno (EINVAL);
	  return -1;
	}
      ts32 = valid_timespec64_to_timespec (*timeout);
      pts32 = &ts32;
    }
# if defined __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  r = INLINE_SYSCALL_CALL (semtimedop, semid, sops, nsops, pts32);
# else
  r = INLINE_SYSCALL_CALL (ipc, IPCOP_semtimedop, semid,
			   SEMTIMEDOP_IPC_ARGS (nsops, sops, pts32));
# endif
#endif /* __ASSUME_TIME64_SYSCALLS  */
  return r;
}
#if __TIMESIZE != 64
libc_hidden_def (__semtimedop64)

int
__semtimedop (int semid, struct sembuf *sops, size_t nsops,
	      const struct timespec *timeout)
{
  struct __timespec64 ts64, *pts64 = NULL;
  if (timeout != NULL)
    {
      ts64 = valid_timespec_to_timespec64 (*timeout);
      pts64 = &ts64;
    }
  return __semtimedop64 (semid, sops, nsops, pts64);
}
#endif
weak_alias (__semtimedop, semtimedop)
