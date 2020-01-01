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

#include <sys/msg.h>
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
msgctl_syscall (int msqid, int cmd, struct msqid_ds *buf)
{
#ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  return INLINE_SYSCALL_CALL (msgctl, msqid, cmd | __IPC_64, buf);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_msgctl, msqid, cmd | __IPC_64, 0,
			      buf);
#endif
}

int
__new_msgctl (int msqid, int cmd, struct msqid_ds *buf)
{
  /* POSIX states ipc_perm mode should have type of mode_t.  */
  _Static_assert (sizeof ((struct msqid_ds){0}.msg_perm.mode)
		  == sizeof (mode_t),
		  "sizeof (msqid_ds.msg_perm.mode) != sizeof (mode_t)");

#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
  struct msqid_ds tmpds;
  if (cmd == IPC_SET)
    {
      tmpds = *buf;
      tmpds.msg_perm.mode *= 0x10000U;
      buf = &tmpds;
    }
#endif

  int ret = msgctl_syscall (msqid, cmd, buf);

#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
  if (ret >= 0)
    {
      switch (cmd)
	{
	case IPC_STAT:
	case MSG_STAT:
	case MSG_STAT_ANY:
	  buf->msg_perm.mode >>= 16;
	}
    }
#endif

  return ret;
}
versioned_symbol (libc, __new_msgctl, msgctl, DEFAULT_VERSION);

#if defined __ASSUME_SYSVIPC_BROKEN_MODE_T \
    && SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_31)
int
attribute_compat_text_section
__msgctl_mode16 (int msqid, int cmd, struct msqid_ds *buf)
{
  return msgctl_syscall (msqid, cmd, buf);
}
compat_symbol (libc, __msgctl_mode16, msgctl, GLIBC_2_2);
#endif

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
struct __old_msqid_ds
{
  struct __old_ipc_perm msg_perm;	/* structure describing operation permission */
  struct msg *__msg_first;		/* pointer to first message on queue */
  struct msg *__msg_last;		/* pointer to last message on queue */
  __time_t msg_stime;			/* time of last msgsnd command */
  __time_t msg_rtime;			/* time of last msgrcv command */
  __time_t msg_ctime;			/* time of last change */
  struct wait_queue *__wwait;		/* ??? */
  struct wait_queue *__rwait;		/* ??? */
  unsigned short int __msg_cbytes;	/* current number of bytes on queue */
  unsigned short int msg_qnum;		/* number of messages currently on queue */
  unsigned short int msg_qbytes;	/* max number of bytes allowed on queue */
  __ipc_pid_t msg_lspid;		/* pid of last msgsnd() */
  __ipc_pid_t msg_lrpid;		/* pid of last msgrcv() */
};

int
attribute_compat_text_section
__old_msgctl (int msqid, int cmd, struct __old_msqid_ds *buf)
{
#if defined __ASSUME_DIRECT_SYSVIPC_SYSCALLS \
    && !defined __ASSUME_SYSVIPC_DEFAULT_IPC_64
  /* For architecture that have wire-up msgctl but also have __IPC_64 to a
     value different than default (0x0) it means the compat symbol used the
     __NR_ipc syscall.  */
  return INLINE_SYSCALL_CALL (msgctl, msqid, cmd, buf);
#else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_msgctl, msqid, cmd, 0, buf);
#endif
}
compat_symbol (libc, __old_msgctl, msgctl, GLIBC_2_0);
#endif
