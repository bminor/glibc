/* Basic definition for Sysv IPC test functions.
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

#ifndef _TEST_SYSV_H
#define _TEST_SYSV_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <include/array_length.h>

/* Return the first invalid command SysV IPC command from common shared
   between message queue, shared memory, and semaphore.  */
static inline int
first_common_invalid_cmd (void)
{
  const int common_cmds[] = {
    IPC_RMID,
    IPC_SET,
    IPC_STAT,
    IPC_INFO,
  };

  int invalid = 0;
  for (int i = 0; i < array_length (common_cmds); i++)
    {
      if (invalid == common_cmds[i])
	{
	  invalid++;
	  i = 0;
        }
    }

  return invalid;
}

/* Return the first invalid command SysV IPC command for semaphore.  */
static inline int
first_sem_invalid_cmd (void)
{
  const int sem_cmds[] = {
    GETPID,
    GETVAL,
    GETALL,
    GETNCNT,
    GETZCNT,
    SETVAL,
    SETALL,
    SEM_STAT,
    SEM_INFO,
#ifdef SEM_STAT_ANY
    SEM_STAT_ANY,
#endif
  };

  int invalid = first_common_invalid_cmd ();
  for (int i = 0; i < array_length (sem_cmds); i++)
    {
      if (invalid == sem_cmds[i])
	{
	  invalid++;
	  i = 0;
	}
    }

  return invalid;
}

#endif /* _TEST_SYSV_H  */
