#ifndef __fsfilcnt_t_defined
#define __fsfilcnt_t_defined 1

#include <bits/types.h>

/* Type to count file system inodes (POSIX).  */
#ifndef __USE_FILE_OFFSET64
typedef __fsfilcnt_t fsfilcnt_t;
#else
typedef __fsfilcnt64_t fsfilcnt_t;
#endif

#endif
