/* Receive the oldest from highest priority messages in message queue
   MQDES, stop waiting if ABS_TIMEOUT expires.

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

#include <errno.h>
#include <mqueue.h>
#include <y2038-support.h>

/* 64-bit time version */

ssize_t
__mq_timedreceiv_time64 (mqd_t mqdes, char *__restrict msg_ptr, size_t msg_len,
			 unsigned int *__restrict msg_prio,
			 const struct __timespec64 *__restrict abs_timeout)
{
  struct timespec ts32, *tsp32 = NULL;

#ifdef __NR_timedreceiv_time64
  int result;

  if (__y2038_get_kernel_support () > 0)
    {
      result = INLINE_SYSCALL (mq_timedreceiv_time64, 5, mqdes, msg_ptr, msg_len,
			       msg_prio, abs_timeout);
      if (result == 0 || errno != ENOSYS)
	return result;
      __y2038_set_kernel_support (-1);
    }
#endif

  if (abs_timeout)
    {
      ts32.tv_sec = abs_timeout->tv_sec;
      ts32.tv_nsec = abs_timeout->tv_nsec;
      tsp32 = &ts32;
    }
  return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, tsp32);
}
