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

#include <sys/sem.h>
#include <stdarg.h>
#include <ipc_priv.h>
#include <sysdep.h>
#include <shlib-compat.h>
#include <errno.h>
#include <struct__semid_ds32.h>
#include <linux/posix_types.h>  /* For __kernel_mode_t.  */

#define IPC_TIME64 \
 (__WORDSIZE == 32 && __TIMESIZE == 64 \
     && (!defined __SYSCALL_WORDSIZE || __SYSCALL_WORDSIZE == 32))

/* Define a `union semun' suitable for Linux here.  */
union semun
{
  int val;			/* value for SETVAL */
#if IPC_TIME64
  struct __semid_ds32 *buf;   /* buffer for IPC_STAT & IPC_SET */
#else
  struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
#endif
  unsigned short int *array;	/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
};

#ifndef DEFAULT_VERSION
# ifndef __ASSUME_SYSVIPC_BROKEN_MODE_T
#  define DEFAULT_VERSION GLIBC_2_2
# else
#  define DEFAULT_VERSION GLIBC_2_31
# endif
#endif

static int
semctl_syscall (int semid, int semnum, int cmd, union semun arg)
{
  int ret;
#if IPC_TIME64
  struct __semid_ds32 tmp, *orig;
  if (cmd == IPC_SET)
    {
      tmp = (struct __semid_ds32) {
        .sem_perm  = arg.buf->sem_perm,
        .sem_otime = arg.buf->sem_otime,
        .sem_ctime = arg.buf->sem_ctime,
        .sem_nsems = arg.buf->sem_nsems,
      };
      orig = arg.buf;
      arg.buf = &tmp;
    }
#endif

#ifdef __ASSUME_DIRECT_SYSVIPC_SYSCALLS
  ret = INLINE_SYSCALL_CALL (semctl, semid, semnum, cmd | __IPC_64,
                             arg.array);
#else
  ret = INLINE_SYSCALL_CALL (ipc, IPCOP_semctl, semid, semnum, cmd | __IPC_64,
                             SEMCTL_ARG_ADDRESS (arg));
#endif

#if IPC_TIME64
  if (ret >= 0
      && (cmd == IPC_STAT || cmd == SEM_STAT || cmd == SEM_STAT_ANY))
    {
      arg.buf = orig;
      arg.buf->sem_perm  = tmp.sem_perm;
      arg.buf->sem_otime = tmp.sem_otime
                           | ((time_t) tmp.sem_otime_high << 32);
      arg.buf->sem_ctime = tmp.sem_ctime
                           | ((time_t) tmp.sem_ctime_high << 32);
      arg.buf->sem_nsems = tmp.sem_nsems;
    }
#endif

    return ret;
}

int
__new_semctl (int semid, int semnum, int cmd, ...)
{
  /* POSIX states ipc_perm mode should have type of mode_t.  */
  _Static_assert (sizeof ((struct semid_ds){0}.sem_perm.mode)
		  == sizeof (mode_t),
		  "sizeof (msqid_ds.msg_perm.mode) != sizeof (mode_t)");

  union semun arg = { 0 };
  va_list ap;

  /* Get the argument only if required.  */
  switch (cmd)
    {
    case SETVAL:        /* arg.val */
    case GETALL:        /* arg.array */
    case SETALL:
    case IPC_STAT:      /* arg.buf */
    case IPC_SET:
    case SEM_STAT:
    case IPC_INFO:      /* arg.__buf */
    case SEM_INFO:
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
      break;
    }

#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
  struct semid_ds tmpds;
  if (cmd == IPC_SET)
    {
      tmpds = *arg.buf;
      tmpds.sem_perm.mode *= 0x10000U;
      arg.buf = &tmpds;
    }
#endif

  int ret = semctl_syscall (semid, semnum, cmd, arg);

  if (ret >= 0)
    {
      switch (cmd)
	{
        case IPC_STAT:
        case SEM_STAT:
        case SEM_STAT_ANY:
#ifdef __ASSUME_SYSVIPC_BROKEN_MODE_T
          arg.buf->sem_perm.mode >>= 16;
#else
	  /* Old Linux kernel versions might not clear the mode padding.  */
	  if (sizeof ((struct semid_ds){0}.sem_perm.mode)
	      != sizeof (__kernel_mode_t))
	    arg.buf->sem_perm.mode &= 0xFFFF;
#endif
	}
    }

  return ret;
}
versioned_symbol (libc, __new_semctl, semctl, DEFAULT_VERSION);

#if defined __ASSUME_SYSVIPC_BROKEN_MODE_T \
    && SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_31)
int
attribute_compat_text_section
__semctl_mode16 (int semid, int semnum, int cmd, ...)
{
  union semun arg = { 0 };
  va_list ap;

  /* Get the argument only if required.  */
  switch (cmd)
    {
    case SETVAL:        /* arg.val */
    case GETALL:        /* arg.array */
    case SETALL:
    case IPC_STAT:      /* arg.buf */
    case IPC_SET:
    case SEM_STAT:
    case IPC_INFO:      /* arg.__buf */
    case SEM_INFO:
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
      break;
    }

  return semctl_syscall (semid, semnum, cmd, arg);
}
compat_symbol (libc, __semctl_mode16, semctl, GLIBC_2_2);
#endif

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2)
/* Since semctl use a variadic argument for semid_ds there is not need to
   define and tie the compatibility symbol to the old 'union semun'
   definition.  */
int
attribute_compat_text_section
__old_semctl (int semid, int semnum, int cmd, ...)
{
  union semun arg = { 0 };
  va_list ap;

  /* Get the argument only if required.  */
  switch (cmd)
    {
    case SETVAL:        /* arg.val */
    case GETALL:        /* arg.array */
    case SETALL:
    case IPC_STAT:      /* arg.buf */
    case IPC_SET:
    case SEM_STAT:
    case IPC_INFO:      /* arg.__buf */
    case SEM_INFO:
      va_start (ap, cmd);
      arg = va_arg (ap, union semun);
      va_end (ap);
      break;
    }

#if defined __ASSUME_DIRECT_SYSVIPC_SYSCALLS \
    && !defined __ASSUME_SYSVIPC_DEFAULT_IPC_64
 /* For architectures that have wire-up semctl but also have __IPC_64 to a
    value different than default (0x0) it means the compat symbol used the
    __NR_ipc syscall.  */
  return INLINE_SYSCALL_CALL (semctl, semid, semnum, cmd, arg.array);
# else
  return INLINE_SYSCALL_CALL (ipc, IPCOP_semctl, semid, semnum, cmd,
			      SEMCTL_ARG_ADDRESS (arg));
# endif
}
compat_symbol (libc, __old_semctl, semctl, GLIBC_2_0);
#endif
