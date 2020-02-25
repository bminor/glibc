/* Generic implementation of the semaphore struct __semid_ds32.
   Copyright (C) 1995-2020 Free Software Foundation, Inc.
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

#if __WORDSIZE == 32
/* This is the "new" y2038 types defined after the 5.1 kernel. It allows
   the kernel to use {o,c}time{_high} values to support a 64-bit time_t.  */
struct __semid_ds32 {
  struct ipc_perm sem_perm;              /* operation permission struct */
  __syscall_ulong_t   sem_otime;         /* last semop() time */
  __syscall_ulong_t   sem_otime_high;    /* last semop() time high */
  __syscall_ulong_t   sem_ctime;         /* last time changed by semctl() */
  __syscall_ulong_t   sem_ctime_high;    /* last time changed by semctl() high */
  __syscall_ulong_t   sem_nsems;         /* number of semaphores in set */
  __syscall_ulong_t   __glibc_reserved3;
  __syscall_ulong_t   __glibc_reserved4;
};
#endif
