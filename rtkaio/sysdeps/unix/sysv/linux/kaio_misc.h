/* Copyright (C) 1997,1999,2000,2001,2002,2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _AIO_MISC_H

#include <sysdep.h>

#if !defined __NR_io_setup || !defined __NR_io_destroy \
    || !defined __NR_io_getevents || !defined __NR_io_submit \
    || !defined __NR_io_cancel

#include <aio_misc.h>

#else

#define _AIO_MISC_H	1
#define USE_KAIO	1

#include <aio.h>
#include <pthread.h>
#include <stdint.h>

typedef unsigned long kctx_t;
#define KCTX_NONE ~0UL
extern kctx_t __aio_kioctx;

enum
{
  IO_CMD_PREAD,
  IO_CMD_PWRITE,
  IO_CMD_FSYNC,
  IO_CMD_FDSYNC,
  IO_CMD_PREADX,
  IO_CMD_POLL
};

struct kiocb
{
  uint64_t kiocb_data;
  uint64_t kiocb_key;
  uint16_t kiocb_lio_opcode;
  int16_t  kiocb_req_prio;
  uint32_t kiocb_fildes;
  uint64_t kiocb_buf;
  uint64_t kiocb_nbytes;
  int64_t  kiocb_offset;
  int64_t  __pad3, __pad4;
};

struct kio_event
{
  uint64_t kioe_data;
  uint64_t kioe_obj;
  int64_t  kioe_res;
  int64_t  kioe_res2;
};

/* Extend the operation enum.  */
enum
{
  LIO_DSYNC = LIO_NOP + 1,
  LIO_SYNC,
  LIO_READ64 = LIO_READ | 128,
  LIO_WRITE64 = LIO_WRITE | 128,
  LIO_KTHREAD = 0x10000,
  LIO_KTHREAD_REQUIRED = 0x20000
};


/* Union of the two request types.  */
typedef union
  {
    struct aiocb aiocb;
    struct aiocb64 aiocb64;
  } aiocb_union;


/* Used to synchronize.  */
struct waitlist
  {
    struct waitlist *next;

    pthread_cond_t *cond;
    volatile int *counterp;
    /* The next field is used in asynchronous `lio_listio' operations.  */
    struct sigevent *sigevp;
#ifdef BROKEN_THREAD_SIGNALS
    /* XXX See requestlist, it's used to work around the broken signal
       handling in Linux.  */
    pid_t caller_pid;
#endif
  };


/* Status of a request.  */
enum
{
  no,
  queued,
  yes,
  allocated,
  done
};


/* Used to queue requests..  */
struct requestlist
  {
    struct kiocb kiocb;
    kctx_t kioctx;

    int running;

    struct requestlist *last_fd;
    struct requestlist *next_fd;
    struct requestlist *next_prio;
    struct requestlist *next_run;
    /* For kioctx != KCTX_NONE requests we are doubly linked.  */
#define prev_prio next_run

    /* Pointer to the actual data.  */
    aiocb_union *aiocbp;

#ifdef BROKEN_THREAD_SIGNALS
    /* PID of the initiator thread.
       XXX This is only necessary for the broken signal handling on Linux.  */
    pid_t caller_pid;
#endif

    /* List of waiting processes.  */
    struct waitlist *waiting;
  };


/* Lock for global I/O list of requests.  */
extern pthread_mutex_t __aio_requests_mutex attribute_hidden;


/* Enqueue request.  */
extern struct requestlist *__aio_enqueue_request_ctx (aiocb_union *aiocbp,
						      int operation,
						      kctx_t kctx)
     attribute_hidden internal_function;

#define __aio_enqueue_request(aiocbp, operation) \
  __aio_enqueue_request_ctx (aiocbp, operation | LIO_KTHREAD, KCTX_NONE)

/* Find request entry for given AIO control block.  */
extern struct requestlist *__aio_find_req (aiocb_union *elem)
     attribute_hidden internal_function;

/* Find request entry for given file descriptor.  */
extern struct requestlist *__aio_find_req_fd (int fildes)
     attribute_hidden internal_function;

/* Find request entry for given file descriptor.  */
extern struct requestlist *__aio_find_kreq_fd (int fildes)
     attribute_hidden internal_function;

/* Remove request from the list.  */
extern void __aio_remove_request (struct requestlist *last,
				  struct requestlist *req, int all)
     attribute_hidden internal_function;

extern void __aio_remove_krequest (struct requestlist *req)
     attribute_hidden internal_function;

/* Release the entry for the request.  */
extern void __aio_free_request (struct requestlist *req)
     attribute_hidden internal_function;

/* Notify initiator of request and tell this everybody listening.  */
extern void __aio_notify (struct requestlist *req)
     attribute_hidden internal_function;

/* Notify initiator of request.  */
#ifdef BROKEN_THREAD_SIGNALS
extern int __aio_notify_only (struct sigevent *sigev, pid_t caller_pid)
     attribute_hidden internal_function;
#else
extern int __aio_notify_only (struct sigevent *sigev)
     attribute_hidden internal_function;
#endif

/* Send the signal.  */
extern int __aio_sigqueue (int sig, const union sigval val, pid_t caller_pid)
     attribute_hidden internal_function;

extern int __aio_wait_for_events (kctx_t kctx, const struct timespec *timeout)
     attribute_hidden internal_function;

extern void __aio_read_one_event (void) attribute_hidden internal_function;

extern int __aio_create_kernel_thread (void)
     attribute_hidden internal_function;

extern int __have_no_kernel_aio attribute_hidden;
extern int __kernel_thread_started attribute_hidden;

#endif
#endif /* aio_misc.h */
