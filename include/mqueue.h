#include <rt/mqueue.h>

#ifndef _ISOMAC
# if IS_IN (librt)
extern __typeof (mq_open) __mq_open __THROW __nonnull ((1)) attribute_hidden;
hidden_proto (mq_timedsend)
extern __typeof (mq_timedsend) __mq_timedsend __nonnull ((2, 5));
hidden_proto (__mq_timedsend)
hidden_proto (mq_timedreceive)
extern __typeof (mq_timedreceive) __mq_timedreceive __nonnull ((2, 5));
hidden_proto (__mq_timedreceive)
hidden_proto (mq_setattr)
# endif
#endif
