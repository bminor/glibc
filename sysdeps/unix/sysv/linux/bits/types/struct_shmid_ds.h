/* Generic implementation of the shared memory struct shmid_ds.
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

#ifndef _SYS_SHM_H
# error "Never include <bits/types/struct_shmid_ds.h> directly; use <sys/shm.h> instead."
#endif

#include <bits/shm-pad.h>

#if __SHM_PAD_BEFORE_TIME
# define __SHM_PAD_TIME(NAME, RES)				\
  unsigned long int __glibc_reserved ## RES; __time_t NAME
#elif __SHM_PAD_AFTER_TIME
# define __SHM_PAD_TIME(NAME, RES)				\
  __time_t NAME; unsigned long int __glibc_reserved ## RES
#else
# define __SHM_PAD_TIME(NAME, RES)		\
  __time_t NAME
#endif

/* Data structure describing a shared memory segment.  */
struct shmid_ds
  {
    struct ipc_perm shm_perm;		/* operation permission struct */
#if !__SHM_SEGSZ_AFTER_TIME
    size_t shm_segsz;			/* size of segment in bytes */
#endif
    __SHM_PAD_TIME (shm_atime, 1);	/* time of last shmat() */
    __SHM_PAD_TIME (shm_dtime, 2);	/* time of last shmdt() */
    __SHM_PAD_TIME (shm_ctime, 3);	/* time of last change by shmctl() */
#if __SHM_PAD_BETWEEN_TIME_AND_SEGSZ
    unsigned long int __glibc_reserved4;
#endif
#if __SHM_SEGSZ_AFTER_TIME
    size_t shm_segsz;			/* size of segment in bytes */
#endif
    __pid_t shm_cpid;			/* pid of creator */
    __pid_t shm_lpid;			/* pid of last shmop */
    shmatt_t shm_nattch;		/* number of current attaches */
    __syscall_ulong_t __glibc_reserved5;
    __syscall_ulong_t __glibc_reserved6;
  };
