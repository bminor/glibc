#ifndef __fsblkcnt_t_defined
#define __fsblkcnt_t_defined 1

#include <bits/types.h>

/* Type to count file system blocks (POSIX).  */
#ifndef __USE_FILE_OFFSET64
typedef __fsblkcnt_t fsblkcnt_t;
#else
typedef __fsblkcnt64_t fsblkcnt_t;
#endif

#endif
