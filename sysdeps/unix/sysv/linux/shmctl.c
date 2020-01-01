/* Copyright (C) 1995-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

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

#include <sys/shm.h>
#include <stdarg.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <shlib-compat.h>
#include <errno.h>

#ifndef DEFAULT_VERSION
# ifndef __ASSUME_SYSVIPC_BROKEN_MODE_T
#  define DEFAULT_VERSION GLIBC_2_2
# else
#  define DEFAULT_VERSION GLIBC_2_31
# endif
#endif

static int
shmctl_syscall (int shmid, int cmd, struct shmid_ds *buf)
{
#ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  return INLINE_SYSCALL_CALL (shmctl, shmid, cmd | __IPC_64, buf);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_shmctl, shmid, cmd | __IPC_64, 0,
			      buf);
#endif
}

/* Provide operations to control over shared memory segments.  */
int
__new_shmctl (int shmid, int cmd, struct shmid_ds *buf)
{
  /* POSIX states ipc_perm mode should have type of mode_t.  */
  _Static_assert (sizeof ((struct shmid_ds){0}.shm_perm.mode)
		  == sizeof (mode_t),
		  "sizeof (msqid_ds.msg_perm.mode) != sizeof (mode_t)");

#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
  struct shmid_ds tmpds;
  if (cmd == IPC_SET)
    {
      tmpds = *buf;
      tmpds.shm_perm.mode *= 0x10000U;
      buf = &tmpds;
    }
#endif

  int ret = shmctl_syscall (shmid, cmd, buf);

#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
  if (ret >= 0)
    {
      switch (cmd)
	{
        case IPC_STAT:
        case SHM_STAT:
        case SHM_STAT_ANY:
          buf->shm_perm.mode >>= 16;
	}
    }
#endif

  return ret;
}
versioned_symbol (libc, __new_shmctl, shmctl, DEFAULT_VERSION);

#if defined __ASSUME_SYSVIPC_BROKEN_MODE_T \
    && SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_31)
int
attribute_compat_text_section
__shmctl_mode16 (int shmid, int cmd, struct shmid_ds *buf)
{
  return shmctl_syscall (shmid, cmd, buf);
}
compat_symbol (libc, __shmctl_mode16, shmctl, GLIBC_2_2);
#endif

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
struct __old_shmid_ds
{
  struct __old_ipc_perm shm_perm;	/* operation permission struct */
  int shm_segsz;			/* size of segment in bytes */
  __time_t shm_atime;			/* time of last shmat() */
  __time_t shm_dtime;			/* time of last shmdt() */
  __time_t shm_ctime;			/* time of last change by shmctl() */
  __ipc_pid_t shm_cpid;			/* pid of creator */
  __ipc_pid_t shm_lpid;			/* pid of last shmop */
  unsigned short int shm_nattch;	/* number of current attaches */
  unsigned short int __shm_npages;	/* size of segment (pages) */
  unsigned long int *__shm_pages;	/* array of ptrs to frames -> SHMMAX */
  struct vm_area_struct *__attaches;	/* descriptors for attaches */
};

int
attribute_compat_text_section
__old_shmctl (int shmid, int cmd, struct __old_shmid_ds *buf)
{
#if defined __ASSUME_DIRECT_SYSVIPC_SYSCALLS \
    && !defined __ASSUME_SYSVIPC_DEFAULT_IPC_64
  /* For architecture that have wire-up shmctl but also have __IPC_64 to a
     value different than default (0x0), it means the compat symbol used the
     __NR_ipc syscall.  */
  return INLINE_SYSCALL_CALL (shmctl, shmid, cmd, buf);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_shmctl, shmid, cmd, 0, buf);
#endif
}
compat_symbol (libc, __old_shmctl, shmctl, GLIBC_2_0);
#endif
