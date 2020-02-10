#include <rt/mqueue.h>

#ifndef _ISOMAC
# if IS_IN (librt)
hidden_proto (mq_timedsend)
extern __typeof (mq_timedsend) __mq_timedsend __nonnull ((2, 5));
hidden_proto (__mq_timedsend)
hidden_proto (mq_timedreceive)
extern __typeof (mq_timedreceive) __mq_timedreceive __nonnull ((2, 5));
hidden_proto (__mq_timedreceive)
hidden_proto (mq_setattr)
# endif
#include <struct___timespec64.h>
#if __TIMESIZE == 64
# define __mq_timedsend_time64 __mq_timedsend
# define __mq_timedreceive_time64 __mq_timedreceive
#else
extern int __mq_timedsend_time64 (mqd_t mqdes, const char *msg_ptr,
                                  size_t msg_len, unsigned int msg_prio,
                                  const struct __timespec64 *abs_timeout);
librt_hidden_proto (__mq_timedsend_time64)
extern ssize_t __mq_timedreceive_time64 (mqd_t mqdes,
                                         char *__restrict msg_ptr,
                                         size_t msg_len,
                                         unsigned int *__restrict msg_prio,
                                         const struct __timespec64 *__restrict
                                         abs_timeout);
librt_hidden_proto (__mq_timedreceive_time64)
#endif
#endif
