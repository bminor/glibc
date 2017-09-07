/* Return resource usage for the current process.
   
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <sys/resource.h>
#include <include/time.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <errno.h>

/* Structure which says how much of each resource has been used.  */
/* 64-bit time version */
/* The purpose of all the unions is to have the kernel-compatible layout
   while keeping the API type as 'long int', and among machines where
   __syscall_slong_t is not 'long int', this only does the right thing
   for little-endian ones, like x32.  */
struct __rusage64
  {
    /* Total amount of user time used.  */
    struct __timeval64 ru_utime;
    /* Total amount of system time used.  */
    struct __timeval64 ru_stime;
    /* Maximum resident set size (in kilobytes).  */
    __extension__ union
      {
	long int ru_maxrss;
	__syscall_slong_t __ru_maxrss_word;
      };
    /* Amount of sharing of text segment memory
       with other processes (kilobyte-seconds).  */
    /* Maximum resident set size (in kilobytes).  */
    __extension__ union
      {
	long int ru_ixrss;
	__syscall_slong_t __ru_ixrss_word;
      };
    /* Amount of data segment memory used (kilobyte-seconds).  */
    __extension__ union
      {
	long int ru_idrss;
	__syscall_slong_t __ru_idrss_word;
      };
    /* Amount of stack memory used (kilobyte-seconds).  */
    __extension__ union
      {
	long int ru_isrss;
	 __syscall_slong_t __ru_isrss_word;
      };
    /* Number of soft page faults (i.e. those serviced by reclaiming
       a page from the list of pages awaiting reallocation.  */
    __extension__ union
      {
	long int ru_minflt;
	__syscall_slong_t __ru_minflt_word;
      };
    /* Number of hard page faults (i.e. those that required I/O).  */
    __extension__ union
      {
	long int ru_majflt;
	__syscall_slong_t __ru_majflt_word;
      };
    /* Number of times a process was swapped out of physical memory.  */
    __extension__ union
      {
	long int ru_nswap;
	__syscall_slong_t __ru_nswap_word;
      };
    /* Number of input operations via the file system.  Note: This
       and `ru_oublock' do not include operations with the cache.  */
    __extension__ union
      {
	long int ru_inblock;
	__syscall_slong_t __ru_inblock_word;
      };
    /* Number of output operations via the file system.  */
    __extension__ union
      {
	long int ru_oublock;
	__syscall_slong_t __ru_oublock_word;
      };
    /* Number of IPC messages sent.  */
    __extension__ union
      {
	long int ru_msgsnd;
	__syscall_slong_t __ru_msgsnd_word;
      };
    /* Number of IPC messages received.  */
    __extension__ union
      {
	long int ru_msgrcv;
	__syscall_slong_t __ru_msgrcv_word;
      };
    /* Number of signals delivered.  */
    __extension__ union
      {
	long int ru_nsignals;
	__syscall_slong_t __ru_nsignals_word;
      };
    /* Number of voluntary context switches, i.e. because the process
       gave up the process before it had to (usually to wait for some
       resource to be available).  */
    __extension__ union
      {
	long int ru_nvcsw;
	__syscall_slong_t __ru_nvcsw_word;
      };
    /* Number of involuntary context switches, i.e. a higher priority process
       became runnable or the current process used up its time slice.  */
    __extension__ union
      {
	long int ru_nivcsw;
	__syscall_slong_t __ru_nivcsw_word;
      };
  };

int __getrusage64 (__rusage_who_t __who, struct __rusage64 *__usage)
{
  int result;
  struct rusage usage32;

  result = INLINE_SYSCALL(getrusage, 2, __who, &usage32);
  /* Copy fields from 32-bit into 64-bit rusage structure */
  /* Total amount of user time used.  */
  __usage->ru_utime.tv_sec = usage32.ru_utime.tv_sec;
  __usage->ru_utime.tv_usec = usage32.ru_utime.tv_usec;
  /* Total amount of system time used.  */
  __usage->ru_stime.tv_sec = usage32.ru_stime.tv_sec;
  __usage->ru_stime.tv_usec = usage32.ru_stime.tv_usec;
  /* Maximum resident set size (in kilobytes).  */
  __usage->ru_maxrss = usage32.ru_maxrss;
  /* Amount of sharing of text segment memory
     with other processes (kilobyte-seconds).  */
  /* Maximum resident set size (in kilobytes).  */
  __usage->ru_ixrss = usage32.ru_ixrss;
  /* Amount of data segment memory used (kilobyte-seconds).  */
  __usage->ru_idrss = usage32.ru_idrss;
  /* Amount of stack memory used (kilobyte-seconds).  */
  __usage->ru_isrss = usage32.ru_isrss;
  /* Number of soft page faults (i.e. those serviced by reclaiming
     a page from the list of pages awaiting reallocation.  */
  __usage->ru_minflt = usage32.ru_minflt;
  /* Number of hard page faults (i.e. those that required I/O).  */
  __usage->ru_majflt = usage32.ru_majflt;
  /* Number of times a process was swapped out of physical memory.  */
  __usage->ru_nswap = usage32.ru_nswap;
  /* Number of input operations via the file system.  Note: This
     and `ru_oublock' do not include operations with the cache.  */
  __usage->ru_inblock = usage32.ru_inblock;
  /* Number of output operations via the file system.  */
  __usage->ru_oublock = usage32.ru_oublock;
  /* Number of IPC messages sent.  */
  __usage->ru_msgsnd = usage32.ru_msgsnd;
  /* Number of IPC messages received.  */
  __usage->ru_msgrcv = usage32.ru_msgrcv;
  /* Number of signals delivered.  */
  __usage->ru_nsignals = usage32.ru_nsignals;
  /* Number of voluntary context switches, i.e. because the process
     gave up the process before it had to (usually to wait for some
     resource to be available).  */
  __usage->ru_nvcsw = usage32.ru_nvcsw;
  /* Number of involuntary context switches, i.e. a higher priority process
     became runnable or the current process used up its time slice.  */
  __usage->ru_nivcsw = usage32.ru_nivcsw;

  return result;
}
