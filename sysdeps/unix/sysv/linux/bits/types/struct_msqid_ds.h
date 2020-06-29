/* Generic implementation of the SysV message struct msqid_ds.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#ifndef _SYS_MSG_H
# error "Never use <bits/msq.h> directly; include <sys/msg.h> instead."
#endif

#include <bits/msq-pad.h>

#if __MSQ_PAD_BEFORE_TIME
# define __MSQ_PAD_TIME(NAME, RES)				\
  unsigned long int __glibc_reserved ## RES; __time_t NAME
#elif __MSQ_PAD_AFTER_TIME
# define __MSQ_PAD_TIME(NAME, RES)				\
  __time_t NAME; unsigned long int __glibc_reserved ## RES
#else
# define __MSQ_PAD_TIME(NAME, RES)		\
  __time_t NAME
#endif

/* Structure of record for one message inside the kernel.
   The type `struct msg' is opaque.  */
struct msqid_ds
{
  struct ipc_perm msg_perm;	/* structure describing operation permission */
  __MSQ_PAD_TIME (msg_stime, 1);	/* time of last msgsnd command */
  __MSQ_PAD_TIME (msg_rtime, 2);	/* time of last msgrcv command */
  __MSQ_PAD_TIME (msg_ctime, 3);	/* time of last change */
  __syscall_ulong_t __msg_cbytes; /* current number of bytes on queue */
  msgqnum_t msg_qnum;		/* number of messages currently on queue */
  msglen_t msg_qbytes;		/* max number of bytes allowed on queue */
  __pid_t msg_lspid;		/* pid of last msgsnd() */
  __pid_t msg_lrpid;		/* pid of last msgrcv() */
  __syscall_ulong_t __glibc_reserved4;
  __syscall_ulong_t __glibc_reserved5;
};
