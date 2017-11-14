#include <bits/wordsize.h>

/* These values are based on internal generic types from
   mutex-internal.h and rwlock-internal.h.  */

#if __WORDSIZE == 64
# define __PTHREAD_MUTEX_NUSERS_OFFSET          12
# define __PTHREAD_MUTEX_KIND_OFFSET            16
# define __PTHREAD_MUTEX_SPINS_OFFSET           20
# define __PTHREAD_MUTEX_LIST_OFFSET            24
#else
# define __PTHREAD_MUTEX_NUSERS_OFFSET          16
# define __PTHREAD_MUTEX_KIND_OFFSET            12
# define __PTHREAD_MUTEX_SPINS_OFFSET           20
# define __PTHREAD_MUTEX_LIST_OFFSET            20
#endif

#if __WORDSIZE == 64
# define __PTHREAD_RWLOCK_FLAGS_OFFSET          48
#else
# if __BYTE_ORDER == __BIG_ENDIAN
#  define __PTHREAD_RWLOCK_FLAGS_OFFSET         27
# else
#  define __PTHREAD_RWLOCK_FLAGS_OFFSET         24
# endif
#endif
