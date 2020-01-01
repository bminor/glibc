/* Copyright (C) 1997-2020 Free Software Foundation, Inc.
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

#include <signal.h>
#include <nptl/pthreadP.h>              /* SIGCANCEL, SIGSETXID */

/* Get and/or change the set of blocked signals.  */
int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  sigset_t local_newmask;

  /* The only thing we have to make sure here is that SIGCANCEL and
     SIGSETXID are not blocked.  */
  if (set != NULL
      && __glibc_unlikely (__sigismember (set, SIGCANCEL)
	|| __glibc_unlikely (__sigismember (set, SIGSETXID))))
    {
      local_newmask = *set;
      __sigdelset (&local_newmask, SIGCANCEL);
      __sigdelset (&local_newmask, SIGSETXID);
      set = &local_newmask;
    }

  return INLINE_SYSCALL_CALL (rt_sigprocmask, how, set, oset, _NSIG / 8);
}
libc_hidden_def (__sigprocmask)
weak_alias (__sigprocmask, sigprocmask)
