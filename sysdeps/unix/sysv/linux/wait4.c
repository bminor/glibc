/* Wait for process to change state.  Linux version.
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
   <https://www.gnu.org/licenses/>.  */

#include <sys/wait.h>
#include <sys/resource.h>
#include <sysdep-cancel.h>

pid_t
__wait4 (pid_t pid, int *stat_loc, int options, struct rusage *usage)
{
#ifdef __NR_wait4
  return SYSCALL_CANCEL (wait4, pid, stat_loc, options, usage);
#elif defined (__ASSUME_WAITID_PID0_P_PGID)
  idtype_t idtype = P_PID;

  if (pid < -1)
    {
      idtype = P_PGID;
      pid *= -1;
    }
  else if (pid == -1)
    idtype = P_ALL;
  else if (pid == 0)
    idtype = P_PGID;

  options |= WEXITED;

  siginfo_t infop;
  if (SYSCALL_CANCEL (waitid, idtype, pid, &infop, options, usage) < 0)
    return -1;

  if (stat_loc)
    {
      switch (infop.si_code)
        {
        case CLD_EXITED:
          *stat_loc = W_EXITCODE (infop.si_status, 0);
          break;
        case CLD_DUMPED:
          *stat_loc = WCOREFLAG | infop.si_status;
	  break;
        case CLD_KILLED:
          *stat_loc = infop.si_status;
          break;
        case CLD_TRAPPED:
        case CLD_STOPPED:
          *stat_loc = W_STOPCODE (infop.si_status);
          break;
        case CLD_CONTINUED:
          *stat_loc = __W_CONTINUED;
          break;
	default:
	  *stat_loc = 0;
	  break;
        }
    }

  return infop.si_pid;
# else
/* Linux waitid prior kernel 5.4 does not support waiting for the current
   process.  It is possible to emulate wait4 it by calling getpgid for
   PID 0, however, it would require an additional syscall and it is inherent
   racy: after the current process group is received and before it is passed
   to waitid a signal could arrive causing the current process group to
   change.  */
# error "The kernel ABI does not provide a way to implement wait4"
#endif
}
libc_hidden_def (__wait4);
weak_alias (__wait4, wait4)
