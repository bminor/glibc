#ifndef __blkcnt_t_defined
#define __blkcnt_t_defined 1

#include <bits/types.h>

/* Type to count number of disk blocks (POSIX).  */
#ifndef __USE_FILE_OFFSET64
typedef __blkcnt_t blkcnt_t;
#else
typedef __blkcnt64_t blkcnt_t;
#endif

#endif
