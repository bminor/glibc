/* Send a message to a message queue with a timeout.  Linux version.
   Copyright (C) 2017-2020 Free Software Foundation, Inc.
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

#include <mqueue.h>
#include <sysdep-cancel.h>

/* Add message pointed by MSG_PTR to message queue MQDES, stop blocking
   on full message queue if ABS_TIMEOUT expires.  */
int
__mq_timedsend_time64 (mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                       unsigned int msg_prio,
                       const struct __timespec64 *abs_timeout)
{
# ifndef __NR_mq_timedsend_time64
#  define __NR_mq_timedsend_time64 __NR_mq_timedsend
# endif
  int ret = SYSCALL_CANCEL (mq_timedsend_time64, mqdes, msg_ptr, msg_len,
			    msg_prio, abs_timeout);

#ifndef __ASSUME_TIME64_SYSCALLS
  if (ret == 0 || errno != ENOSYS)
    return ret;

  struct timespec ts32;
  if (abs_timeout != NULL)
    {
      if (! in_time_t_range (abs_timeout->tv_sec))
        {
          __set_errno (EOVERFLOW);
          return -1;
        }

      ts32 = valid_timespec64_to_timespec (*abs_timeout);
    }

  ret = SYSCALL_CANCEL (mq_timedsend, mqdes, msg_ptr, msg_len, msg_prio,
			abs_timeout != NULL ? &ts32 : NULL);
#endif

  return ret;
}

#if __TIMESIZE != 64
librt_hidden_def (__mq_timedsend_time64)

int
__mq_timedsend (mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                unsigned int msg_prio, const struct timespec *abs_timeout)
{
  struct __timespec64 ts64;
  if (abs_timeout != NULL)
    ts64 = valid_timespec_to_timespec64 (*abs_timeout);

  return __mq_timedsend_time64 (mqdes, msg_ptr, msg_len, msg_prio,
                                abs_timeout != NULL ? &ts64 : NULL);
}
#endif

hidden_def (__mq_timedsend)
weak_alias (__mq_timedsend, mq_timedsend)
hidden_weak (mq_timedsend)
